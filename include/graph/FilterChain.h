#pragma once
#include "PathMatcher.h"
#include "Source.h"
#include <cypher/Path.h>
#include <memory>
#include <optional>
#include <vector>

namespace graph {

class PathResult;

class FilterChain
{
 public:
    FilterChain(UA_Server* server);
    void run();

    void createHierachicalVisitorSource(const UA_NodeId& root,
                                        const UA_NodeId& referenceTypeId,
                                        UA_UInt32 nodeclasMask);
    void createColumnAsSource(const column_t& col);
    void createReferenceFilter(const cypher::Path& path, int startIndex);
    const column_t* results() const;
    const column_t* results(const std::string& identifier) const;
    const PathResult& pathResult() const;

 private:
    UA_Server* m_server;
    std::unique_ptr<Source> m_src;
    PathMatcher m_pathMatcher;
    cypher::Path m_path;
};

int findStartIndex(const cypher::Path& p);
const column_t*
findSourceColumn(const std::string id,
                 const std::vector<std::reference_wrapper<const FilterChain>> ctx);

// translates a cypher Path to a FilterChain
std::unique_ptr<FilterChain>
createFilterChain(const cypher::Path& path,
                  std::vector<std::reference_wrapper<const FilterChain>> ctx,
                  UA_Server* server);

} // namespace graph