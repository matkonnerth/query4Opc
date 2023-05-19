#pragma once
#include "Helper.h"
#include "Types.h"
#include <algorithm>
#include <cypher/Path.h>

namespace graph {

struct Node
{
    UA_NodeClass nodeClass;
    std::optional<UA_NodeId> id;
    std::optional<UA_NodeId> typeDefinitionId;
};

/*
   referenceType: the reference to match, UA_NODEID_NULL for any referenceType
*/
struct Relation
{
    UA_BrowseDirection direction;
    UA_NodeId referenceType;
};

static inline bool
is_matching_NodeId(const std::optional<UA_NodeId>& optId, const UA_NodeId& id)
{
    if (!optId)
    {
        return true;
    }
    return UA_NodeId_equal(&(optId.value()), &id);
}

static inline bool is_matching_NodeClass(UA_NodeClass current, UA_NodeClass expected)
{
    return current == expected || current == UA_NODECLASS_UNSPECIFIED;
}

static inline bool is_matching(const path_element_t& ref, const struct Node& node)
{
    return is_matching_NodeId(node.id, ref.nodeId.nodeId) &&
           is_matching_NodeClass(node.nodeClass, ref.nodeClass) &&
           is_matching_NodeId(node.typeDefinitionId, ref.typeDefinition.nodeId);
}

static inline UA_BrowseDirection getBrowseDirection(const cypher::Relationship& r)
{
    if (r.direction == 1)
    {
        return UA_BROWSEDIRECTION_FORWARD;
    }
    else if (r.direction == -1)
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
} // namespace graph
