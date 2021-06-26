#include "QueryLite.h"
#include <iostream>

void QueryLite::hierachicalVisit(UA_Server *server, const UA_NodeId &root,
                                 const UA_NodeId &referenceType,
                                 AbstractFilter *filter) {
  UA_BrowseDescription bd;
  UA_BrowseDescription_init(&bd);
  bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
  bd.includeSubtypes = true;
  bd.referenceTypeId = referenceType;
  //bd.resultMask = UA_BROWSERESULTMASK_ALL;
  bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION;
  bd.nodeId = root;
  bd.nodeClassMask =
      UA_NODECLASS_OBJECT | UA_NODECLASS_OBJECTTYPE;
  // bd.nodeClassMask
  UA_BrowseResult br = UA_Server_browse(server, 1000, &bd);
  if (br.statusCode == UA_STATUSCODE_GOOD) {
    for (UA_ReferenceDescription *rd = br.references;
         rd != br.references + br.referencesSize; rd++) {

      filter->filter(root, *rd);

      hierachicalVisit(server, rd->nodeId.nodeId, referenceType, filter);
    }
  }
  UA_BrowseResult_clear(&br);
}

std::vector<Result> QueryLite::lookupInstances(UA_Server *server,
                                               const UA_NodeId &rootId,
                                               const UA_NodeId &type) {
  TakeAllFilter takeAll{};

  hierachicalVisit(server, type, UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE),
                   &takeAll);
  takeAll.results().push_back(type);
  std::cout << "types found: " << takeAll.results().size() << "\n";

  TypeFilter instancesOfType{takeAll.results()};

  hierachicalVisit(server, rootId,
                   UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES),
                   &instancesOfType);
  return instancesOfType.results();
}