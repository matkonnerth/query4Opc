#pragma once
#include "PathMatcher.h"
#include "Source.h"
#include <cypher/Path.h>
#include <memory>
#include <optional>
#include <vector>

namespace graph {

class PathResult;

class MatchClause
{
 public:
    MatchClause(UA_Server* server, const cypher::Path& path);
    void run();

    void createHierachicalVisitorSource(const UA_NodeId& root,
                                        const UA_NodeId& referenceTypeId,
                                        UA_UInt32 nodeclasMask);
    void createColumnAsSource(const column_t& col);
    void createReferenceFilter(int startIndex);
    void createDefaultSink();
    
    const column_t* results() const;
    const column_t* results(const std::string& identifier) const;
    
    const PathResult& pathResult() const;

    std::string explain() const
    {
        std::string explanation{ "MatchClause\n" };
        if (m_src)
        {
            explanation.append(m_src->explain());
        }
        if (m_sink)
        {
            explanation.append(m_sink->explain());
        }
        return explanation;
    }

 private:
    UA_Server* m_server;
    std::unique_ptr<Source> m_src;
    std::unique_ptr<Sink> m_sink;
    cypher::Path m_path;
};

int findStartIndex(const cypher::Path& p,
                   const std::vector<std::reference_wrapper<const MatchClause>> ctx);
const column_t*
findSourceColumn(const std::string id,
                 const std::vector<std::reference_wrapper<const MatchClause>> ctx);

// translates a cypher Path to a MatchClause
std::unique_ptr<MatchClause>
createMatchClause(const cypher::Match& match,
                  std::vector<std::reference_wrapper<const MatchClause>> ctx,
                  UA_Server* server);



} // namespace graph