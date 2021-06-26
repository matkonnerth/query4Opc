#pragma once
#include <functional>
#include <open62541/server.h>
#include <vector>

struct Result {
  UA_NodeId parentId;
  UA_ReferenceDescription target;
};

class AbstractFilter {
public:
  virtual void filter(const UA_NodeId &parentId,
                      const UA_ReferenceDescription &ref) = 0;
  virtual ~AbstractFilter() = default;
};

/*
template <typename Derived>
class Matcher
{
public:
    bool match(const UA_NodeId&parentId, const UA_ReferenceDescription& ref)
    {
      Derived *d = static_cast<Derived *>(this);
      return d->match(parentId, ref);
    }
}
*/

template <typename C, typename Derived> class Filter : public AbstractFilter {
public:
  void filter(const UA_NodeId &parentId,
              const UA_ReferenceDescription &ref) override {
    if (match(parentId, ref)) {
      emplace(ref.nodeId.nodeId, ref, container);
    }
  }

  const auto &results() const { return container; }
  auto &results() { return container; }

private:
  bool match(const UA_NodeId &parentId, const UA_ReferenceDescription &ref) {
    Derived *d = static_cast<Derived *>(this);
    return d->match(parentId, ref);
  }

  void emplace(const UA_NodeId &parentId, const UA_ReferenceDescription &ref,
               C &container) {
    Derived *d = static_cast<Derived *>(this);
    d->emplace(parentId, ref, container);
  }

protected:
  C container;
};

class TakeAllFilter : public Filter<std::vector<UA_NodeId>, TakeAllFilter> {
public:
  bool match(const UA_NodeId &parentId, const UA_ReferenceDescription &ref) {
    return true;
  }

  void emplace(const UA_NodeId &parentId, const UA_ReferenceDescription &ref,
               std::vector<UA_NodeId> &container) {
    container.emplace_back(ref.nodeId.nodeId);
  }
};

class TypeFilter : public Filter<std::vector<Result>, TypeFilter> {
public:
  TypeFilter(const std::vector<UA_NodeId> &types):m_types{types} {}

  bool match(const UA_NodeId &parentId, const UA_ReferenceDescription &ref) {
    for (const auto &id : m_types) {
      if (UA_NodeId_equal(&id, &ref.typeDefinition.nodeId)) {
        return true;
      }
    }
    return false;
  }

  void emplace(const UA_NodeId &parentId, const UA_ReferenceDescription &ref,
               std::vector<Result> &container) {
    container.emplace_back(Result{parentId, ref.nodeId.nodeId});
  }

private:
  std::vector<UA_NodeId> m_types;
};

class AttributeFilter: public Filter<std::vector<Result>, AttributeFilter>
{
public:
    //AttributeFilter()
    bool match(const UA_NodeId& parentId, const UA_ReferenceDescription& ref)
    {
        //UA_ReadValueId
        UA_ReadValueId valId;
        UA_ReadValueId_init(&valId);
        valId.attributeId = UA_ATTRIBUTEID_EVENTNOTIFIER;
        valId.nodeId = ref.nodeId.nodeId;
        auto val = UA_Server_read(m_server, &valId, UA_TIMESTAMPSTORETURN_NEITHER);
        if(val.status!=UA_STATUSCODE_GOOD)
        {
            return false;
        }
        if(!val.hasValue)
        {
            return false;
        }
        if(val.value.type = &UA_TYPES[UA_TYPES_BYTE])
        {
            auto* e = (UA_Byte*)val.value.data;
            if(*e!=0)
            {
                return true;
            }
        }
        return false;
    }

private:
    UA_Server* m_server;
};