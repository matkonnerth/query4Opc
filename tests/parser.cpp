#include <cypher/Parser.h>
#include <gtest/gtest.h>

using namespace cypher;

TEST(Parser, quick)
{
   Parser p;
   auto q = p.parse("MATCH (n:Object) RETURN n");
   ASSERT_TRUE(q);
   q = p.parse("Match (n:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:\"123\"}) RETURN n");
   ASSERT_TRUE(q);
   q = p.parse("abc");
   ASSERT_FALSE(q);
   q = p.parse(R"(
      MATCH (obj:Object)-[:HasProperty]->(:ObjectType{NodeId: "i=2004"})
      MATCH (obj) RETURN obj
    )");
   ASSERT_TRUE(q);
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

TEST(Parser, noRefType)
{
    Parser p;
    auto q = p.parse(
    "Match (n:Object)-->(:Object) RETURN n");
    ASSERT_TRUE(q);
    //forward ref
    ASSERT_EQ(q->matchClauses[0].path.relations[0].direction, 1);
}

TEST(Parser, singleNode)
{
    Parser p;
    auto q = p.parse("Match (n:Object) RETURN n");
    ASSERT_TRUE(q);
}