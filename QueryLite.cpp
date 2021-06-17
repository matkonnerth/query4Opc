#include "QueryLite.h"
#include <iostream>



void QueryLite::iterate(UA_Server *server, const UA_NodeId &root, const UA_NodeId& referenceType,
             const FilterFunction &filter, std::vector<UA_NodeId> &results) {
  UA_BrowseDescription bd;
  UA_BrowseDescription_init(&bd);
  bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
  bd.includeSubtypes = true;
  bd.referenceTypeId = referenceType;
  // bd.resultMask = UA_BROWSERESULTMASK_ALL;
  bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION;
  bd.nodeId = root;
  bd.nodeClassMask =
      UA_NODECLASS_VARIABLE | UA_NODECLASS_OBJECT | UA_NODECLASS_OBJECTTYPE;
  // bd.nodeClassMask
  UA_BrowseResult br = UA_Server_browse(server, 1000, &bd);
  if (br.statusCode == UA_STATUSCODE_GOOD) {
    for (UA_ReferenceDescription *rd = br.references;
         rd != br.references + br.referencesSize; rd++) {

      if (filter(*rd)) {
        results.push_back(rd->nodeId.nodeId);
      }

      iterate(server, rd->nodeId.nodeId, referenceType, filter, results);
    }
  }
  UA_BrowseResult_clear(&br);
}

std::vector<UA_NodeId> QueryLite::lookupInstances(UA_Server *server,
                                       const UA_NodeId &rootId,
                                       const UA_NodeId &type) {
  std::vector<UA_NodeId> results;
  std::vector<UA_NodeId> typeIds;
  // add the root Type himself
  typeIds.push_back(type);

  FilterFunction alwaysTrueFilter = [](const UA_ReferenceDescription &ref) {
    return true;
  };
  iterate(server, type, UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), alwaysTrueFilter, typeIds);
  std::cout << "types found: " << typeIds.size() << "\n";

  FilterFunction typeFilter = [&](const UA_ReferenceDescription &rd) {
    for (const auto &type : typeIds) {
      if (UA_NodeId_equal(&rd.typeDefinition.nodeId, &type)) {
        return true;
      }
    }
    return false;
  };

  iterate(server, rootId, UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), typeFilter, results);
  return results;
}