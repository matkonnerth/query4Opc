#include <graph/FilterChain.h>
#include <graph/Helper.h>

FilterChain::FilterChain(UA_Server* server)
: m_server{ server }
{}

void FilterChain::run()
{
    auto f = [&](path_element_t&& pe) { m_pathMatcher->match(pe); };
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

// here (node)-(reference) decays to a single PathElement
// think this makes sense because the opc ua browse returns n ReferenceDescriptions for a node
void FilterChain::createReferenceFilter(const cypher::Path& path, size_t startIndex)
{

    std::vector<PathElement> p;
    cypher::PathIterator it{ path };
    size_t idx = 0;
    while (auto node = it.currentNode())
    {
        if (!it.nextRel())
        {
            break;
        }
        PathElement e{};
        if (idx < startIndex)
        {
            e.referenceType = lookupReferenceType(it.nextRel()->type);
            e.nodeClass = parseOptionalNodeClass(node->label);
            if (node->NodeId())
            {
                e.targetId = parseOptionalNodeId(node->NodeId());
            }
            if (it.nextRel()->direction == 1)
            {
                e.direction = UA_BROWSEDIRECTION_INVERSE;
            }
            else if (it.nextRel()->direction == -1)
            {
                e.direction = UA_BROWSEDIRECTION_FORWARD;
            }
            else
            {
                e.direction = UA_BROWSEDIRECTION_BOTH;
            }
        }
        else
        {
            e.referenceType = lookupReferenceType(it.nextRel()->type);
            e.nodeClass = parseOptionalNodeClass(it.nextNode()->label);
            if (it.nextNode()->NodeId())
            {
                e.targetId = parseOptionalNodeId(it.nextNode()->NodeId());
            }
            if (it.nextRel()->direction == 1)
            {
                e.direction = UA_BROWSEDIRECTION_FORWARD;
            }
            else if (it.nextRel()->direction == -1)
            {
                e.direction = UA_BROWSEDIRECTION_INVERSE;
            }
            else
            {
                e.direction = UA_BROWSEDIRECTION_BOTH;
            }
        }

        p.emplace_back(e);
        it++;
        idx++;
    }
    m_pathMatcher = std::make_unique<PathMatcher>(m_server, p, startIndex);
    m_path = path;
}

const column_t* FilterChain::results() const
{
    return m_pathMatcher->results().col();
}

const column_t* FilterChain::results(const std::string& identifier) const
{
    size_t idx = 0;
    for (const auto& n : m_path.nodes)
    {
        if (n.identifier && n.identifier == identifier)
        {
            return m_pathMatcher->results().col(idx);
        }
        idx++;
    }
    return nullptr;
}

size_t findStartIndex(const cypher::Path& p)
{
    auto idx = 0u;
    for (const auto& e : p.nodes)
    {
        if (e.identifier)
        {
            return idx;
        }
        idx += 1;
    }
    return 0u;
}

const column_t*
findSourceColumn(const std::string id,
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
createFilterChain(const cypher::Path& path,
                  std::vector<std::reference_wrapper<const FilterChain>> ctx,
                  UA_Server* server)
{
    auto start = findStartIndex(path);
    auto f = std::make_unique<FilterChain>(server);
    if (ctx.size() == 0)
    {
        f->createHierachicalVisitorSource(
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES),
        parseOptionalNodeClass(path.nodes[start].label));
    }
    else
    {
        // lookup the source column
        if (!path.nodes[start].identifier)
        {
            return nullptr;
        }

        auto col = findSourceColumn(*path.nodes[start].identifier, ctx);
        if (!col)
        {
            return nullptr;
        }
        f->createColumnAsSource(*col);
    }

    f->createReferenceFilter(path, start);
    return f;
}