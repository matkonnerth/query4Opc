#include <NodesetLoader/backendOpen62541.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <vector>
#include <iostream>
#include "QueryLite.h"


#include <signal.h>
#include <stdlib.h>

static volatile UA_Boolean running = true;
static void stopHandler(int sig) {
  UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
  running = false;
}



int main(int argc, const char *argv[]) {
  UA_Server *server = UA_Server_new();
  UA_ServerConfig_setDefault(UA_Server_getConfig(server));

  for (int cnt = 1; cnt < argc; cnt++) {
    if (!NodesetLoader_loadFile(server, argv[cnt], NULL)) {
      printf("nodeset could not be loaded, exit\n");
      return 1;
    }
  }

  UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "start");
  auto results =
      QueryLite::lookupInstances(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                      UA_NODEID_NUMERIC(0, UA_NS0ID_SERVERTYPE));
  std::cout << "results: " << results.size() << "\n";
  UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "finished");

  //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "start");
  //results = QueryLite::lookupInstances(
  //    server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
  //    UA_NODEID_STRING(7, "Heating.TZone"));
  // std::cout << "results: " << results.size() << "\n";
  //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "finished");

  UA_Server_run(server, &running);
  UA_Server_delete(server);
}
