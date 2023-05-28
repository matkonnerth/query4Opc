#include <cypher/Parser.h>
#include <graph/QueryEngine.h>
#include <graph/json/json.h>
#include <open62541/plugin/nodesetloader.h>

static UA_StatusCode queryCallback(UA_Server* server,
                                              const UA_NodeId* sessionId,
                                              void* sessionHandle,
                                              const UA_NodeId* methodId,
                                              void* methodContext,
                                              const UA_NodeId* objectId,
                                              void* objectContext,
                                              size_t inputSize,
                                              const UA_Variant* input,
                                              size_t outputSize,
                                              UA_Variant* output)
{
    UA_String* inputStr = (UA_String*)input->data;

    std::string queryString{};
    queryString.assign((char*)inputStr->data, inputStr->length);

    cypher::Parser p;
    auto q = p.parse(queryString);
    if(!q)
    {
        auto result = UA_STRING("parsing the query failed");
        UA_Variant_setScalarCopy(output, &result, &UA_TYPES[UA_TYPES_STRING]);
        return UA_STATUSCODE_GOOD;
    }

    graph::QueryEngine e{ server };
    e.scheduleQuery(*q);
    e.run();

    auto result = graph::json_encode(e.pathResult());
    UA_String uaResult{};
    uaResult.data = (UA_Byte*)result.data();
    uaResult.length = result.size();

    UA_Variant_setScalarCopy(output, &uaResult, &UA_TYPES[UA_TYPES_STRING]);
    return UA_STATUSCODE_GOOD;
}

static void addQueryMethod(UA_Server* server)
{
    UA_Argument inputArgument;
    UA_Argument_init(&inputArgument);
    inputArgument.description = UA_LOCALIZEDTEXT("en-US", "Cypher query string");
    inputArgument.name = UA_STRING("CypherQueryString");
    inputArgument.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArgument.valueRank = UA_VALUERANK_SCALAR;

    UA_Argument outputArgument;
    UA_Argument_init(&outputArgument);
    outputArgument.description = UA_LOCALIZEDTEXT("en-US", "Cypher query result");
    outputArgument.name = UA_STRING("CypherQueryResult");
    outputArgument.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    outputArgument.valueRank = UA_VALUERANK_SCALAR;

    UA_MethodAttributes helloAttr = UA_MethodAttributes_default;
    helloAttr.description = UA_LOCALIZEDTEXT("en-US", "Say `Hello World`");
    helloAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Hello World");
    helloAttr.executable = true;
    helloAttr.userExecutable = true;
    UA_Server_addMethodNode(server,
                            UA_NODEID_NUMERIC(1, 62541),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, "Query"),
                            helloAttr,
                            &queryCallback,
                            1,
                            &inputArgument,
                            1,
                            &outputArgument,
                            NULL,
                            NULL);
}

int main(int argc, char* argv[])
{
    UA_Server* server = UA_Server_new();

    addQueryMethod(server);

    for (int cnt = 1; cnt < argc; cnt++)
    {
        if (UA_StatusCode_isBad(UA_Server_loadNodeset(server, argv[cnt], NULL)))
        {
            printf("Nodeset %s could not be loaded, exit\n", argv[cnt]);
            return EXIT_FAILURE;
        }
    }

    UA_StatusCode retval = UA_Server_runUntilInterrupt(server);

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}