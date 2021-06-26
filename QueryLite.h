#pragma once
#include <functional>
#include <open62541/server.h>
#include <vector>
#include "Filter.h"


class QueryLite {
public:
  static std::vector<Result> lookupInstances(UA_Server *server,
                                             const UA_NodeId &rootId,
                                             const UA_NodeId &type);

private:
  static void hierachicalVisit(UA_Server *server, const UA_NodeId &root,
                      const UA_NodeId &referenceType, AbstractFilter* filter);
};