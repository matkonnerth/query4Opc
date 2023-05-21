#include <cypher/Parser.h>
#include <graph/QueryEngine.h>
#include <gtest/gtest.h>
#include <iostream>
#include <open62541/plugin/nodesetloader.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

std::string g_path = "";
using namespace cypher;
using namespace graph;

class QueryTest : public ::testing::Test
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
    ASSERT_TRUE(UA_StatusCode_isGood(
    UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));

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
    ASSERT_EQ(1, results->size());
    ASSERT_EQ(1, e.pathResult().paths().size());
    ASSERT_EQ(2, e.pathResult().paths()[0].size());
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
    ASSERT_EQ(1, results->size());
    ASSERT_EQ(1, e.pathResult().paths().size());
    ASSERT_EQ(3, e.pathResult().paths()[0].size());
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
    ASSERT_TRUE(results->size() > 0);
    ASSERT_TRUE(e.pathResult().paths().size() > 0);
    ASSERT_EQ(2, e.pathResult().paths()[0].size());
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
    ASSERT_TRUE(results->size() > 0);
    ASSERT_TRUE(e.pathResult().paths().size() > 0);
    ASSERT_EQ(1, e.pathResult().paths()[0].size());
}

TEST_F(QueryTest, TypeDefinitionId)
{
    Parser p;
    auto q =
    p.parse("MATCH (obj:Object{TypeDefinitionId:\"i=2004\"}) RETURN obj");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(1, results->size());
}

TEST_F(QueryTest, ObjectTypes)
{
    Parser p;
    auto q = p.parse("MATCH (types:ObjectType) RETURN types");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_TRUE(results->size()>0);
}

TEST_F(QueryTest, subfolder)
{
    ASSERT_TRUE(UA_StatusCode_isGood(
    UA_Server_loadNodeset(server, (g_path + "/subfolder.xml").c_str(), NULL)));

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
    ASSERT_EQ(3, results->size());
    ASSERT_EQ(3, e.pathResult().paths().size());
    ASSERT_EQ(4, e.pathResult().paths()[0].size());
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
    ASSERT_EQ(results->size(), 2);
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
    ASSERT_EQ(2, results->size());
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
    ASSERT_EQ(e.pathResult().paths().size(), 1);
    ASSERT_EQ(e.pathResult().paths()[0].size(), 1);
}

TEST_F(QueryTest, path_to_ServerObject)
{
    auto path = graph::getPathToParentNode(server, UA_NODEID_NUMERIC(0, 2253));

    ASSERT_EQ(2, path.size());
    ASSERT_EQ(85u, path[0].nodeId.nodeId.identifier.numeric);
    ASSERT_EQ(84u, path[1].nodeId.nodeId.identifier.numeric);
}

TEST_F(QueryTest, types)
{
    ASSERT_TRUE(UA_StatusCode_isGood(
    UA_Server_loadNodeset(server, (g_path + "/objectwithproperty.xml").c_str(), NULL)));

    //this is working because the open62541 server also adds the inverse reference from objectType to object
    Parser p;
    auto q = p.parse(R"(
        MATCH (:Object)-->(types:ObjectType)
        MATCH (types)<--(obj:Object{NodeId:"i=2253"}) RETURN obj
        )");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_TRUE(results);
    ASSERT_EQ(1, results->size());
    ASSERT_EQ(1, e.pathResult().paths().size());
    ASSERT_EQ(2004, e.pathResult().paths()[0].at(0).nodeId.nodeId.identifier.numeric);
}

TEST_F(QueryTest, allSubTypes_TempDevice_and_SpecialTempDevice)
{
    ASSERT_TRUE(UA_StatusCode_isGood(
    UA_Server_loadNodeset(server, (g_path + "/subtype.xml").c_str(), NULL)));
    Parser p;
    auto q = p.parse(R"(MATCH (types: ObjectType{NodeId:"ns=2;i=1002", includeSubTypes: "true"}) RETURN types)");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_EQ(2, e.pathResult().paths().size());
    ASSERT_EQ(1, e.pathResult().paths()[0].size());
}

TEST_F(QueryTest, allSubTypes_ServerTypeHasNoSubType_onlyServerType)
{
    Parser p;
    auto q = p.parse(R"(MATCH (types: ObjectType{NodeId:"i=2253", includeSubTypes: "true"}) RETURN types)");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_EQ(1, e.pathResult().paths().size());
    ASSERT_EQ(1, e.pathResult().paths()[0].size());
    ASSERT_EQ(2253, e.pathResult().paths()[0][0].nodeId.nodeId.identifier.numeric);
}

TEST_F(QueryTest, includeSubTypes_specialTempDevice)
{
    ASSERT_TRUE(UA_StatusCode_isGood(
    UA_Server_loadNodeset(server, (g_path + "/subtype.xml").c_str(), NULL)));
    Parser p;
    auto q = p.parse(R"(
        MATCH (types: ObjectType{NodeId:"ns=2;i=1002", includeSubTypes: "true"}) RETURN types
        MATCH (obj: Object)-->(types) RETURN obj
        )");
    ASSERT_TRUE(q);

    QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
    ASSERT_EQ(2, e.pathResult(0).paths().size());
    ASSERT_EQ(1, e.pathResult().paths().size());
    ASSERT_EQ(5003, e.pathResult().paths()[0][0].nodeId.nodeId.identifier.numeric);
}


int main(int argc, char** argv)
{

    testing::InitGoogleTest(&argc, argv);

    if (!(argc > 1))
        return 1;
    g_path = argv[1];

    return RUN_ALL_TESTS();
}