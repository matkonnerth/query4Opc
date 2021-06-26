#pragma once
#include <open62541/server.h>
#include <vector>

struct Result {
  UA_NodeId parentId;
  UA_ReferenceDescription target;
};

template<typename T>
class AbstractFilter {
public:
  virtual void filter(T &&input) = 0;
  virtual ~AbstractFilter() = default;
};

template<typename T>
class Sink : public AbstractFilter<T> {
public:
  void filter(T &&r) override { res.emplace_back(r); }

  const auto &results() const { return res; }
  auto &results() { return res; }

private:
  std::vector<T> res;
};

template <typename T, typename Derived> class Filter : public AbstractFilter<T>{
public:
  void filter(T &&input) override {
    if (match(input)) {
      m_nextFilter->filter(std::move(input));
    }
  }

  void connect(AbstractFilter<T> *filter) { m_nextFilter = filter; }

private:
  bool match(const T &input) {
    Derived *d = static_cast<Derived *>(this);
    return d->match(input);
  }
  AbstractFilter<T> *m_nextFilter;
};

template<typename T>
class TakeAllFilter : public Filter<T, TakeAllFilter<T>> {
public:
  bool match(const T&input) { return true; }
};

template<typename T>
class TypeFilter : public Filter<T, TypeFilter<T>> {
public:
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