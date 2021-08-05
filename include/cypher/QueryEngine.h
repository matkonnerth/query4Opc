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
      auto s = PathToSimplePath(q.matchClauses[0].path);
      if (s)
      {
         m_filterChain = createFilterChain(*s, m_server);
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