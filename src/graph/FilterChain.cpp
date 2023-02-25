#include <graph/FilterChain.h>
#include <graph/Helper.h>

using graph::FilterChain;
using graph::createFilterChain;
using graph::parseOptionalNodeClass;
using graph::column_t;

FilterChain::FilterChain(UA_Server* server)
: m_server{ server }
{}

void FilterChain::run()
{
    auto f = [&](path_element_t&& pe) { m_pathMatcher.match(pe); };
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
    m_pathMatcher = std::move(PathMatcher(m_server, Path{path}, startIndex));
    m_path = path;
}

const column_t* FilterChain::results() const
{
    return m_pathMatcher.results().col();
}

const column_t* FilterChain::results(const std::string& identifier) const
{
    size_t idx = 0;
    for (const auto& n : m_path.nodes)
    {
        if (n.identifier && n.identifier == identifier)
        {
            return m_pathMatcher.results().col(idx);
        }
        idx++;
    }
    return nullptr;
}

int graph::findStartIndex(const cypher::Path& p)
{
    auto idx = 0;
    for (const auto& e : p.nodes)
    {
        if (e.identifier)
        {
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
    if (ctx.size() == 0)
    {
        //TODO: remove hardcoded start node
        f->createHierachicalVisitorSource(
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES),
        parseOptionalNodeClass(path.nodes[static_cast<size_t>(start)].label));
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
    }

    f->createReferenceFilter(path, start);
    return f;
}