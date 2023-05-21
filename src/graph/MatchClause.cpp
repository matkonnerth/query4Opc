#include <graph/MatchClause.h>
#include <graph/Helper.h>
#include <graph/PathResult.h>
#include <iostream>

using graph::column_t;
using graph::createMatchClause;
using graph::MatchClause;
using graph::parseOptionalNodeClass;
using graph::PathResult;

MatchClause::MatchClause(UA_Server* server, const cypher::Path& path)
: m_server{ server }, m_path{path}
{}

void MatchClause::run()
{
    auto f = [&](path_element_t&& pe) { m_sink->filter(std::move(pe)); };
    m_src->generate(f);
}

void MatchClause::createHierachicalVisitorSource(const UA_NodeId& root,
                                                 const UA_NodeId& referenceTypeId,
                                                 UA_UInt32 nodeclasMask)
{
    m_src = std::make_unique<HierachicalVisitor>(m_server, root, referenceTypeId, nodeclasMask);
}

void MatchClause::createColumnAsSource(const column_t& col)
{
    m_src = std::make_unique<ColumnAsSource>(col);
}

void MatchClause::createReferenceFilter(int startIndex)
{
    m_sink = std::make_unique<PathMatcher>(m_server, Path{ m_path }, startIndex);
}

const column_t* MatchClause::results() const
{
    return m_sink->results().col();
}

const column_t* MatchClause::results(const std::string& identifier) const
{
    size_t idx = 0;
    for (const auto& n : m_path.nodes)
    {
        if (n.identifier && n.identifier == identifier)
        {
            return m_sink->results().col(idx);
        }
        idx++;
    }
    return nullptr;
}

void MatchClause::createDefaultSink()
{
    m_sink = std::make_unique<DefaultSink>();
}

int graph::findStartIndex(const cypher::Path& p,
                          const std::vector<std::reference_wrapper<const MatchClause>> ctx)
{
    auto idxWithIdentifier = 0;
    auto idx = 0;
    for (const auto& e : p.nodes)
    {
        if (e.identifier)
        {
            std::cout << "start Index for MatchClause at node with identifier: "
                      << *e.identifier << "\n";
            auto c = graph::findSourceColumn(*e.identifier, ctx);
            if(c)
            {
                return idx;
            }
            if(idxWithIdentifier==0)
            {
                idxWithIdentifier = idx;
            }
        }
        idx += 1;
    }
    return idxWithIdentifier;
}

const column_t*
graph::findSourceColumn(const std::string id,
                        const std::vector<std::reference_wrapper<const MatchClause>> ctx)
{
    for (const auto& f : ctx)
    {
        auto c = f.get().results(id);
        if (c)
        {
            return c;
        }
    }
    return nullptr;
}

// translates a cypher Path to a MatchClause
std::unique_ptr<MatchClause>
graph::createMatchClause(const cypher::Path& path,
                         std::vector<std::reference_wrapper<const MatchClause>> ctx,
                         UA_Server* server)
{
    auto start = graph::findStartIndex(path, ctx);
    auto f = std::make_unique<MatchClause>(server, path);

    // get objectTypes and subtypes
    if (ctx.size() == 0)
    {
        if (path.nodes.size() == 1 && path.nodes[0].queryObjectTypeAndAllSubTypes())
        {
            auto rootNode = parseNodeId(*path.nodes[0].NodeId());
            f->createHierachicalVisitorSource(rootNode,
                                              UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES),
                                              UA_NODECLASS_OBJECTTYPE);
            f->createDefaultSink();
        }
        else
        {
            auto startNodeClass =
            parseOptionalNodeClass(path.nodes[static_cast<size_t>(start)].label);

            auto rootNode = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);

            if (startNodeClass == UA_NODECLASS_OBJECTTYPE)
            {
                rootNode = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTTYPESFOLDER);
            }

            f->createHierachicalVisitorSource(rootNode,
                                              UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES),
                                              startNodeClass);
            f->createReferenceFilter(start);
        }
    }
    else
    {
        // lookup the source column
        if (!path.nodes[static_cast<size_t>(start)].identifier)
        {
            return nullptr;
        }

        auto col =
        graph::findSourceColumn(*path.nodes[static_cast<size_t>(start)].identifier, ctx);
        assert(col && "findSourceColumn failed");
        if (!col)
        {
            
            return nullptr;
        }
        f->createColumnAsSource(*col);
        f->createReferenceFilter(start);
    }
    return f;
}

const PathResult& MatchClause::pathResult() const
{
    return m_sink->results();
}