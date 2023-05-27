#include <open62541/client_config_default.h>
#include <open62541/client_highlevel_async.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server_config_default.h>

#include <iostream>
#include <mutex>
#include <optional>
#include <signal.h>
#include <stdlib.h>
#include <thread>
#include <unordered_map>

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
        std::cout << result;
    }
}

void invokeQuery(UA_Client* client, const std::string& query)
{

    UA_Variant var{};
    UA_String queryString{};
    queryString.data = (UA_Byte*)query.data();
    queryString.length = query.length();
    UA_Variant_setScalarCopy(&var, &queryString, &UA_TYPES[UA_TYPES_STRING]);
    UA_UInt32 id{};
    UA_Client_call_async(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_NODEID_NUMERIC(1, 62541), 1u, &var, methodCalled, nullptr, &id);
}

class Commands
{
 public:
    void query(const std::string& query)
    {
        std::scoped_lock<std::mutex> m_lock{ m_mutex };
        m_queries.emplace(std::make_pair(query, ""));
    }

    std::optional<std::string> getNextQuery()
    {
        std::scoped_lock<std::mutex> m_lock{ m_mutex };
        if (m_queries.empty())
        {
            return std::nullopt;
        }
        auto s = m_queries.begin()->first;
        m_queries.erase(m_queries.begin());
        return s;
    }

 private:
    std::unordered_map<std::string, std::string> m_queries{};
    std::mutex m_mutex{};
};

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


int main(int argc, char* argv[])
{
    signal(SIGINT, stopHandler); /* catches ctrl-c */

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

    std::thread inputThread{ input, &c };

    /* Endless loop runAsync */
    while (running)
    {
        UA_Client_run_iterate(client, 100);
        if(auto q = c.getNextQuery(); q)
        {
            invokeQuery(client, *q);
        }
    }

    /* Clean up */
    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return EXIT_SUCCESS;
}
