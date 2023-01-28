#pragma once
#include "Filter.h"
#include "Types.h"
#include <open62541/server.h>
#include <open62541/types.h>
#include <optional>
#include <vector>
#include "Path.h"

namespace graph
{
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
        assert(p.size()==data.size());
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

//takes an graph::Path and matches against the server address space with browsing

class PathMatcher
{
 public:
    PathMatcher(UA_Server* server, const Path& path, size_t startIndex = 0);

    PathMatcher(const PathMatcher&) = delete;
    PathMatcher& operator=(const PathMatcher&) = delete;
    PathMatcher(PathMatcher&& other);
    PathMatcher& operator=(PathMatcher&& other);

    void match(const UA_ReferenceDescription& startNode);
    const PathResult& results() const;

 private:
    std::vector<path_t> checkPath(const UA_ReferenceDescription& startNode);

    UA_BrowseDescription createBrowseDescription(const UA_NodeId& node,
                                                 UA_BrowseDirection direction,
                                                 const UA_NodeId& refType,
                                                 UA_NodeClass nodeClass);

    // returns paths satisfying the right side
    std::vector<path_t> checkRightSide(const UA_ReferenceDescription& start);
    std::vector<path_t> checkLeftSide(const UA_ReferenceDescription& start);

    
    std::vector<path_t> check(const UA_ReferenceDescription& start, const Path& p);

    UA_Server* m_server;
    Path m_path{};
    Path m_rhs{};
    Path m_lhs{};
    size_t m_idx{ 0 };
    PathResult m_results;
};

}