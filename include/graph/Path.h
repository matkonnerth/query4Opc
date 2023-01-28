#pragma once
#include <cypher/Path.h>
#include "Helper.h"
#include <algorithm>

namespace graph
{
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
        return UA_BROWSEDIRECTION_FORWARD;
    }
    else if (r.direction== -1)
    {
        return UA_BROWSEDIRECTION_INVERSE;
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
           ++idx;
           return m_path.getNode(idx);
       }

       const Node* current()
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

    Path();
    Path(const cypher::Path& p);
    Path(const std::vector<Node>& nodes, const std::vector<Relation>& relations);

    size_t size() const;
    bool empty() const;

    const Node* getNode(size_t idx) const;
    const Relation* RelationLHS(size_t idx) const;
    const Relation* RelationRHS(size_t idx) const;
    PathIterator it() const;
    void invertBrowseDirections(size_t idx);
    std::pair<Path, Path> split(int idx) const;

    void transformForLeftToRightTraversal();
    

private:
   void invert();
   std::vector<Node> m_nodes{};
   std::vector<Relation> m_relations{};
};
}
