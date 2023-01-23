#include <graph/Path.h>
#include <gtest/gtest.h>

std::string g_path = "";

TEST(serverType, findServerObjectWithPath)
{
   cypher::Path p;
   p.nodes.emplace_back(Node{"obj", "Object"});
   Node b;
   b.properties.emplace(std::make_pair("NodeId", "i=2004"));
   p.nodes.emplace_back(b);
   p.relations.emplace_back(Relationship{"i=40", 1});

   Path path(p);
  
}