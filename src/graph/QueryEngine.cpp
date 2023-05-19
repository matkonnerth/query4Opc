#include <graph/PathResult.h>
#include <graph/QueryEngine.h>

using graph::PathResult;
using graph::QueryEngine;

QueryEngine::QueryEngine(UA_Server* server)
: m_server{ server }
{}
void QueryEngine::scheduleQuery(const cypher::Query& q)
{
    for (const auto& m : q.matchClauses)
    {
        auto f = graph::createFilterChain(m.path, getContext(), m_server);
        assert(f && "creating filter chain failed");
        m_filterChains.emplace_back(std::move(f));
    }
}

const std::vector<UA_ReferenceDescription>* QueryEngine::run()
{
    for (auto& f : m_filterChains)
    {
        f->run();
    }
    return m_filterChains.back()->results();
}

const PathResult& QueryEngine::pathResult() const
{
    return m_filterChains.back()->pathResult();
}


std::vector<std::reference_wrapper<const graph::FilterChain>> QueryEngine::getContext() const
{
    std::vector<std::reference_wrapper<const graph::FilterChain>> ctx{};
    for (const auto& f : m_filterChains)
    {
        ctx.emplace_back(*f.get());
    }
    return ctx;
}