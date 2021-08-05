#pragma once
#include "Filter.h"
#include "Sink.h"
#include <open62541/server.h>

class Source
{
public:
   virtual void generate(AbstractFilter<Result>& filter) const = 0;
   virtual ~Source() = default;
};

class HierachicalVisitor : public Source
{
public:
   HierachicalVisitor(UA_Server* server, const UA_NodeId& root, const UA_NodeId& referenceTypeId, UA_UInt32 nodeclasMask)
   : m_server{ server }
   , m_root{ root }
   , m_referenceType{ referenceTypeId }
   , m_nodeClassMask{ nodeclasMask }
   {}

   void generate(AbstractFilter<Result>& filter) const override
   {
      visit(m_root, filter);
   }

private:
   UA_UInt32 calculateNodeClassMaskForBrowse() const
   {
      switch (m_nodeClassMask)
      {
      case UA_NODECLASS_OBJECT:
         return UA_NODECLASS_OBJECT | UA_NODECLASS_OBJECTTYPE;
      case UA_NODECLASS_VARIABLE:
         return UA_NODECLASS_VARIABLE | UA_NODECLASS_OBJECT | UA_NODECLASS_OBJECTTYPE | UA_NODECLASS_METHOD;
      default:
         return UA_NODECLASS_UNSPECIFIED;
      }
      return UA_NODECLASS_UNSPECIFIED;
   }
   void visit(const UA_NodeId& root, AbstractFilter<Result>& filter) const
   {
      UA_BrowseDescription bd;
      UA_BrowseDescription_init(&bd);
      bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
      bd.includeSubtypes = true;
      bd.referenceTypeId = m_referenceType;
      //TODO: perfomance?
      bd.resultMask = UA_BROWSERESULTMASK_ALL;
      // bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION;
      bd.nodeId = root;
      bd.nodeClassMask = calculateNodeClassMaskForBrowse();
      UA_BrowseResult br = UA_Server_browse(m_server, 1000, &bd);
      if (br.statusCode == UA_STATUSCODE_GOOD)
      {
         for (UA_ReferenceDescription* rd = br.references; rd != br.references + br.referencesSize; rd++)
         {
            if (rd->nodeClass == m_nodeClassMask)
            {
               filter.filter(Result{ root, *rd });
            }
            //TODO: performance, shouldn't browse variable again?
            visit(rd->nodeId.nodeId, filter);
         }
      }
      UA_BrowseResult_clear(&br);
   }
   UA_Server* m_server;
   const UA_NodeId m_root;
   const UA_NodeId m_referenceType;
   UA_UInt32 m_nodeClassMask{ UA_NODECLASS_UNSPECIFIED };
};

class SinkToSource : public Source
{
public:
   SinkToSource(const Sink<Result>& s)
   : m_sink{ s }
   {}

   void generate(AbstractFilter<Result>& filter) const override
   {
      // TODO: unnecessary copy?
      for (auto res : m_sink.results())
      {
         filter.filter(std::move(res));
      }
   }

private:
   const Sink<Result>& m_sink;
};