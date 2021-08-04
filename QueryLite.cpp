#include "QueryLite.h"
#include <iostream>
#include "Source.h"

Sink<Result> QueryLite::getTypes(UA_Server *server, const UA_NodeId &type) {
  Sink<Result> types;

  HierachicalVisitor vis{server, type,
                         UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE)};

  TakeAllFilter<Result> ta{};
  ta.append(types);

  vis.generate(ta);
  UA_ReferenceDescription typeRef{};
  typeRef.nodeId.nodeId = type;
  types.results().push_back(Result{UA_NODEID_NUMERIC(0, 0), typeRef});
  std::cout << "types found: " << types.results().size() << "\n";
  return types;
}

std::vector<Result> QueryLite::lookupInstances(UA_Server *server,
                                               const UA_NodeId &rootId,
                                               const UA_NodeId &type) {

  auto types = getTypes(server, type);
  TypeFilter<Result> instancesOfType{types.results()};
  Sink<Result> instances;
  instancesOfType.append(instances);

  HierachicalVisitor vis{server, rootId,
                         UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES)};

  vis.generate(instancesOfType);

  return instances.results();
}