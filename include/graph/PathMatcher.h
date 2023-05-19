#pragma once
#include "Path.h"
#include "PathResult.h"
#include "Types.h"
#include <open62541/server.h>
#include <open62541/types.h>
#include <optional>
#include <vector>

namespace graph {
// takes an graph::Path and matches against the server address space with browsing
class PathMatcher
{
 public:
    PathMatcher() = default;
    PathMatcher(UA_Server* server, const Path& path, int startIndex = 0);

    PathMatcher(const PathMatcher&) = delete;
    PathMatcher& operator=(const PathMatcher&) = delete;
    PathMatcher(PathMatcher&& other) = default;
    PathMatcher& operator=(PathMatcher&& other) = default;

    void match(const UA_ReferenceDescription& startNode);
    const PathResult& results() const;

 private:
    std::vector<path_t> checkPath(const UA_ReferenceDescription& startNode);

    UA_BrowseDescription createBrowseDescription(const UA_NodeId& node,
                                                 UA_BrowseDirection direction,
                                                 const UA_NodeId& refType,
                                                 UA_NodeClass nodeClass);

    std::vector<path_t> check(const UA_ReferenceDescription& start, const Path& p);

    UA_Server* m_server{ nullptr };
    Path m_path{};
    Path m_rhs{};
    Path m_lhs{};
    int m_idx{ 0 };
    PathResult m_results;
};

} // namespace graph