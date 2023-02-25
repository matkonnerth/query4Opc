#include <benchmark/benchmark.h>
#include <cypher/Parser.h>
#include <graph/QueryEngine.h>
#include <iostream>
#include <open62541/plugin/nodesetloader.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

static auto serverId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER);

static bool found{false};

std::string g_path{};

static void findServerObject(UA_Server* server, const UA_NodeId& startNode)
{
    UA_BrowseDescription bd;
    UA_BrowseDescription_init(&bd);
    bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    bd.includeSubtypes = true;
    bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES);
    // TODO: perfomance?
    bd.resultMask = UA_BROWSERESULTMASK_ALL;
    // bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION;
    bd.nodeId = startNode;
    bd.nodeClassMask = UA_NODECLASS_OBJECT;
    UA_BrowseResult br = UA_Server_browse(server, 1000, &bd);
    if (br.statusCode == UA_STATUSCODE_GOOD)
    {
        for (UA_ReferenceDescription* rd = br.references;
             rd != br.references + br.referencesSize;
             rd++)
        {
            if(UA_NodeId_equal(&rd->nodeId.nodeId, &serverId))
            {
                found = true;
            }
            findServerObject(server, rd->nodeId.nodeId);
        }
    }
    UA_BrowseResult_clear(&br);
}

static void addObject(UA_Server* server, int depth, int rowIndex)
{
    UA_NodeId parentId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    if(depth>0)
    {
        parentId = UA_NODEID_NUMERIC(1, rowIndex * 100 + depth -1);
    }
    UA_ObjectAttributes attr = UA_ObjectAttributes_default;
    UA_Server_addObjectNode(server, UA_NODEID_NUMERIC(1, rowIndex*100+depth), parentId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
    UA_QUALIFIEDNAME_ALLOC(1, ("Object_"+std::to_string(rowIndex*100)+"_"+std::to_string(depth)).c_str()), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), attr, nullptr, nullptr);
}

void addObjectsToAddressSpace(UA_Server* server, int maxDepth, int maxRows)
{
    for(int row=1; row<=maxRows; row++)
    {
        for(int depth=0; depth<maxDepth; depth++)
        {
            addObject(server, depth, row);
        }
    }
}

static void standardBrowse(benchmark::State& state)
{
    auto server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    addObjectsToAddressSpace(server, 100, 1000);



    for (auto _ : state)
    {
        findServerObject(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
    }

    UA_Server_delete(server);

    std::cout << "found: " << found;
}

void queryServerObjectImpl(UA_Server* server)
{
    cypher::Parser p;
    auto q = p.parse("MATCH(obj:Object)-[:HasTypeDefinition]->(:ObjectType{"
                        "NodeId: \"i=2004\"}) RETURN obj");

    graph::QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
}

void queryServerObjectImplInvertPath(UA_Server* server)
{
    cypher::Parser p;
    auto q =
    p.parse("MATCH(:ObjectType{""NodeId: \"i=2004\"})<-[:HasTypeDefinition]-(obj:Object) RETURN obj");

    graph::QueryEngine e{ server };
    e.scheduleQuery(*q);
    auto results = e.run();
}

static void queryServerObject(benchmark::State& state)
{
    auto server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    addObjectsToAddressSpace(server, 100, 1000);

    for (auto _ : state)
    {
        queryServerObjectImpl(server);
    }

    UA_Server_delete(server);

    std::cout << "found: " << found;
}

static void queryServerObjectInvertPath(benchmark::State& state)
{
    auto server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    addObjectsToAddressSpace(server, 100, 1000);

    for (auto _ : state)
    {
        queryServerObjectImplInvertPath(server);
    }

    UA_Server_delete(server);

    std::cout << "found: " << found;
}

BENCHMARK(standardBrowse);
BENCHMARK(queryServerObject);
BENCHMARK(queryServerObjectInvertPath);

int main(int argc, char** argv)
{
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}