#include <NodesetLoader/backendOpen62541.h>
#include <graph/Filter.h>
#include <graph/Source.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <graph/PathMatcher.h>
#include "testHelper.h"

std::string g_path = "";

TEST(import, testImport)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/graph.xml").c_str(), NULL));

   cleanupServer(server);
}

TEST(objectWithProperty, findAllTempDevices)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));

   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   std::vector<PathElement> path{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODECLASS_OBJECTTYPE, UA_NODEID_NUMERIC(2, 1002), UA_BROWSEDIRECTION_FORWARD } };

   PathMatcher p{server, path, 0};

   auto f = [&](path_element_t&& res){p.match(res);};

   vis.generate(f);
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 2);

   cleanupServer(server);
}


TEST(objectWithProperty, findAllTempDevicesWithProperty)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));

   std::vector<PathElement> path{ 
      PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_NODECLASS_VARIABLE, std::nullopt, UA_BROWSEDIRECTION_BOTH },
      PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODECLASS_OBJECTTYPE, UA_NODEID_NUMERIC(2,1002), UA_BROWSEDIRECTION_FORWARD}
       };

   PathMatcher p{ server, path, 1 };

   auto f = [&](path_element_t&& res) { p.match(res); };
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };
   vis.generate(f);
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 1);

   cleanupServer(server);
}

TEST(objectWithProperty, findAllTempDevicesWithPropertyAndCertainPropertyId)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));

   std::vector<PathElement> path{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_NODECLASS_VARIABLE, UA_NODEID_NUMERIC(2, 6001), UA_BROWSEDIRECTION_BOTH },
                                  PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODECLASS_OBJECTTYPE, UA_NODEID_NUMERIC(2, 1002), UA_BROWSEDIRECTION_FORWARD } };

   PathMatcher p{ server, path, 1 };

   auto f = [&](path_element_t&& res) { p.match(res); };
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };
   vis.generate(f);
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 1);

   cleanupServer(server);
}


TEST(objectWithProperty, allObjects)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(2, 5002), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   //empty Path
   PathMatcher p{ server, std::vector<PathElement>{}};

   auto f = [&](path_element_t&& res) { p.match(res); };

   vis.generate(f);
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 2);
   cleanupServer(server);
}


TEST(objectWithProperty, allVariables)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));
 
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(2, 5002), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_VARIABLE };

   // empty Path
   PathMatcher p{ server, std::vector<PathElement>{} };

   auto f = [&](path_element_t&& res) { p.match(res); };

   vis.generate(f);
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 1);
   cleanupServer(server);
}


TEST(serverType, findServerObject)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   std::vector<PathElement> path{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODECLASS_OBJECTTYPE, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVERTYPE), UA_BROWSEDIRECTION_FORWARD } };
   PathMatcher p{ server, path };
   vis.generate([&](path_element_t&& res) { p.match(res); });
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 1);

   cleanupServer(server);
}

int main(int argc, char** argv)
{

   testing::InitGoogleTest(&argc, argv);

   if (!(argc > 1))
      return 1;
   g_path = argv[1];

   return RUN_ALL_TESTS();
}