#pragma once
#include <cypher/Ast.h>
#include <cypher/Path.h>
#include <graph/FilterChain.h>
#include <memory>

namespace graph {

class PathResult;
class QueryEngine
{
 public:
    QueryEngine(UA_Server* server);
    void scheduleQuery(const cypher::Query& q);
    const std::vector<UA_ReferenceDescription>* run();
    const PathResult& pathResult() const;

 private:
    std::vector<std::reference_wrapper<const graph::FilterChain>> getContext() const;
    UA_Server* m_server;
    std::vector<std::unique_ptr<graph::FilterChain>> m_filterChains;
};
} // namespace graph