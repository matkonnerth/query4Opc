#pragma once
#include "Ast.h"
#include "Path.h"
#include <graph/FilterChain.h>
#include <memory>

namespace cypher {
class QueryEngine
{
 public:
    QueryEngine(UA_Server* server)
    : m_server{ server }
    {}
    void scheduleQuery(const Query& q)
    {
        m_filterChain = createFilterChain(q.matchClauses[0].path, m_server);
    }

    const std::vector<UA_ReferenceDescription>* run()
    {
        m_filterChain->run();
        return m_filterChain->results();
    }

 private:
    UA_Server* m_server;
    std::unique_ptr<FilterChain> m_filterChain{ nullptr };
};
} // namespace cypher