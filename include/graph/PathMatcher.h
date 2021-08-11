#pragma once
#include "Filter.h"
#include <open62541/server.h>
#include <open62541/types.h>
#include <optional>
#include <vector>
#include "Types.h"

class BrowseResultWrapper
{
 public:
    BrowseResultWrapper(UA_BrowseResult br)
    : m_br{ std::move(br) } {};
    ~BrowseResultWrapper()
    {
        UA_BrowseResult_clear(&m_br);
    }
    BrowseResultWrapper(const BrowseResultWrapper&) = delete;
    BrowseResultWrapper& operator=(const BrowseResultWrapper&) = delete;

    UA_BrowseResult& raw()
    {
        return m_br;
    };

 private:
    UA_BrowseResult m_br;
};

/*
   referenceType: the reference to match, UA_NODEID_NULL for any referenceType
*/
struct PathElement
{
    UA_NodeId referenceType;
    UA_NodeClass nodeClass;
    std::optional<UA_NodeId> targetId;
    UA_BrowseDirection direction;
};



template <bool leftToRight>
struct ResultVector
{};

template <>
struct ResultVector<true>
{
    void addResult(const UA_ReferenceDescription& ref)
    {
        data.emplace_back(ref);
    }

    const UA_ReferenceDescription& getLastResult() const
    {
        return data.back();
    }

    path_t data;
};

template <>
struct ResultVector<false>
{
    void addResult(const UA_ReferenceDescription& ref)
    {
        data.insert(data.begin(), ref);
    }

    const UA_ReferenceDescription& getLastResult() const
    {
        return data.front();
    }

    path_t data;
};

class PathResult
{
 public:
    PathResult(size_t columnCount)
    {
        for (auto i = 0u; i < columnCount; ++i)
        {
            data.emplace_back(std::vector<path_element_t>{});
        }
    }

    void emplace(path_t&& p)
    {
        size_t c = 0u;
        for (auto&& e : p)
        {
            data[c].emplace_back(std::move(e));
            ++c;
        }
    }

    const column_t* col() const
    {
        return col(0);
    }

    const column_t* col(size_t idx) const
    {
        if (idx < data.size())
        {
            return &data[idx];
        }
        return nullptr;
    }

    std::vector<path_t> paths() const
    {
        std::vector<path_t> res;
        for (auto row = 0u; row < data[0].size(); ++row)
        {
            path_t p;
            for (auto col = 0u; col < data.size(); ++col)
            {
                p.emplace_back(data[col][row]);
            }
            res.emplace_back(std::move(p));
        }
        return res;
    }

    column_t& operator[](size_t idx)
    {
        assert(idx < data.size());
        return data[idx];
    }

 private:
    std::vector<column_t> data;
};


class PathMatcher
{
 public:
    PathMatcher(UA_Server* server, const std::vector<PathElement>& path, size_t startIndex = 0)
    : m_server{ server }
    , m_path{ path }
    , m_idx{ startIndex }
    , m_results{ path.size() + 1 }
    {}

    PathMatcher(const PathMatcher&) = delete;
    PathMatcher& operator=(const PathMatcher&) = delete;

    PathMatcher(PathMatcher&& other)
    : m_server{ other.m_server }
    , m_path{ other.m_path }
    , m_idx{ other.m_idx }
    , m_results{ other.m_results }
    {}

    PathMatcher& operator=(PathMatcher&& other)
    {
        m_server = other.m_server;
        m_path = std::move(other.m_path);
        m_idx = other.m_idx;
        m_results = std::move(other.m_results);
        return *this;
    }

    void match(const UA_ReferenceDescription& startNode)
    {
        if (m_path.empty())
        {
            m_results[0].emplace_back(startNode);
            return;
        }

        for (auto&& p : checkPath(startNode))
        {
            m_results.emplace(std::move(p));
        }
    }

    const PathResult& results() const
    {
        return m_results;
    }


