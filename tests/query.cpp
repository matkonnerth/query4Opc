#include <cypher/QueryEngine.h>
#include <cypher/Parser.h>
#include <NodesetLoader/backendOpen62541.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

std::string path = "";


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
   ASSERT_TRUE(results);
   ASSERT_EQ(results->size(), 1);
   UA_Server_delete(server);
}



int main(int argc, char** argv)
{

   testing::InitGoogleTest(&argc, argv);

   if (!(argc > 1))
      return 1;
   path = argv[1];

   return RUN_ALL_TESTS();
}