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
        auto f = graph::createMatchClause(m.path, getContext(), m_server);
        assert(f && "creating filter chain failed");
        m_matchClauses.emplace_back(std::move(f));
    }
}

const std::vector<UA_ReferenceDescription>* QueryEngine::run()
{
    for (auto& f : m_matchClauses)
    {
        f->run();
    }
    return m_matchClauses.back()->results();
}

const PathResult& QueryEngine::pathResult() const
{
    return m_matchClauses.back()->pathResult();
}

const PathResult& QueryEngine::pathResult(size_t idx) const
{
    return m_matchClauses.at(idx)->pathResult();
}


std::vector<std::reference_wrapper<const graph::MatchClause>> QueryEngine::getContext() const
{
    std::vector<std::reference_wrapper<const graph::MatchClause>> ctx{};
    for (const auto& f : m_matchClauses)
    {
        ctx.emplace_back(*f.get());
    }
    return ctx;
}