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
   //TODO: reordering of path, where is the identifier to return, let's assume there is only one identifer


  
   SplittedPaths splitted;
   splitted.path = p;
   return splitted;
}