#pragma once
#include <open62541/server.h>
#include <functional>
#include <vector>
#include "Types.h"
#include "tracing.h"

namespace graph
{
class Source
{
public:
   virtual void generate(const std::function<void(path_element_t&&)>& filter) const = 0;
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

   void generate(const std::function<void(path_element_t&&)>& filter) const override
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
   void visit(const UA_NodeId& root, const std::function<void(path_element_t&&)>& filter) const
   {
      UA_BrowseDescription bd;
      UA_BrowseDescription_init(&bd);
      bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
      bd.includeSubtypes = true;
      bd.referenceTypeId = m_referenceType;
      //TODO: perfomance?
      bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION |
                      UA_BROWSERESULTMASK_NODECLASS;
      bd.nodeId = root;
      bd.nodeClassMask = calculateNodeClassMaskForBrowse();
      browseSource();
      UA_BrowseResult br = UA_Server_browse(m_server, 1000, &bd);
      if (br.statusCode == UA_STATUSCODE_GOOD)
      {
         for (UA_ReferenceDescription* rd = br.references; rd != br.references + br.referencesSize; rd++)
         {
            if (rd->nodeClass == m_nodeClassMask)
            {
               filter(std::move(*rd));
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

class ColumnAsSource : public Source
{
public:
   ColumnAsSource(const column_t& c)
   : col{ c }
   {}

   void generate(const std::function<void(path_element_t&&)>& filter) const override
   {
      // TODO: unnecessary copy?
      for (auto pe : col)
      {
         filter(std::move(pe));
      }
   }

private:
   const column_t& col;
};

inline std::optional<path_element_t> getInverseHierachicalReference(UA_Server* server, const UA_NodeId& node)
{
    UA_BrowseDescription bd;
    UA_BrowseDescription_init(&bd);
    bd.browseDirection = UA_BROWSEDIRECTION_INVERSE;
    bd.includeSubtypes = true;
    bd.referenceTypeId = UA_NODEID_NUMERIC(0u, UA_NS0ID_HIERARCHICALREFERENCES);
    // TODO: perfomance?
    bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION | UA_BROWSERESULTMASK_NODECLASS;
    bd.nodeId = node;
    bd.nodeClassMask = UA_NODECLASS_UNSPECIFIED;
    UA_BrowseResult br = UA_Server_browse(server, 1000, &bd);
    if (br.statusCode == UA_STATUSCODE_GOOD)
    {
        for (UA_ReferenceDescription* rd = br.references;
             rd != br.references + br.referencesSize;
             rd++)
        {
            auto result = *rd;
            UA_BrowseResult_clear(&br);
            return result;
        }
    }
    UA_BrowseResult_clear(&br);
    return std::nullopt;
}

const int MAX_PATH_LENGTH=100;

inline path_t getPathToParentNode(UA_Server* server, const UA_NodeId& node)
{
   path_t path{};
   auto actNode = node;
   for(int i=0; i<MAX_PATH_LENGTH; ++i)
   {
       auto parent = getInverseHierachicalReference(server, actNode);
       if(!parent)
       {
         break;
       }
       actNode=parent->nodeId.nodeId;
       path.emplace_back(std::move(*parent));
   }
   return path;
}
}
