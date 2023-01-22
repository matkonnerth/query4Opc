#include "testHelper.h"
#include <cypher/Parser.h>
#include <cypher/QueryEngine.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/nodesetloader.h>

std::string g_path = "";
using namespace cypher;

TEST(serverType, findServerObject)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    Parser p;
    auto q = p.parse("MATCH(obj:Object)-[:HasTypeDefinition]->(:ObjectType{"
                     "NodeId: \"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
    UA_Server_delete(server);
}

TEST(serverType, findServerObject_VariablePath_WrongVariable)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

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
    UA_Server_delete(server);
}

TEST(serverType, findServerObject_VariablePath_WrongDirection)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    Parser p;
    auto q = p.parse("MATCH(obj:Object)<-[:HasTypeDefinition]-(:ObjectType{"
                     "NodeId: \"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 0);
    UA_Server_delete(server);
}

TEST(serverType, findServerObject_VariablePath_DirectionDoesntMatter)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    Parser p;
    auto q = p.parse("MATCH(obj:Object)-[:HasTypeDefinition]-(:ObjectType{"
                     "NodeId: \"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 1);
    UA_Server_delete(server);
}

TEST(serverType, findServerObject_reorderQuery)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

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
    UA_Server_delete(server);
}

TEST(serverType, findServerObject_WrongReferenceType)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    Parser p;
    auto q = p.parse("MATCH(obj:Object)-[:HasProperty]->(:ObjectType{NodeId: "
                     "\"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 0);
    UA_Server_delete(server);
}

TEST(serverType, findServerObject_MultipleMatchClauses)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

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
    UA_Server_delete(server);
}

TEST(objectWithProperty, findAllTempDevicesWithProperty)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    ASSERT_EQ(UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL),
              UA_STATUSCODE_GOOD);

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
    cleanupServer(server);
}


TEST(serverType, findObjectsWhichReferencesVariables)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    Parser p;
    auto q = p.parse("MATCH(obj:Object)--(:Variable) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_GT(results->size(), 0);
    UA_Server_delete(server);
}

TEST(serverType, emptyPath)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    Parser p;
    auto q = p.parse("MATCH (obj:Object) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_GT(results->size(), 0);
    UA_Server_delete(server);
}

TEST(serverType, emptyPathObjectType)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    Parser p;
    auto q = p.parse("MATCH (obj:ObjectType) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 0);
    UA_Server_delete(server);
}

TEST(serverType, subfolder)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    ASSERT_EQ(UA_Server_loadNodeset(server, (g_path + "/subfolder.xml").c_str(), NULL),
              UA_STATUSCODE_GOOD);

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
    cleanupServer(server);
}


TEST(serverType, playground)
{
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    Parser p;
    auto q = p.parse("MATCH (obj:Object)-->(:Method)-->(:Variable) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(results->size(), 2);
    UA_Server_delete(server);
}


int main(int argc, char** argv)
{

    testing::InitGoogleTest(&argc, argv);

    if (!(argc > 1))
        return 1;
    g_path = argv[1];

    return RUN_ALL_TESTS();
}