#pragma once
#include "Filter.h"
#include <open62541/server.h>
#include <open62541/types.h>
#include <optional>
#include <vector>

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

using path_t = std::vector<UA_ReferenceDescription>;
using column_t = std::vector<UA_ReferenceDescription>;
class PathMatcher
{
 public:
    PathMatcher(UA_Server* server, const std::vector<PathElement>& path, size_t startIndex = 0)
    : m_server{ server }
    , m_path{ path }
    , m_idx{ startIndex }
    {
        // add the result vectors
        //+1 for the start node
        for (auto i = 0u; i < path.size() + 1; i++)
        {
            m_results.emplace_back(path_t{});
        }
    }

    PathMatcher(const PathMatcher&) = delete;
    PathMatcher& operator=(const PathMatcher&) = delete;

    PathMatcher(PathMatcher&& other)
    : m_server{ other.m_server }
    , m_path{ other.m_path }
    , m_results{ other.m_results }
    , m_idx{ other.m_idx }
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
        // empty Path
        if (m_path.empty())
        {
            m_results[0].emplace_back(startNode);
            return;
        }

        for (const auto& p : checkPath(startNode))
        {
            // add the path to the results
            for (auto i = 0u; i < p.size(); ++i)
            {
                m_results[i].emplace_back(std::move(p[i]));
            }
        }
    }

    const column_t* results() const
    {
        return &m_results[0];
    }

    const column_t* results(size_t idx) const
    {
        if (idx < m_results.size())
        {
            return &m_results[idx];
        }
        return nullptr;
    }

 private:
    std::vector<path_t> checkPath(const UA_ReferenceDescription& startNode)
    {
        auto result = checkRightSide(startNode);

        std::vector<path_t> endResult{};
        for (auto& p : result)
        {
            if (checkLeftSide(p))
            {
                endResult.emplace_back(p);
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
        std::vector<path_t> paths;
        path_t actPath{};
        actPath.push_back(start);

        std::optional<UA_ReferenceDescription> lastRef{ std::nullopt };
        std::optional<PathMatcher> lastMatcher{ std::nullopt };

        for (auto it = m_path.cbegin() + static_cast<int>(m_idx); it != m_path.cend(); it++)
        {
            auto bd = createBrowseDescription(actPath.back().nodeId.nodeId,
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
                // we have to instantiate a pathMatcher for each of this subPaths
                PathMatcher m{ m_server,
                               std::vector<PathElement>{ it + 1, m_path.cend() } };
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
                actPath.push_back(*lastRef);
                lastRef = std::nullopt;
            }
            else if (lastMatcher)
            {
                std::vector<path_t> res;
                // add the results
                if (lastMatcher->results())
                {
                    size_t maxPaths = 0;
                    if (lastMatcher->results())
                    {
                        maxPaths = lastMatcher->results()->size();
                    }
                    for (auto i = 0u; i < maxPaths; i++)
                    {
                        size_t c = 0;
                        path_t newPath{ actPath };
                        while (const auto* p = lastMatcher->results(c))
                        {
                            newPath.emplace_back(p->at(i));
                            c += 1;
                        }
                        res.emplace_back(newPath);
                    }
                }
                return res;
            }
            else
            {
                return std::vector<path_t>{};
            }
        }
        paths.emplace_back(actPath);
        actPath.clear();
        return paths;
    }


    bool checkLeftSide(path_t& actPath)
    {
        for (auto it = m_path.crbegin() + static_cast<int>(m_path.size() - m_idx);
             it != m_path.crend();
             it++)
        {
            auto bd = createBrowseDescription(actPath.front().nodeId.nodeId,
                                              it->direction,
                                              it->referenceType,
                                              it->nodeClass);
            BrowseResultWrapper br{ UA_Server_browse(m_server, 1000, &bd) };
            if (br.raw().statusCode != UA_STATUSCODE_GOOD)
            {
                return false;
            }

            auto result = false;
            if (it->targetId)
            {
                for (const auto* ref = br.raw().references;
                     ref != br.raw().references + br.raw().referencesSize;
                     ++ref)
                {
                    if (UA_NodeId_equal(&it->targetId.value(), &ref->nodeId.nodeId))
                    {
                        actPath.insert(actPath.begin(), *ref);
                        result = true;
                        break;
                    }
                }
            }
            else
            {
                // we have no clear startId
                for (const auto* ref = br.raw().references;
                     ref != br.raw().references + br.raw().referencesSize;
                     ++ref)
                {
                    actPath.insert(actPath.begin(), *ref);
                    result = true;
                    break;
                }
            }
            if (!result)
            {
                return false;
            }
        }
        return true;
    }

    UA_Server* m_server;
    std::vector<PathElement> m_path;
    std::vector<column_t> m_results;
    size_t m_idx{ 0 };
};