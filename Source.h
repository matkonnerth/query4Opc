#pragma once
#include "Filter.h"
#include <open62541/server.h>

class Source {
public:
  virtual void generate(AbstractFilter<Result> &filter) = 0;
  virtual ~Source() = default;
};

class HierachicalVisitor : public Source {
public:
  HierachicalVisitor(UA_Server *server, const UA_NodeId &root)
      : m_server{server}, m_root{root}, m_referenceType{UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES)} {}

  HierachicalVisitor(UA_Server *server, const UA_NodeId &root, const UA_NodeId& referenceTypeId)
      : m_server{server}, m_root{root},
        m_referenceType{referenceTypeId} {
  }

  void generate(AbstractFilter<Result> &filter) override {
    visit(m_root, filter);
  }

private:
  void visit(const UA_NodeId &root, AbstractFilter<Result> &filter) {
    UA_BrowseDescription bd;
    UA_BrowseDescription_init(&bd);
    bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    bd.includeSubtypes = true;
    bd.referenceTypeId = m_referenceType;
    // bd.resultMask = UA_BROWSERESULTMASK_ALL;
    bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION;
    bd.nodeId = root;
    bd.nodeClassMask = UA_NODECLASS_OBJECT | UA_NODECLASS_OBJECTTYPE;
    UA_BrowseResult br = UA_Server_browse(m_server, 1000, &bd);
    if (br.statusCode == UA_STATUSCODE_GOOD) {
      for (UA_ReferenceDescription *rd = br.references;
           rd != br.references + br.referencesSize; rd++) {

        filter.filter(Result{root, *rd});

        visit(rd->nodeId.nodeId, filter);
      }
    }
    UA_BrowseResult_clear(&br);
  }
  UA_Server *m_server;
  const UA_NodeId m_root;
  const UA_NodeId m_referenceType;
};