#pragma once
#include <functional>
#include <open62541/server.h>
#include <vector>

struct Result
{
    UA_NodeId parentId;
    UA_ReferenceDescription target;
};
class QueryLite {
public:

template<typename T>
 using FilterFunction =
      std::function<void(const UA_NodeId& parentId, const UA_ReferenceDescription &ref, std::vector<T>& results)>;



  static std::vector<Result> lookupInstances(UA_Server *server,
                                                    const UA_NodeId &rootId,
                                                    const UA_NodeId &type);

private:
    template<typename T>
  static void iterate(UA_Server *server, const UA_NodeId &root,
                                 const UA_NodeId &referenceType,
                                 const FilterFunction<T> &filter,
                                 std::vector<T> &results);
};