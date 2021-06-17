#include "QueryLite.h"
#include <iostream>


template<typename T>
void QueryLite::iterate(UA_Server *server, const UA_NodeId &root, const UA_NodeId& referenceType,
             const FilterFunction<T> &filter, std::vector<T> &results) {
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

    filter(root, *rd, results);
      

      iterate(server, rd->nodeId.nodeId, referenceType, filter, results);
    }
  }
  UA_BrowseResult_clear(&br);
}

std::vector<Result> QueryLite::lookupInstances(UA_Server *server,
                                       const UA_NodeId &rootId,
                                       const UA_NodeId &type) {
  std::vector<Result> results;
  std::vector<UA_NodeId> typeIds;
  // add the root Type himself
  typeIds.push_back(type);

  FilterFunction<UA_NodeId> takeAll = [](const UA_NodeId& parent, const UA_ReferenceDescription &ref, std::vector<UA_NodeId>& res) {
    res.push_back(ref.nodeId.nodeId);
  };
  iterate(server, type, UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), takeAll, typeIds);
  std::cout << "types found: " << typeIds.size() << "\n";

  FilterFunction<Result> onlyInstancesWithType = [&](const UA_NodeId &parent,
                                             const UA_ReferenceDescription &rd,
                                             std::vector<Result> &res) {
    for (const auto &type : typeIds) {
      if (UA_NodeId_equal(&rd.typeDefinition.nodeId, &type)) {
        res.push_back(Result{parent, rd});
      }
    }
    return false;
  };

  iterate(server, rootId, UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES),
          onlyInstancesWithType, results);
  return results;
}