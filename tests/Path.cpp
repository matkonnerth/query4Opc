#include <graph/Path.h>
#include <gtest/gtest.h>

std::string g_path = "";

TEST(PahtTest, buildPath)
{
   cypher::Path p;
   p.nodes.emplace_back(cypher::Node{"obj", "Object"});
   cypher::Node b;
   b.properties.emplace(std::make_pair("NodeId", "i=2004"));
   p.nodes.emplace_back(b);
   p.relations.emplace_back(cypher::Relationship{"i=40", 1});

   graph::Path path(p);
}