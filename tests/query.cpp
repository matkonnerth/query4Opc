#include <cypher/QueryEngine.h>
#include <cypher/Parser.h>
#include <NodesetLoader/backendOpen62541.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

std::string g_path = "";

TEST(serverType, findServerObject)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   Parser p;
   auto q = p.parse("MATCH(obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId: \"i=2004\"}) RETURN obj");
   ASSERT_TRUE(q);

   QueryEngine e{server};
   e.scheduleQuery(*q);
   auto results = e.run();
   ASSERT_EQ(results.size(), 1);
   UA_Server_delete(server);
}

TEST(serverType, findServerObject_VariablePath_WrongVariable)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   Parser p;
   auto q = p.parse("MATCH(obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId: \"i=2004\"})--(:Variable{NodeId:\"123\"}) RETURN obj");
   ASSERT_TRUE(q);

   QueryEngine e{ server };
   e.scheduleQuery(*q);
   auto results = e.run();
   ASSERT_EQ(results.size(), 0);
   UA_Server_delete(server);
}

TEST(serverType, findServerObject_VariablePath_WrongDirection)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   Parser p;
   auto q = p.parse("MATCH(obj:Object)<-[:HasTypeDefinition]-(:ObjectType{NodeId: \"i=2004\"}) RETURN obj");
   ASSERT_TRUE(q);

   QueryEngine e{ server };
   e.scheduleQuery(*q);
   auto results = e.run();
   ASSERT_EQ(results.size(), 0);
   UA_Server_delete(server);
}

TEST(serverType, findServerObject_VariablePath_DirectionDoesntMatter)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   Parser p;
   auto q = p.parse("MATCH(obj:Object)-[:HasTypeDefinition]-(:ObjectType{NodeId: \"i=2004\"}) RETURN obj");
   ASSERT_TRUE(q);

   QueryEngine e{ server };
   e.scheduleQuery(*q);
   auto results = e.run();
   ASSERT_EQ(results.size(), 1);
   UA_Server_delete(server);
}

/*
TEST(serverType, findServerObject_reorderQuery)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   Parser p;
   auto q = p.parse("MATCH (:ObjectType{NodeId: \"i=2004\"})<-[:HasTypeDefinition]-(obj:Object) RETURN obj");
   ASSERT_TRUE(q);

   QueryEngine e{ server };
   e.scheduleQuery(*q);
   auto results = e.run();
   ASSERT_TRUE(results);
   ASSERT_EQ(results->size(), 1);
   UA_Server_delete(server);
}
*/

TEST(serverType, findServerObject_WrongReferenceType)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   Parser p;
   auto q = p.parse("MATCH(obj:Object)-[:HasProperty]->(:ObjectType{NodeId: \"i=2004\"}) RETURN obj");
   ASSERT_TRUE(q);

   QueryEngine e{ server };
   e.scheduleQuery(*q);
   auto results = e.run();
   ASSERT_EQ(results.size(), 0);
   UA_Server_delete(server);
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
   ASSERT_GT(results.size(), 0);
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
   ASSERT_GT(results.size(), 0);
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
   ASSERT_EQ(results.size(), 0);
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