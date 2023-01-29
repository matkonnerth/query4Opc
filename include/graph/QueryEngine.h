#pragma once
#include <cypher/Ast.h>
#include <cypher/Path.h>
#include <graph/FilterChain.h>
#include <memory>

namespace graph {
class QueryEngine
{
 public:
    QueryEngine(UA_Server* server)
    : m_server{ server }
    {}
    void scheduleQuery(const cypher::Query& q)
    {
        for(const auto& m : q.matchClauses)
        {
            auto f = graph::createFilterChain(m.path, getContext(), m_server);
            assert(f && "creating filter chain failed");
            m_filterChains.emplace_back(std::move(f));
        }
        
    }

    const std::vector<UA_ReferenceDescription>* run()
    {
        for(auto & f : m_filterChains)
        {
            f->run();
        }
        return m_filterChains.back()->results();
    }

 private:
    std::vector<std::reference_wrapper<const graph::FilterChain>> getContext() const
    {
        std::vector<std::reference_wrapper<const graph::FilterChain>> ctx{};
        for(const auto& f:m_filterChains)
        {
            ctx.emplace_back(*f.get());
        }
        return ctx;
    }
    UA_Server* m_server;
    std::vector<std::unique_ptr<graph::FilterChain>> m_filterChains;
};
} // namespace cypher