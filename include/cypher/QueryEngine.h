#pragma once
#include "Ast.h"
#include "Path.h"
#include <graph/FilterChain.h>
#include <memory>

class QueryEngine
{
public:
   QueryEngine(UA_Server* server)
   : m_server{ server }
   {}
   void scheduleQuery(const Query& q)
   {
      auto s = splitPaths(q.matchClauses[0].path);
      if (s)
      {
         if (s->emptyPath)
         {
            m_filterChain = createFilterChain(*s->emptyPath, m_server);
         }
         if(s->path)
         {
            m_filterChain = createFilterChain(*s->path, m_server);
            return;
         }
      }
   }

   const std::vector<Result>* run()
   {
      if (m_filterChain)
      {
         m_filterChain->run();
         return &m_filterChain->getSink().results();
      }
      return nullptr;
   }

private:
   UA_Server* m_server;
   std::unique_ptr<FilterChain> m_filterChain{ nullptr };
};