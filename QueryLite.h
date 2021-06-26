#pragma once
#include "Filter.h"
#include <functional>
#include <open62541/server.h>
#include <vector>

/*
 */
class QueryLite {
public:
  static std::vector<Result> lookupInstances(UA_Server *server,
                                             const UA_NodeId &rootId,
                                             const UA_NodeId &type);

private:
  static Sink<Result> getTypes(UA_Server *server, const UA_NodeId &type);
  static void hierachicalVisit(UA_Server *server, const UA_NodeId &root,
                               const UA_NodeId &referenceType,
                               AbstractFilter<Result> *filter);
};