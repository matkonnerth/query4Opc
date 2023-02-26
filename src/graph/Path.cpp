#include <graph/Path.h>

using namespace graph;

Path::Path()
{}

Path::Path(const cypher::Path& p)
{
    for (const auto& cn : p.nodes)
    {
        Node n;
        n.id = parseOptionalNodeId(cn.NodeId());
        n.typeDefinitionId = parseOptionalNodeId(cn.TypeDefinitionId());
        n.nodeClass = parseOptionalNodeClass(cn.label);
        m_nodes.push_back(n);
    }

    for (const auto& cr : p.relations)
    {
        Relation r;
        r.direction = getBrowseDirection(cr);
        r.referenceType = lookupReferenceType(cr.type);
        m_relations.push_back(r);
    }
}

Path::Path(const std::vector<Node>& nodes, const std::vector<Relation>& relations)
: m_nodes{ nodes }
, m_relations{ relations }
{}

size_t Path::size() const
{
    return m_nodes.size();
}

bool Path::empty() const
{
    return m_nodes.empty();
}

const Node* Path::getNode(size_t idx) const
{
    if (idx < m_nodes.size())
    {
        return &m_nodes[idx];
    }
    return nullptr;
}

const Relation* Path::RelationLHS(size_t idx) const
{
    if (idx > 0 && idx < m_relations.size() + 1 )
    {
        return &m_relations[idx-1];
    }
    return nullptr;
}

const Relation* Path::RelationRHS(size_t idx) const
{
    if (idx < m_relations.size())
    {
        return &m_relations[idx];
    }
    return nullptr;
}

Path::PathIterator Path::it() const
{
    return Path::PathIterator(*this);
}

void Path::invertBrowseDirections(size_t idx)
{
    auto start = m_relations.begin();
    std::advance(start, idx);
    std::for_each(start, m_relations.end(), [](auto& rel) {
        if (rel.direction == UA_BROWSEDIRECTION_FORWARD)
        {
            rel.direction = UA_BROWSEDIRECTION_INVERSE;
        }
        else if (rel.direction == UA_BROWSEDIRECTION_INVERSE)
        {
            rel.direction = UA_BROWSEDIRECTION_FORWARD;
        }
    });
}

std::pair<Path, Path> Path::split(int idx) const
{
    Path lhs{ std::vector<Node>{ m_nodes.cbegin(), m_nodes.cbegin() + idx },
              std::vector<Relation>{ m_relations.cbegin(), m_relations.cbegin() + idx } };
    Path rhs{ std::vector<Node>{ m_nodes.cbegin() + idx, m_nodes.cend() },
              std::vector<Relation>{ m_relations.cbegin() + idx, m_relations.cend() } };
    return std::make_pair(lhs, rhs);
}

void Path::invert()
{
    std::reverse(m_nodes.begin(), m_nodes.end());
    std::reverse(m_relations.begin(), m_relations.end());
    invertBrowseDirections(0);
}

void Path::transformForLeftToRightTraversal()
{
    if(empty())
    {
        return;
    }
    //dummy node
    m_nodes.emplace_back(Node{});
    invert();
}