 private:
    std::vector<path_t> checkPath(const UA_ReferenceDescription& startNode)
    {
        auto rResults = checkRightSide(startNode);
        std::vector<path_t> endResult{};
        for (auto& r : rResults)
        {
            if (m_idx > 0)
            {
                auto lResults = checkLeftSide(r.at(0));
                for (auto& l : lResults)
                {
                    r.insert(r.begin(), l.begin(), l.end() - 1);
                    endResult.emplace_back(r);
                }
            }
            else
            {
                endResult.emplace_back(r);
            }
        }
        return endResult;
    }

    UA_BrowseDescription createBrowseDescription(const UA_NodeId& node,
                                                 UA_BrowseDirection direction,
                                                 const UA_NodeId& refType,
                                                 UA_NodeClass nodeClass)
    {
        UA_BrowseDescription bd;
        UA_BrowseDescription_init(&bd);
        bd.nodeId = node;
        bd.browseDirection = direction;
        bd.includeSubtypes = true;
        bd.referenceTypeId = refType;
        bd.resultMask = UA_BROWSERESULTMASK_REFERENCETYPEID |
                        UA_BROWSERESULTMASK_ISFORWARD | UA_BROWSERESULTMASK_TYPEDEFINITION |
                        UA_BROWSERESULTMASK_NODECLASS | UA_BROWSERESULTMASK_TARGETINFO;
        bd.nodeClassMask = nodeClass;
        return bd;
    }

    // returns paths satisfying the right side
    std::vector<path_t> checkRightSide(const UA_ReferenceDescription& start)
    {
        return check<ResultVector<true>>(start,
                                         m_path.cbegin() + static_cast<int>(m_idx),
                                         m_path.cend());
    }

    std::vector<path_t> checkLeftSide(const UA_ReferenceDescription& start)
    {
        return check<ResultVector<false>>(start,
                                          m_path.crbegin() +
                                          static_cast<int>(m_path.size() - m_idx),
                                          m_path.crend());
    }

    template <typename act_path_t, typename IT>
    std::vector<path_t> check(const UA_ReferenceDescription& start, IT begin, IT end)
    {
        std ::vector<path_t> paths;
        act_path_t actPath{};
        actPath.addResult(start);

        std::optional<UA_ReferenceDescription> lastRef{ std::nullopt };
        std::optional<PathMatcher> lastMatcher{ std::nullopt };
        for (auto it = begin; it != end; it++)
        {
            auto bd = createBrowseDescription(actPath.getLastResult().nodeId.nodeId,
                                              it->direction,
                                              it->referenceType,
                                              it->nodeClass);
            BrowseResultWrapper br{ UA_Server_browse(m_server, 1000, &bd) };
            if (br.raw().statusCode != UA_STATUSCODE_GOOD)
            {
                return std::vector<path_t>{};
            }

            if (it->targetId)
            {
                for (const auto* ref = br.raw().references;
                     ref != br.raw().references + br.raw().referencesSize;
                     ++ref)
                {
                    if (UA_NodeId_equal(&it->targetId.value(), &ref->nodeId.nodeId))
                    {
                        lastRef = *ref;
                        break;
                    }
                }
            }
            else
            {
                // we have to instantiate a pathMatcher for each of this
                // subPaths rightToLeft is here not handled correctly
                PathMatcher m{ m_server, std::vector<PathElement>{ it + 1, end } };
                for (const auto* ref = br.raw().references;
                     ref != br.raw().references + br.raw().referencesSize;
                     ++ref)
                {
                    m.match(*ref);
                }
                lastMatcher.emplace(std::move(m));
            }

            if (lastRef)
            {
                actPath.addResult(*lastRef);
                lastRef = std::nullopt;
            }
            else if (lastMatcher)
            {
                std::vector<path_t> res;
                for (auto&& pe : lastMatcher->results().paths())
                {
                    path_t newPath{ actPath.data };
                    newPath.insert(newPath.end(), pe.begin(), pe.end());
                    res.emplace_back(newPath);
                }
                return res;
            }
            else
            {
                return std::vector<path_t>{};
            }
        }
        paths.emplace_back(actPath.data);
        return paths;
    }

    UA_Server* m_server;
    std::vector<PathElement> m_path;
    size_t m_idx{ 0 };
    PathResult m_results;
};