#include <graph/HierachicalVisitor.h>

using graph::HierachicalVisitor;

HierachicalVisitor::HierachicalVisitor(UA_Server* server,
                                       const UA_NodeId& root,
                                       const UA_NodeId& referenceTypeId,
                                       UA_UInt32 nodeclasMask)
: m_server{ server }
, m_root{ root }
, m_referenceType{ referenceTypeId }
, m_nodeClassMask{ nodeclasMask }
{}

void HierachicalVisitor::generate(const std::function<void(path_element_t&&)>& filter) const
{
    UA_NodeClass nodeclass{};
    auto status = UA_Server_readNodeClass(m_server, m_root, &nodeclass);
    if (!UA_StatusCode_isGood(status))
    {
        std::cout << "root node not found \n";
        return;
    }
    UA_ReferenceDescription rd{};
    rd.nodeId.nodeId = m_root;
    rd.nodeClass = nodeclass;
    filter(ReferenceDescription{rd});
    visit(m_root, filter);
}

const UA_NodeId& HierachicalVisitor::startNode() const
{
    return m_root;
}

std::string HierachicalVisitor::explain() const
{
    std::string explanation{ "HierachicalVisitor\n" };
    explanation.append("startNode: ");
    UA_String id{};
    UA_NodeId_print(&startNode(), &id);
    std::string idString{};
    idString.assign((char*)id.data, id.length);
    explanation.append(idString);
    explanation.append("\n");
    return explanation;
}

UA_UInt32 HierachicalVisitor::calculateNodeClassMaskForBrowse(UA_UInt32 nodeClassToFind)
{
    switch (nodeClassToFind)
    {
    case UA_NODECLASS_OBJECT:
        return UA_NODECLASS_OBJECT | UA_NODECLASS_OBJECTTYPE;
    case UA_NODECLASS_VARIABLE:
        return UA_NODECLASS_VARIABLE | UA_NODECLASS_OBJECT |
               UA_NODECLASS_OBJECTTYPE | UA_NODECLASS_METHOD;
    case UA_NODECLASS_OBJECTTYPE:
        return UA_NODECLASS_OBJECTTYPE;
    case UA_NODECLASS_DATATYPE:
        return UA_NODECLASS_DATATYPE;
    case UA_NODECLASS_REFERENCETYPE:
        return UA_NODECLASS_REFERENCETYPE;
    default:
        return UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE | UA_NODECLASS_METHOD;
    }
    return UA_NODECLASS_UNSPECIFIED;
}

void HierachicalVisitor::visit(const UA_NodeId& root,
                               const std::function<void(path_element_t&&)>& filter) const
{
    UA_BrowseDescription bd;
    UA_BrowseDescription_init(&bd);
    bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    bd.includeSubtypes = true;
    bd.referenceTypeId = m_referenceType;
    // TODO: perfomance?
    bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION | UA_BROWSERESULTMASK_NODECLASS;
    bd.nodeId = root;
    bd.nodeClassMask = calculateNodeClassMaskForBrowse(m_nodeClassMask);
    browseSource();
    UA_BrowseResult br = UA_Server_browse(m_server, 1000, &bd);
    if (br.statusCode == UA_STATUSCODE_GOOD)
    {
        for (UA_ReferenceDescription* rd = br.references;
             rd != br.references + br.referencesSize;
             rd++)
        {
            if (rd->nodeClass == m_nodeClassMask)
            {
                filter(ReferenceDescription{*rd});
            }
            // TODO: performance, shouldn't browse variable again?
            visit(rd->nodeId.nodeId, filter);
        }
    }
    UA_BrowseResult_clear(&br);
}