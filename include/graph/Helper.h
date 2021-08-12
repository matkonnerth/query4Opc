#pragma once
#include <open62541/types.h>
#include <string>
#include <unordered_map>

UA_NodeClass parseNodeClass(const std::string& nodeclass)
{
    static const std::unordered_map<std::string, UA_NodeClass> m{
        { "Object", UA_NODECLASS_OBJECT },
        { "Variable", UA_NODECLASS_VARIABLE },
        { "ObjectType", UA_NODECLASS_OBJECTTYPE },
        { "Method", UA_NODECLASS_METHOD },
        { "DataType", UA_NODECLASS_DATATYPE },
        { "View", UA_NODECLASS_VIEW },
        { "VariableType", UA_NODECLASS_VARIABLETYPE},
        { "ReferenceType", UA_NODECLASS_REFERENCETYPE}
    };

    auto it = m.find(nodeclass);
    if (it != m.end())
    {
        return it->second;
    }
    return UA_NODECLASS_UNSPECIFIED;
}

UA_NodeClass parseOptionalNodeClass(const std::optional<std::string>& nodeclass)
{
    if (!nodeclass)
    {
        return UA_NODECLASS_UNSPECIFIED;
    }
    return parseNodeClass(*nodeclass);
}

UA_NodeId parseNodeId(const std::string& id)
{
    return UA_NODEID(id.c_str());
}

UA_NodeId parseOptionalNodeId(const std::optional<std::string>& id)
{
    if (!id)
    {
        return UA_NODEID_NULL;
    }
    return parseNodeId(*id);
}

UA_NodeId parseOptionalNodeId(const std::string* id)
{
    if (!id)
    {
        return UA_NODEID_NULL;
    }
    return parseNodeId(*id);
}

UA_NodeId lookupReferenceType(const std::optional<std::string>& ref)
{
    if (!ref)
    {
        return UA_NODEID_NULL;
    }
    static const std::unordered_map<std::string, UA_NodeId> m{
        { "HasTypeDefinition", UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION) },
        { "HasSubType", UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE) },
        { "HasProperty", UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY) },
        { "HasComponent", UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT)}
    };
    auto it = m.find(*ref);
    if (it != m.end())
    {
        return it->second;
    }
    return UA_NODEID_NULL;
}
