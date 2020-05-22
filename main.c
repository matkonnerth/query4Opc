#include <NodesetLoader/backendOpen62541.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <signal.h>
#include <stdlib.h>

static volatile UA_Boolean running = true;
static void stopHandler(int sig) {
  UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
  running = false;
}

void iterate(UA_Server *server, const UA_NodeId *id) {
  UA_BrowseDescription bd;
  UA_BrowseDescription_init(&bd);
  bd.browseDirection = UA_BROWSEDIRECTION_FORWARD;
  bd.includeSubtypes = true;
  bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES);
  // bd.resultMask = UA_BROWSERESULTMASK_ALL;
  bd.resultMask = UA_BROWSERESULTMASK_BROWSENAME;
  bd.nodeId = *id;
  bd.nodeClassMask = UA_NODECLASS_OBJECT;
  // bd.nodeClassMask
  UA_BrowseResult br = UA_Server_browse(server, 1000, &bd);
  if (br.statusCode == UA_STATUSCODE_GOOD) {
    for (UA_ReferenceDescription *rd = br.references;
         rd != br.references + br.referencesSize; rd++) {

      
      const char* bn = "Heating";
      if (strlen(bn) == rd->browseName.name.length &&
          !strncmp(bn, rd->browseName.name.data,
                   strlen(bn)))
      {
          printf("found :printf %.*s\n", rd->browseName.name.length,
                 rd->browseName.name.data);
      }

      iterate(server, &rd->nodeId.nodeId);
    }
  }
  UA_BrowseResult_clear(&br);
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
  UA_NodeId startId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  iterate(server, &startId);
  UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "finished");

  UA_Server_run(server, &running);
  UA_Server_delete(server);
}
