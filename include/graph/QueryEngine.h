#pragma once
#include <cypher/Ast.h>
#include <cypher/Path.h>
#include <graph/MatchClause.h>
#include <memory>

namespace graph {

class PathResult;
class QueryEngine final
{
 public:
    QueryEngine(UA_Server* server);
    void scheduleQuery(const cypher::Query& q);
    void run();
    const PathResult& pathResult() const;
    const PathResult& pathResult(size_t idx) const;

 private:
    std::vector<std::reference_wrapper<const graph::MatchClause>> getContext() const;
    UA_Server* m_server;
    std::vector<std::unique_ptr<graph::MatchClause>> m_matchClauses;
};
} // namespace graph