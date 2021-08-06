#pragma once
#include "Filter.h"
#include "Helper.h"
#include "Sink.h"
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
      m_src->generate(*m_filters.front());
   }

   void createHierachicalVisitorSource(const UA_NodeId& root, const UA_NodeId& referenceTypeId, UA_UInt32 nodeclasMask)
   {
      m_src = std::make_unique<HierachicalVisitor>(m_server, root, referenceTypeId, nodeclasMask);
   }

   void createSinkToSource(const Sink<Result>& sink)
   {
      m_src = std::make_unique<SinkToSource>(sink);
   }

   void createReferenceFilter(const std::vector<PathElement>& path)
   {
      auto filter = std::make_unique<ReferenceFilter<Result>>(m_server, path);
      m_filters.emplace_back(std::move(filter));

      if (m_filters.size() > 1u)
      {
         m_filters[m_filters.size() - 2u]->append(*m_filters[m_filters.size() - 1u]);
      }
   }


   void createSink()
   {
      m_sink = std::make_unique<Sink<Result>>();
      if (m_filters.empty())
      {
         m_filters.emplace_back(std::make_unique<TakeAllFilter<Result>>());
      }
      m_filters.back()->append(*m_sink);
   }

   const Sink<Result>& getSink() const
   {
      return *m_sink;
   }

private:
   UA_Server* m_server;
   std::unique_ptr<Source> m_src;
   std::unique_ptr<Sink<Result>> m_sink;
   std::vector<std::unique_ptr<AbstractFilter<Result>>> m_filters;
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
   f->createSink();
   return f;
}

std::unique_ptr<FilterChain> createFilterChain(const EmptyPath& path, UA_Server* server)
{
   auto f = std::make_unique<FilterChain>(server);
   f->createHierachicalVisitorSource(UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), parseOptionalNodeClass(path.m_node.label));
   f->createSink();
   return f;
}