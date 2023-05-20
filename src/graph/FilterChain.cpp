#include <graph/FilterChain.h>
#include <graph/Helper.h>
#include <graph/PathResult.h>
#include <iostream>

using graph::column_t;
using graph::createFilterChain;
using graph::FilterChain;
using graph::parseOptionalNodeClass;
using graph::PathResult;

FilterChain::FilterChain(UA_Server* server)
: m_server{ server }
{}

void FilterChain::run()
{
    auto f = [&](path_element_t&& pe) { m_sink->filter(std::move(pe)); };
    m_src->generate(f);
}

void FilterChain::createHierachicalVisitorSource(const UA_NodeId& root,
                                                 const UA_NodeId& referenceTypeId,
                                                 UA_UInt32 nodeclasMask)
{
    m_src = std::make_unique<HierachicalVisitor>(m_server, root, referenceTypeId, nodeclasMask);
}

void FilterChain::createColumnAsSource(const column_t& col)
{
    m_src = std::make_unique<ColumnAsSource>(col);
}

void FilterChain::createReferenceFilter(const cypher::Path& path, int startIndex)
{
    m_sink = std::make_unique<PathMatcher>(m_server, Path{ path }, startIndex);
    m_path = path;
}

const column_t* FilterChain::results() const
{
    return m_sink->results().col();
}

const column_t* FilterChain::results(const std::string& identifier) const
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

void FilterChain::createDefaultSink()
{
    m_sink = std::make_unique<DefaultSink>();
}

int graph::findStartIndex(const cypher::Path& p)
{
    auto idx = 0;
    for (const auto& e : p.nodes)
    {
        if (e.identifier)
        {
            std::cout << "start Index for FilterChain at node with identifier: "
                      << *e.identifier << "\n";
            return idx;
        }
        idx += 1;
    }
    return 0;
}

const column_t*
graph::findSourceColumn(const std::string id,
                        const std::vector<std::reference_wrapper<const FilterChain>> ctx)
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

// translates a cypher Path to a FilterChain
std::unique_ptr<FilterChain>
graph::createFilterChain(const cypher::Path& path,
                         std::vector<std::reference_wrapper<const FilterChain>> ctx,
                         UA_Server* server)
{
    auto start = graph::findStartIndex(path);
    auto f = std::make_unique<FilterChain>(server);

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
            f->createReferenceFilter(path, start);
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
        if (!col)
        {
            return nullptr;
        }
        f->createColumnAsSource(*col);
        f->createReferenceFilter(path, start);
    }
    return f;
}

const PathResult& FilterChain::pathResult() const
{
    return m_sink->results();
}