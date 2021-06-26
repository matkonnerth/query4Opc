#pragma once
#include <open62541/server.h>
#include <vector>

struct Result {
  UA_NodeId parentId;
  UA_ReferenceDescription target;
};

class AbstractFilter {
public:
  virtual void filter(Result &&input) = 0;
  virtual ~AbstractFilter() = default;
};

class Sink : public AbstractFilter {
public:
  void filter(Result &&r) override { res.emplace_back(r); }

  const auto &results() const { return res; }
  auto &results() { return res; }

private:
  std::vector<Result> res;
};

template <typename Derived> class Filter : public AbstractFilter {
public:
  void filter(Result &&input) override {
    if (match(input)) {
      m_nextFilter->filter(std::move(input));
    }
  }

  void connect(AbstractFilter *filter) { m_nextFilter = filter; }

private:
  bool match(const Result &input) {
    Derived *d = static_cast<Derived *>(this);
    return d->match(input);
  }
  AbstractFilter *m_nextFilter;
};

class TakeAllFilter : public Filter<TakeAllFilter> {
public:
  bool match(const Result &input) { return true; }
};

class TypeFilter : public Filter<TypeFilter> {
public:
  TypeFilter(const std::vector<Result> &types) : m_types{types} {}

  bool match(const Result &input) {
    for (const auto &type : m_types) {
      if (UA_NodeId_equal(&type.target.nodeId.nodeId,
                          &input.target.typeDefinition.nodeId)) {
        return true;
      }
    }
    return false;
  }

private:
  std::vector<Result> m_types;
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