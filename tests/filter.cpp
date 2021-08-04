#include "Filter.h"
#include "Sink.h"
#include "Source.h"
#include <NodesetLoader/backendOpen62541.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

std::string path = "";

TEST(import, testImport)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (path + "/graph.xml").c_str(), NULL));

   UA_Server_delete(server);
}

TEST(filter, basics)
{
   std::vector<UA_ReferenceDescription> refs;
   UA_ReferenceDescription ref;
   ref.typeDefinition = UA_EXPANDEDNODEID_NUMERIC(0, 58);
   ref.nodeId = UA_EXPANDEDNODEID_NUMERIC(1, 10000);
   for (auto i = 0; i < 100; ++i)
   {

      refs.emplace_back(ref);
   }

   Sink<Result> s{};
   UA_ReferenceDescription typeRef{};
   typeRef.nodeId.nodeId = UA_NODEID_NUMERIC(0, 58);
   TypeFilter<Result> typeFilter{ std::vector<Result>{ Result{ UA_NODEID_NUMERIC(0, 58), typeRef } } };
   TypeFilter<Result> typeFilter2{ std::vector<Result>{ Result{ UA_NODEID_NUMERIC(0, 58), typeRef } } };

   TakeAllFilter<Result> ta{};
   typeFilter.append(typeFilter2).append(ta).append(s);

   for (const auto& r : refs)
   {
      typeFilter.filter(Result{ UA_NODEID_NUMERIC(0, 85), r });
   }

   ASSERT_EQ(s.results().size(), 100);
}

TEST(objectWithProperty, findAllTempDevices)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (path + "/objectwithproperty.xml").c_str(), NULL));

   TypeFilter<Result> instancesOfType{ UA_NODEID_NUMERIC(2, 1002) };
   Sink<Result> s;
   instancesOfType.append(s);
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   vis.generate(instancesOfType);
   ASSERT_EQ(s.results().size(), 2);

   UA_Server_delete(server);
}

TEST(objectWithProperty, findAllTempDevicesWithProperty)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (path + "/objectwithproperty.xml").c_str(), NULL));

   TypeFilter<Result> instancesOfType{ UA_NODEID_NUMERIC(2, 1002) };
   Sink<Result> s;

   ReferenceFilter<Result> hasProperty{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY) };

   instancesOfType.append(hasProperty).append(s);
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   vis.generate(instancesOfType);
   ASSERT_EQ(s.results().size(), 1);

   UA_Server_delete(server);
}

TEST(objectWithProperty, findAllTempDevicesWithPropertyAndCertainPropertyId)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (path + "/objectwithproperty.xml").c_str(), NULL));

   TypeFilter<Result> instancesOfType{ UA_NODEID_NUMERIC(2, 1002) };
   Sink<Result> s;

   instancesOfType.append(s);
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   vis.generate(instancesOfType);
   ASSERT_EQ(s.results().size(), 2);

   ReferenceFilter<Result> hasProperty{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), std::make_unique<MatchTargetNodeId>(UA_NODEID_NUMERIC(2, 6001)) };

   Sink<Result> sinkWithProperty;
   hasProperty.append(sinkWithProperty);

   SinkToSource source2{ s };
   source2.generate(hasProperty);
   ASSERT_EQ(sinkWithProperty.results().size(), 1);

   UA_Server_delete(server);
}

TEST(objectWithProperty, allObjects)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (path + "/objectwithproperty.xml").c_str(), NULL));
   Sink<Result> s;

   TakeAllFilter<Result> ta{};

   ta.append(s);
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(2, 5002), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   vis.generate(ta);
   ASSERT_EQ(s.results().size(), 2);
   UA_Server_delete(server);
}

/* not working -- other nodeclasses have to be also traversed
TEST(objectWithProperty, allVariables)
{
   UA_Server* server = UA_Server_new();
   UA_ServerConfig_setDefault(UA_Server_getConfig(server));

   ASSERT_TRUE(NodesetLoader_loadFile(server, (path + "/objectwithproperty.xml").c_str(), NULL));
   Sink<Result> s;

   TakeAllFilter<Result> ta{};

   ta.append(s);
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(2, 5002), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_VARIABLE};

   vis.generate(ta);
   ASSERT_EQ(s.results().size(), 1);
   UA_Server_delete(server);
}
*/

int main(int argc, char** argv)
{

   testing::InitGoogleTest(&argc, argv);

   if (!(argc > 1))
      return 1;
   path = argv[1];

   return RUN_ALL_TESTS();
}