#include <NodesetLoader/backendOpen62541.h>
#include <graph/Filter.h>
#include <graph/Source.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <graph/PathMatcher.h>

std::string g_path = "";

TEST(import, testImport)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/graph.xml").c_str(), NULL));

   UA_Server_delete(server);
}

TEST(objectWithProperty, findAllTempDevices)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));

   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   std::vector<PathElement> path{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODECLASS_OBJECTTYPE, UA_NODEID_NUMERIC(2, 1002), UA_BROWSEDIRECTION_FORWARD } };

   PathMatcher p{server, path};

   auto f = [&](Result&& res){p.match(res.target);};

   vis.generate(f);
   ASSERT_EQ(p.results()->size(), 2);

   UA_Server_delete(server);
}

/*
TEST(objectWithProperty, findAllTempDevicesWithProperty)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));

   TypeFilter<Result> instancesOfType{ UA_NODEID_NUMERIC(2, 1002) };
   Sink<Result> s;

   std::vector<PathElement> p{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_NODECLASS_VARIABLE, std::nullopt, UA_BROWSEDIRECTION_BOTH } };

   ReferenceFilter<Result> hasProperty{ server, p };

   instancesOfType.append(hasProperty).append(s);
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   vis.generate(instancesOfType);
   ASSERT_EQ(s.results().size(), 1);

   UA_Server_delete(server);
}
*/


/*
TEST(objectWithProperty, findAllTempDevicesWithPropertyAndCertainPropertyId)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));

   TypeFilter<Result> instancesOfType{ UA_NODEID_NUMERIC(2, 1002) };
   Sink<Result> s;

   instancesOfType.append(s);
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   vis.generate(instancesOfType);
   ASSERT_EQ(s.results().size(), 2);

   std::vector<PathElement> p{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_NODECLASS_VARIABLE, UA_NODEID_NUMERIC(2, 6001), UA_BROWSEDIRECTION_BOTH } };
   ReferenceFilter<Result> hasProperty{ server, p};

   Sink<Result>
   sinkWithProperty;
   hasProperty.append(sinkWithProperty);

   SinkToSource source2{ s };
   source2.generate(hasProperty);
   ASSERT_EQ(sinkWithProperty.results().size(), 1);

   UA_Server_delete(server);
}
*/

TEST(objectWithProperty, allObjects)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(2, 5002), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   //empty Path
   PathMatcher p{ server, std::vector<PathElement>{}};

   auto f = [&](Result&& res) { p.match(res.target); };

   vis.generate(f);
   ASSERT_EQ(p.results()->size(), 2);
   UA_Server_delete(server);
}


TEST(objectWithProperty, allVariables)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (g_path + "/objectwithproperty.xml").c_str(), NULL));
 
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(2, 5002), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_VARIABLE };

   // empty Path
   PathMatcher p{ server, std::vector<PathElement>{} };

   auto f = [&](Result&& res) { p.match(res.target); };

   vis.generate(f);
   ASSERT_EQ(p.results()->size(), 1);
   UA_Server_delete(server);
}


TEST(serverType, findServerObject)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   std::vector<PathElement> path{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODECLASS_OBJECTTYPE, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVERTYPE), UA_BROWSEDIRECTION_FORWARD } };
   PathMatcher p{ server, path };
   vis.generate([&](Result&& res) { p.match(res.target); });
   ASSERT_EQ(p.results()->size(), 1);

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