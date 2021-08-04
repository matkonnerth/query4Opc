#pragma once
#include <open62541/server.h>
#include <vector>
#include "Sink.h"

/*
template <typename Derived>
struct TypeDefinitionTrait {
  UA_NodeId typeDefinition() {
    Derived *d = static_cast<Derived *>(this);
    return d->typeDefinition();
  }
};
*/

struct Result {
  UA_NodeId parentId;
  UA_ReferenceDescription target;
};

template <typename T> class AbstractFilter {
public:
  virtual void filter(T &&input) = 0;
  virtual AbstractFilter<T>& append(AbstractFilter<T>& filter)=0;
  virtual void append(Sink<T>& sink) = 0;
  virtual ~AbstractFilter() = default;
};

template <typename T, typename Derived>
class Filter : public AbstractFilter<T> {
public:
  void filter(T &&input) override {
    if (match(input)) {
      if(m_nextFilter)
      {
        m_nextFilter->filter(std::move(input));
      }
      else
      {
        m_sink->emplace_back(std::move(input));
      }      
    }
  }

  /*
    append a filter, returns the appended filter
  */
  AbstractFilter<T> &append(AbstractFilter<T> &filter) {
    m_nextFilter = &filter;
    return *m_nextFilter;
  }

  /*
  */
  void append(Sink<T>& s) { m_sink = &s;}

private:
  bool match(const T &input) {
    Derived *d = static_cast<Derived *>(this);
    return d->match(input);
  }
  AbstractFilter<T> *m_nextFilter{nullptr};
  Sink<T>* m_sink{nullptr};
};



template <typename T> class TakeAllFilter : public Filter<T, TakeAllFilter<T>> {
public:
  bool match(const T &input) { return true; }
};

/*
  matches nodes which have one of the given typedefinitions
*/
template <typename T> class TypeFilter : public Filter<T, TypeFilter<T>> {
public:
  /*
    construct TypeFilter with a single typeId
  */
  TypeFilter(const UA_NodeId& typeId)
  {
    UA_ReferenceDescription typeRef{};
    typeRef.nodeId.nodeId = typeId;
    m_types.emplace_back(Result{UA_NODEID_NUMERIC(0, 0), typeRef});
  }
  TypeFilter(const std::vector<T> &types) : m_types{types} {}

  bool match(const T &input) {
    for (const auto &type : m_types) {
      if (UA_NodeId_equal(&type.target.nodeId.nodeId,
                          &input.target.typeDefinition.nodeId)) {
        return true;
      }
    }
    return false;
  }

private:
  std::vector<T> m_types;
};

/*
  matches nodes which have references with a certain type
*/
template <typename T>
class ReferenceFilter : public Filter<T, ReferenceFilter<T>> {
public:
  ReferenceFilter(UA_Server *server, const UA_NodeId &referenceType)
      : m_server{server}, m_referenceType{referenceType} {}

  bool match(const T &input) {
    UA_BrowseDescription bd;
    UA_BrowseDescription_init(&bd);
    bd.browseDirection = UA_BROWSEDIRECTION_BOTH;
    bd.includeSubtypes = true;
    bd.referenceTypeId = m_referenceType;
    bd.resultMask = UA_BROWSERESULTMASK_NONE;
    bd.nodeId = input.target.nodeId.nodeId;
    bd.nodeClassMask = UA_NODECLASS_UNSPECIFIED;
    UA_BrowseResult br = UA_Server_browse(m_server, 1000, &bd);
    auto result = false;
    if (br.statusCode == UA_STATUSCODE_GOOD) {
      if (br.referencesSize > 0) {
        result = true;
      }
      else
      {
        result = false;
      }      
    }
    UA_BrowseResult_clear(&br);
    return result;
  }

private:
  UA_Server *m_server;
  UA_NodeId m_referenceType;
};

/*
class AttributeFilter : public Filter<AttributeFilter> {
public:
  AttributeFilter(UA_Server *server) : m_server{server} {}
  bool match(const Result &input) {
    UA_ReadValueId valId;
    UA_ReadValueId_init(&valId);
    valId.attributeId = UA_ATTRIBUTEID_EVENTNOTIFIER;
    valId.nodeId = input.target.nodeId.nodeId;
    auto val = UA_Server_read(m_server, &valId, UA_TIMESTAMPSTORETURN_NEITHER);
    if (val.status != UA_STATUSCODE_GOOD) {
      return false;
    }
    if (!val.hasValue) {
      return false;
    }
    if (val.value.type = &UA_TYPES[UA_TYPES_BYTE]) {
      auto *e = (UA_Byte *)val.value.data;
      if (*e != 0) {
        return true;
      }
    }
    return false;
  }

private:
  UA_Server *m_server;
};
*/