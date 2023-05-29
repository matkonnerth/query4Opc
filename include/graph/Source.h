#pragma once
#include "Types.h"
#include "tracing.h"
#include <functional>
#include <iostream>
#include <open62541/server.h>
#include <vector>

namespace graph {
class Source
{
 public:
    virtual void generate(const std::function<void(path_element_t&&)>& filter) const = 0;
    virtual std::string explain() const = 0;
    virtual ~Source() = default;
};

inline std::optional<path_element_t>
getInverseHierachicalReference(UA_Server* server, const UA_NodeId& node)
{
    UA_BrowseDescription bd;
    UA_BrowseDescription_init(&bd);
    bd.browseDirection = UA_BROWSEDIRECTION_INVERSE;
    bd.includeSubtypes = true;
    bd.referenceTypeId = UA_NODEID_NUMERIC(0u, UA_NS0ID_HIERARCHICALREFERENCES);
    // TODO: perfomance?
    bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION | UA_BROWSERESULTMASK_NODECLASS;
    bd.nodeId = node;
    bd.nodeClassMask = UA_NODECLASS_UNSPECIFIED;
    UA_BrowseResult br = UA_Server_browse(server, 1000, &bd);
    if (br.statusCode == UA_STATUSCODE_GOOD)
    {
        for (UA_ReferenceDescription* rd = br.references;
             rd != br.references + br.referencesSize;
             rd++)
        {
            auto result = *rd;
            UA_BrowseResult_clear(&br);
            return result;
        }
    }
    UA_BrowseResult_clear(&br);
    return std::nullopt;
}

const int MAX_PATH_LENGTH = 100;

inline path_t getPathToParentNode(UA_Server* server, const UA_NodeId& node)
{
    path_t path{};
    auto actNode = node;
    for (int i = 0; i < MAX_PATH_LENGTH; ++i)
    {
        auto parent = getInverseHierachicalReference(server, actNode);
        if (!parent)
        {
            break;
        }
        actNode = parent->nodeId.nodeId;
        path.emplace_back(std::move(*parent));
    }
    return path;
}
} // namespace graph
