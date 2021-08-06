#include <graph/FilterChain.h>
#include <NodesetLoader/backendOpen62541.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

std::string g_path = "";

/*
TEST(serverType, findServerObject)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   FilterChain f{ server };

   f.createHierachicalVisitorSource(UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT);
   f.createReferenceFilter(UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODEID_NUMERIC(0, UA_NS0ID_SERVERTYPE), UA_NODECLASS_UNSPECIFIED);
   f.createSink();
   f.run();

   ASSERT_EQ(f.getSink().results().size(), 1);

   UA_Server_delete(server);
}
*/

TEST(serverType, findServerObjectWithPath)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   Path p;
   p.nodes.emplace_back(Node{.label="Object"});
   Node b;
   b.properties.emplace(std::make_pair("NodeId", "i=2004"));
   p.nodes.emplace_back(b);
   p.relations.emplace_back(Relationship{.type="i=40"});

   auto f = createFilterChain(p, server);
   f->run();

   ASSERT_EQ(f->getSink().results().size(), 1);

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