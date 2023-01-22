#include <graph/Filter.h>
#include <graph/PathMatcher.h>
#include <graph/Source.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/plugin/nodesetloader.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

std::string g_path = "";

class FilterTest : public ::testing::Test
{
 protected:
    UA_Server* server{ nullptr };

    void SetUp() override
    {
        server = UA_Server_new();
        UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    }

    void TearDown() override
    {
        UA_Server_delete(server);
    }
};

TEST_F(FilterTest, testImport)
{
   ASSERT_TRUE(UA_StatusCode_isGood(
   UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));
}

TEST_F(FilterTest, findAllTempDevices)
{
   ASSERT_EQ(UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL), UA_STATUSCODE_GOOD);

   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   std::vector<PathElement> path{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODECLASS_OBJECTTYPE, UA_NODEID_NUMERIC(2, 1002), UA_BROWSEDIRECTION_FORWARD} };

   PathMatcher p{server, path, 0};

   auto f = [&](path_element_t&& res){p.match(res);};

   vis.generate(f);
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 2);
}


TEST_F(FilterTest, findAllTempDevicesWithProperty)
{
   ASSERT_TRUE(UA_StatusCode_isGood(UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));

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
}

TEST_F(FilterTest, findAllTempDevicesWithPropertyAndCertainPropertyId)
{
   ASSERT_TRUE(UA_StatusCode_isGood(UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));

   std::vector<PathElement> path{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_NODECLASS_VARIABLE, UA_NODEID_NUMERIC(2, 6001), UA_BROWSEDIRECTION_BOTH },
                                  PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODECLASS_OBJECTTYPE, UA_NODEID_NUMERIC(2, 1002), UA_BROWSEDIRECTION_FORWARD } };

   PathMatcher p{ server, path, 1 };

   auto f = [&](path_element_t&& res) { p.match(res); };
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };
   vis.generate(f);
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 1);
}


TEST_F(FilterTest, allObjects)
{
   ASSERT_TRUE(UA_StatusCode_isGood(
   UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(2, 5002), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   //empty Path
   PathMatcher p{ server, std::vector<PathElement>{}};

   auto f = [&](path_element_t&& res) { p.match(res); };

   vis.generate(f);
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 2);
}


TEST_F(FilterTest, allVariables)
{
   ASSERT_TRUE(UA_StatusCode_isGood(
   UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));

   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(2, 5002), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_VARIABLE };

   // empty Path
   PathMatcher p{ server, std::vector<PathElement>{} };

   auto f = [&](path_element_t&& res) { p.match(res); };

   vis.generate(f);
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 1);
}


TEST_F(FilterTest, findServerObject)
{
   HierachicalVisitor vis{ server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), UA_NODECLASS_OBJECT };

   std::vector<PathElement> path{ PathElement{ UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION), UA_NODECLASS_OBJECTTYPE, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVERTYPE), UA_BROWSEDIRECTION_FORWARD } };
   PathMatcher p{ server, path };
   vis.generate([&](path_element_t&& res) { p.match(res); });
   ASSERT_TRUE(p.results().col());
   ASSERT_EQ(p.results().col()->size(), 1);
}

int main(int argc, char** argv)
{

   testing::InitGoogleTest(&argc, argv);

   if (!(argc > 1))
      return 1;
   g_path = argv[1];

   return RUN_ALL_TESTS();
}