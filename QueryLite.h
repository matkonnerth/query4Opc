#pragma once
#include <functional>
#include <open62541/server.h>
#include <vector>

class QueryLite {
public:
  using FilterFunction =
      std::function<bool(const UA_ReferenceDescription &ref)>;

  static std::vector<UA_NodeId> lookupInstances(UA_Server *server,
                                                    const UA_NodeId &rootId,
                                                    const UA_NodeId &type);

private:
  static void iterate(UA_Server *server, const UA_NodeId &root,
                                 const UA_NodeId &referenceType,
                                 const FilterFunction &filter,
                                 std::vector<UA_NodeId> &results);
};