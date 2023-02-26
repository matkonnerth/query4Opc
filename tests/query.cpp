#include <cypher/Parser.h>
#include <graph/QueryEngine.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/nodesetloader.h>

std::string g_path = "";
using namespace cypher;
using namespace graph;

class QueryTest : public ::testing::Test
{
 protected:
    UA_Server* server{nullptr};
    
    void SetUp() override
    {
        server = UA_Server_new();
        UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    }

    void TearDown() override {
        UA_Server_delete(server);
    }
};

TEST_F(QueryTest, findServerObject)
{
    Parser p;
    auto q = p.parse("MATCH(obj:Object)-[:HasTypeDefinition]->(:ObjectType{"
                     "NodeId: \"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
}

TEST_F(QueryTest, findServerObject_VariablePath_WrongVariable)
{
    Parser p;
    auto q =
    p.parse("MATCH (obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId: "
            "\"i=2004\"})--(:Variable{NodeId:\"123\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 0);
}

TEST_F(QueryTest, findServerObject_VariablePath_WrongDirection)
{
    Parser p;
    auto q = p.parse("MATCH(obj:Object)<-[:HasTypeDefinition]-(:ObjectType{"
                     "NodeId: \"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 0);
}

TEST_F(QueryTest, findServerObject_VariablePath_DirectionDoesntMatter)
{
    Parser p;
    auto q = p.parse("MATCH(obj:Object)-[:HasTypeDefinition]-(:ObjectType{"
                     "NodeId: \"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
}

TEST_F(QueryTest, findServerObject_reorderQuery)
{
    Parser p;
    auto q =
    p.parse("MATCH (:ObjectType{NodeId: "
            "\"i=2004\"})<-[:HasTypeDefinition]-(obj:Object) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
}

TEST_F(QueryTest, findServerObject_WrongReferenceType)
{
    Parser p;
    auto q = p.parse("MATCH(obj:Object)-[:HasProperty]->(:ObjectType{NodeId: "
                     "\"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 0);
}

TEST_F(QueryTest, findServerObject_MultipleMatchClauses)
{
    Parser p;
    auto q = p.parse(R"(
      MATCH (obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId: "i=2004"})
      MATCH (obj) RETURN obj
    )");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
}

TEST_F(QueryTest, findAllTempDevicesWithProperty)
{
    ASSERT_TRUE(UA_StatusCode_isGood(UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));

    Parser p;
    auto q = p.parse(R"(
      MATCH (obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId: "ns=2;i=1002"})
      MATCH (obj)-[:HasProperty]->(:Variable) RETURN obj
    )");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
}

TEST_F(QueryTest, findAllTempDevicesWithPropertyOneQuery)
{
    ASSERT_TRUE(UA_StatusCode_isGood(
    UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));

    Parser p;
    auto q = p.parse(R"(
      MATCH (:Variable)<-[:HasProperty]-(obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId: "ns=2;i=1002"})
      RETURN obj
    )");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
}


TEST_F(QueryTest, findObjectsWhichReferencesVariables)
{
    Parser p;
    auto q = p.parse("MATCH(obj:Object)--(:Variable) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_GT(results->size(), 0);
}

TEST_F(QueryTest, emptyPath)
{
    Parser p;
    auto q = p.parse("MATCH (obj:Object) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_GT(results->size(), 0);
}

TEST_F(QueryTest, TypeDefinitionId)
{
    Parser p;
    auto q = p.parse("MATCH (obj:Object{TypeDefinitionId:\"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
}

TEST_F(QueryTest, emptyPathObjectType)
{
    Parser p;
    auto q = p.parse("MATCH (obj:ObjectType) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 0);
}

TEST_F(QueryTest, subfolder)
{
    ASSERT_TRUE(UA_StatusCode_isGood(UA_Server_loadNodeset(server, (g_path + "/subfolder.xml").c_str(), NULL)));

    // match all BaseObjects below a folder
    Parser p;
    auto q = p.parse("MATCH "
                     "(:ObjectType{NodeId:\"i=61\"})<--(obj:Object)-->(:Object)"
                     "-->(:ObjectType{NodeId:\"i=58\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 3);
}

TEST_F(QueryTest, eightObjects)
{
    ASSERT_TRUE(UA_StatusCode_isGood(
    UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));
    Parser p;
    auto q = p.parse("MATCH "
                     "(obj:Object)-->(:Object)-->(:Object)-->(:Object)-->(:"
                     "Object)-->(:Object)-->(:Object)-->(:Object) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
}

TEST_F(QueryTest, playground)
{
    GTEST_SKIP();
    Parser p;
    auto q = p.parse("MATCH (obj:Object)-->(:Method)-->(:Variable) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 2);
}

TEST_F(QueryTest, serverObject)
{
    // match all BaseObjects below a folder
    Parser p;
    auto q = p.parse("Match(obj: Object{NodeId:\"i=2253\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
}

int main(int argc, char** argv)
{

    testing::InitGoogleTest(&argc, argv);

    if (!(argc > 1))
        return 1;
    g_path = argv[1];

    return RUN_ALL_TESTS();
}