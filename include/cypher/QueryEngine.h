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
         m_filterChain = createFilterChain(*s->path, m_server);
         return;
      }
   }

   const std::vector<UA_ReferenceDescription>* run()
   {
      m_filterChain->run();
      return m_filterChain->results();
   }

private:
   UA_Server* m_server;
   std::unique_ptr<FilterChain> m_filterChain{ nullptr };
};