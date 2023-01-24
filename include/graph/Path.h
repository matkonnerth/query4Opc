#pragma once
#include <cypher/Path.h>
#include "Helper.h"
#include <algorithm>

struct Node
{
    UA_NodeClass nodeClass;
    std::optional<UA_NodeId> id;
};

/*
   referenceType: the reference to match, UA_NODEID_NULL for any referenceType
*/
struct Relation
{
    UA_BrowseDirection direction;
    UA_NodeId referenceType;
};

static inline UA_BrowseDirection getBrowseDirection(const cypher::Relationship& r)
{
    if (r.direction == 1)
    {
        return UA_BROWSEDIRECTION_INVERSE;
    }
    else if (r.direction== -1)
    {
        return UA_BROWSEDIRECTION_FORWARD;
    }
    else
    {
        return UA_BROWSEDIRECTION_BOTH;
    }
    return UA_BROWSEDIRECTION_BOTH;
}



class Path
{
public:
   class PathIterator
   {
    public:
       PathIterator(const Path& p)
       : m_path{ p }
       {}

       const Node* next()
       {
           auto node = m_path.getNode(idx);
           ++idx;
           return node;
       }

       const Relation* RelationLHS() const
       {
           return m_path.RelationLHS(idx);
       }

       const Relation* RelationRHS() const
       {
           return m_path.RelationRHS(idx);
       }

    private:
       const Path& m_path;
       size_t idx{ 0 };
   };

   Path(){}

    Path(const cypher::Path& p)
    {
        for(const auto&cn : p.nodes)
        {
            Node n;
            n.id = parseOptionalNodeId(cn.NodeId());
            n.nodeClass = parseOptionalNodeClass(cn.label);
            m_nodes.push_back(n);
        }

        for(const auto& cr:p.relations)
        {
            Relation r;
            r.direction = getBrowseDirection(cr);
            r.referenceType = lookupReferenceType(cr.type);
            m_relations.push_back(r);
        }
    }

    Path(const std::vector<Node>& nodes, const std::vector<Relation>& relations):m_nodes{nodes}, m_relations{relations}{}

    size_t size() const
    {
        return m_nodes.size();
    }

    bool empty() const
    {
        return m_nodes.empty();
    }

    const Node* getNode(size_t idx) const
    {
        if (idx < m_nodes.size())
        {
            return &m_nodes[idx];
        }
        return nullptr;
    }

    const Relation* RelationLHS(size_t idx) const
    {
        int relIdx = static_cast<int>(idx)-1;
        if(relIdx>=0 && relIdx < static_cast<int>(m_relations.size()))
        {
            return &m_relations[relIdx];
        }
        return nullptr;
    }

    const Relation* RelationRHS(size_t idx) const
    {
        if (idx < m_relations.size())
        {
            return &m_relations[idx];
        }
        return nullptr;
    }

    PathIterator it() const
    {
        return PathIterator(*this);
    }

    void invertBrowseDirections(size_t idx)
    {
        for(auto it = m_relations.begin()+idx; it!=m_relations.end(); ++it)
        {
            if(it->direction==UA_BROWSEDIRECTION_FORWARD)
            {
                it->direction=UA_BROWSEDIRECTION_INVERSE;
            }
            else if(it->direction==UA_BROWSEDIRECTION_INVERSE)
            {
                it->direction=UA_BROWSEDIRECTION_FORWARD;
            }
        }
    }

    std::pair<Path, Path> split(size_t idx) const
    {
        Path lhs{std::vector<Node>{m_nodes.cbegin(),m_nodes.cbegin()+idx}, std::vector<Relation>{m_relations.cbegin(), m_relations.cbegin()+idx}};
        Path rhs{std::vector<Node>{m_nodes.cbegin()+idx, m_nodes.cend()}, std::vector<Relation>{m_relations.cbegin()+idx, m_relations.cend()}};
        return std::make_pair(lhs, rhs);
    }

    void invert()
    {
        std::reverse(m_nodes.begin(), m_nodes.end());
        std::reverse(m_relations.begin(), m_relations.end());
        invertBrowseDirections(0);
    }


private:
    std::vector<Node> m_nodes{};
    std::vector<Relation> m_relations{};
};

