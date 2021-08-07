#pragma once
#include "PathMatcher.h"
#include "Helper.h"
#include "Source.h"
#include <cypher/Path.h>
#include <memory>
#include <optional>
#include <vector>

class FilterChain
{
public:
   FilterChain(UA_Server* server)
   : m_server{ server }
   {}
   void run()
   {
      auto f = [&](Result&& res)
      {
         m_pathMatcher->match(res.target);
      };
      m_src->generate(f);
   }

   void createHierachicalVisitorSource(const UA_NodeId& root, const UA_NodeId& referenceTypeId, UA_UInt32 nodeclasMask)
   {
      m_src = std::make_unique<HierachicalVisitor>(m_server, root, referenceTypeId, nodeclasMask);
   }

   void createReferenceFilter(const std::vector<PathElement>& path)
   {
      m_pathMatcher = std::make_unique<PathMatcher>(m_server, path);
   }

   std::vector<UA_ReferenceDescription> results() const
   {
      return m_pathMatcher->results();
   }

private:
   UA_Server* m_server;
   std::unique_ptr<Source> m_src;
   std::unique_ptr<PathMatcher> m_pathMatcher;
};

/* Source starts always at node 0 */
std::unique_ptr<FilterChain> createFilterChain(const Path& path, UA_Server* server)
{
   auto f = std::make_unique<FilterChain>(server);
   //we start always at the first node
   f->createHierachicalVisitorSource(UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), parseOptionalNodeClass(path.nodes[0].label));

   std::vector<PathElement> p;
   PathIterator it{path};
   while(auto rel= it.nextRel())
   {
      PathElement e{};
      e.referenceType = lookupReferenceType(it.nextRel()->type);
      e.nodeClass = parseOptionalNodeClass(it.nextNode()->label);
      if (it.nextNode()->NodeId())
      {
         e.targetId = parseOptionalNodeId(it.nextNode()->NodeId());
      }
      if(it.nextRel()->direction==1)
      {
         e.direction = UA_BROWSEDIRECTION_FORWARD;
      }
      else if(it.nextRel()->direction==-1)
      {
         e.direction = UA_BROWSEDIRECTION_INVERSE;
      }
      else
      {
         e.direction = UA_BROWSEDIRECTION_BOTH;
      }
      p.emplace_back(e);
      it++;
   }
  
   f->createReferenceFilter(p);
   return f;
}