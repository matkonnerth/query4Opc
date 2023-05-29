#pragma once
#include "Source.h"

namespace graph {

class HierachicalVisitor : public Source
{
 public:
    HierachicalVisitor(UA_Server* server,
                       const UA_NodeId& root,
                       const UA_NodeId& referenceTypeId,
                       UA_UInt32 nodeclasMask);

    void generate(const std::function<void(path_element_t&&)>& filter) const override;
    const UA_NodeId& startNode() const;
    std::string explain() const override;

    static UA_UInt32 calculateNodeClassMaskForBrowse(UA_UInt32 interestingNodeClass);

 private:
    void visit(const UA_NodeId& root, const std::function<void(path_element_t&&)>& filter) const;
    UA_Server* m_server;
    const UA_NodeId m_root;
    const UA_NodeId m_referenceType;
    UA_UInt32 m_nodeClassMask{ UA_NODECLASS_UNSPECIFIED };
};

} // namespace graph