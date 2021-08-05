#include <cypher/Path.h>

std::optional<SplittedPaths> splitPaths(const Path& p)
{
   if(p.nodes.size()==0 || (p.nodes.size()-1!=p.relations.size()))
   {
      return std::nullopt;
   }
   if(p.nodes.size()==1)
   {
      SplittedPaths s;
      EmptyPath e;
      e.m_node = p.nodes[0];
      s.emptyPath = e;
      return s;
   }
   //where is the identifier to return, let's assume there is only one identifer

   SimplePath s;
   PathIterator it{p};

   while(auto n = it.currentNode())
   {
      if(n->identifier)
      {
         s.m_nodeA = *n;
         if(it.nextNode())
         {
            s.m_nodeB = *it.nextNode();
            s.m_rel = *it.nextRel();
         }
         else
         {
            s.m_nodeB = *it.prevNode();
            s.m_rel = *it.prevRel();
         }
      }
      it++;
   }
  
   SplittedPaths splitted;
   splitted.simplePath = s;
   return splitted;
}