#include <cypher/Path.h>

std::optional<SimplePath> PathToSimplePath(const Path& p)
{
   if (p.nodes.size() != 2 || p.relations.size() != 1)
   {
      std::cout << "conversion from path to simplePath not possible"
                << "\n";
      return std::nullopt;
   }
   SimplePath s;
   s.m_nodeA = p.nodes[0];
   s.m_nodeB = p.nodes[1];
   s.m_rel = p.relations[0];
   return s;
}