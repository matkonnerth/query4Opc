#include <open62541/client_config_default.h>
#include <open62541/client_highlevel_async.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server_config_default.h>

#include "Commands.h"
#include <httplib.h>
#include <iostream>
#include <optional>
#include <signal.h>
#include <stdlib.h>
#include <thread>

using namespace httplib;

UA_Boolean running = true;

static void stopHandler(int sig)
{
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

static void stateCallback(UA_Client* client,
                          UA_SecureChannelState channelState,
                          UA_SessionState sessionState,
                          UA_StatusCode connectStatus)
{
    if (sessionState == UA_SESSIONSTATE_ACTIVATED)
    {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "A session with the server is activated");
    }

    if (sessionState == UA_SESSIONSTATE_CLOSED)
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Session disconnected");
}

static void methodCalled(UA_Client* client, void* userdata, UA_UInt32 requestId, UA_CallResponse* response)
{
    UA_LOG_INFO(UA_Log_Stdout,
                UA_LOGCATEGORY_USERLAND,
                "**** CallRequest Response - Req:%u with %u results",
                requestId,
                (UA_UInt32)response->resultsSize);
    UA_StatusCode retval = response->responseHeader.serviceResult;
    if (retval != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout,
                     UA_LOGCATEGORY_USERLAND,
                     "**** CallRequest Response - Req:%u FAILED",
                     requestId);
        return;
    }

    for (size_t i = 0; i < response->resultsSize; i++)
    {
        retval = response->results[i].statusCode;
        if (retval != UA_STATUSCODE_GOOD)
        {
            UA_CallResponse_clear(response);
            UA_LOG_INFO(UA_Log_Stdout,
                        UA_LOGCATEGORY_USERLAND,
                        "**** CallRequest Response - Req: %u (%lu) failed",
                        requestId,
                        (unsigned long)i);
            continue;
        }

        std::string result{};


        UA_LOG_INFO(UA_Log_Stdout,
                    UA_LOGCATEGORY_USERLAND,
                    "---Method call was successful, returned %lu values.\n",
                    (unsigned long)response->results[i].outputArgumentsSize);


        auto uaString = (UA_String*)response->results[i].outputArguments->data;
        result.assign((char*)uaString->data, uaString->length);

        auto commands = (Commands*)userdata;
        std::cout << result;
        commands->queryResponse(result);
    }
}

void invokeQuery(UA_Client* client, const std::string& query, Commands& c)
{

    UA_Variant var{};
    UA_String queryString{};
    queryString.data = (UA_Byte*)query.data();
    queryString.length = query.length();
    UA_Variant_setScalarCopy(&var, &queryString, &UA_TYPES[UA_TYPES_STRING]);
    UA_UInt32 id{};
    UA_Client_call_async(client,
                         UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                         UA_NODEID_NUMERIC(1, 62541),
                         1u,
                         &var,
                         methodCalled,
                         &c,
                         &id);
}


/*
void input(Commands* c)
{
    while (running)
    {
        for (std::string line; std::getline(std::cin, line);)
        {
            c->query(line);
        }
    }
}
*/

void input(Commands* c, std::string indexHtml)
{
    Server svr;
    svr.Get("/", [&indexHtml](const Request& /*req*/, Response& res) {
        res.set_content(indexHtml, "text/html");
    });

    svr.Post("/query", [&c](const Request& req, Response& res) {
        using namespace std::chrono_literals;
        auto queryString = req.get_file_value("queryString").content;
        std::cout << queryString << "\n";
        auto f = c->query(queryString);

        switch (auto status = f.wait_for(1s); status)
        {
        case std::future_status::deferred:
            std::cout << "deferred\n";
            res.set_content("query timeout", "text/plain");
            break;
        case std::future_status::timeout:
            res.set_content("query timeout", "text/plain");
            break;
        case std::future_status::ready:
            res.set_content(f.get(), "text/plain");
            break;
        }
    });
    svr.listen("0.0.0.0", 12121);
}


int main(int argc, char* argv[])
{
    signal(SIGINT, stopHandler); /* catches ctrl-c */

    char buf[500]{};
    readlink("/proc/self/exe", buf, sizeof(buf));
    std::cout << buf << "\n";

    std::string binaryPath = buf;
    auto pos = binaryPath.find_last_of('/');
    binaryPath.erase(pos);


    std::cout << "binary path: " << binaryPath << "\n";
    // get index.html
    std::string indexHtml{};
    {
        std::ifstream index(binaryPath + "/index.html");
        std::stringstream buffer;
        buffer << index.rdbuf();
        index.close();
        indexHtml = buffer.str();
    }

    UA_Client* client = UA_Client_new();
    UA_ClientConfig* cc = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(cc);
    /* we use a high timeout because there may be other client and
     * processing may take long if many method calls are waiting */
    cc->timeout = 60000;

    /* Set stateCallback */
    cc->stateCallback = stateCallback;


    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:4840");
    if (retval != UA_STATUSCODE_GOOD)
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Not connected. Retrying to connect in 1 second");
        UA_Client_delete(client);
        return EXIT_SUCCESS;
    }

    Commands c{};

    std::thread inputThread{ input, &c, indexHtml };


    /* Endless loop runAsync */
    while (running)
    {
        UA_Client_run_iterate(client, 100);
        if (auto q = c.getNextQuery(); q)
        {
            invokeQuery(client, *q, c);
        }
    }

    /* Clean up */
    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return EXIT_SUCCESS;
}
