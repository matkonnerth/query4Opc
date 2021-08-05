#include <cypher/Parser.h>
#include <gtest/gtest.h>


std::string path = "";


TEST(Parser, quick)
{
   Parser p;
   auto q = p.parse("MATCH (n:Object) RETURN n");
   ASSERT_TRUE(q);
   q = p.parse("Match (n:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:\"123\"}) RETURN n");
   ASSERT_TRUE(q);
   q = p.parse("abc");
   ASSERT_FALSE(q);
}

TEST(Parser, Relationship)
{
   Parser p;
   auto q = p.parse("Match (n:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:\"123\"}) RETURN n");
   ASSERT_TRUE(q);
   ASSERT_EQ(q->matchClauses.size(), 1);
   ASSERT_EQ(q->matchClauses[0].path.nodes.size(), 2);
   ASSERT_EQ(*q->matchClauses[0].path.nodes[0].identifier, "n");
   ASSERT_EQ(*q->matchClauses[0].path.nodes[0].label, "Object");
   ASSERT_EQ(q->matchClauses[0].path.relations.size(), 1);
   ASSERT_EQ(*q->matchClauses[0].path.relations[0].type, "HasTypeDefinition");
}

int main(int argc, char** argv)
{

   testing::InitGoogleTest(&argc, argv);

   if (!(argc > 1))
      return 1;
   path = argv[1];

   return RUN_ALL_TESTS();
}