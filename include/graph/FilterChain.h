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

   void createReferenceFilter(const cypher::Path& path, size_t startIndex)
   {
      std::vector<PathElement> p;
      cypher::PathIterator it{ path };
      size_t idx = 0;
      while (auto node = it.currentNode())
      {
         if (!it.nextRel())
         {
            break;
         }
         PathElement e{};
         if (idx < startIndex)
         {
            e.referenceType = lookupReferenceType(it.nextRel()->type);
            e.nodeClass = parseOptionalNodeClass(node->label);
            if (node->NodeId())
            {
               e.targetId = parseOptionalNodeId(node->NodeId());
            }
            if (it.nextRel()->direction == 1)
            {
               e.direction = UA_BROWSEDIRECTION_INVERSE;
            }
            else if (it.nextRel()->direction == -1)
            {
               e.direction = UA_BROWSEDIRECTION_FORWARD;
            }
            else
            {
               e.direction = UA_BROWSEDIRECTION_BOTH;
            }
         }
         else
         {
            e.referenceType = lookupReferenceType(it.nextRel()->type);
            e.nodeClass = parseOptionalNodeClass(it.nextNode()->label);
            if (it.nextNode()->NodeId())
            {
               e.targetId = parseOptionalNodeId(it.nextNode()->NodeId());
            }
            if (it.nextRel()->direction == 1)
            {
               e.direction = UA_BROWSEDIRECTION_FORWARD;
            }
            else if (it.nextRel()->direction == -1)
            {
               e.direction = UA_BROWSEDIRECTION_INVERSE;
            }
            else
            {
               e.direction = UA_BROWSEDIRECTION_BOTH;
            }
         }

         p.emplace_back(e);
         it++;
         idx++;
      }
      m_pathMatcher = std::make_unique<PathMatcher>(m_server, p, startIndex);
      m_path = path;
   }

   const column_t* results() const
   {
      return m_pathMatcher->results().col();
   }

   const column_t* results(const std::string& identifier)
   {
      size_t idx = 0;
      for(const auto&n:m_path.nodes)
      {
         if(n.identifier && n.identifier==identifier)
         {
            return m_pathMatcher->results().col(idx);
         }
         idx++;
      }
      return nullptr;
   }

private:
   UA_Server* m_server;
   std::unique_ptr<Source> m_src;
   std::unique_ptr<PathMatcher> m_pathMatcher;
   cypher::Path m_path;
};

size_t findStartIndex(const cypher::Path &p)
{
   auto idx = 0u;
   for(const auto& e:p.nodes)
   {
      if(e.identifier)
      {
         return idx;
      }
      idx+=1;
   }
   return 0u;
}

std::unique_ptr<FilterChain> createFilterChain(const cypher::Path& path, UA_Server* server)
{
   auto start = findStartIndex(path);

   auto f = std::make_unique<FilterChain>(server);
   f->createHierachicalVisitorSource(UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), parseOptionalNodeClass(path.nodes[start].label));
   f->createReferenceFilter(path, start);
   return f;
}