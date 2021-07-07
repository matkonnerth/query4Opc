#include "Filter.h"
#include <iostream>

#include <gtest/gtest.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <NodesetLoader/backendOpen62541.h>

std::string path = "";

TEST(import, nodeclass) {
  UA_Server *server = UA_Server_new();
  UA_ServerConfig_setDefault(UA_Server_getConfig(server));


  ASSERT_TRUE(NodesetLoader_loadFile(server, (path+"/graph.xml").c_str(), NULL));

  UA_Server_delete(server);
}

TEST(filter, basics)
{
  std::vector<UA_ReferenceDescription> refs;
  UA_ReferenceDescription ref;
  ref.typeDefinition = UA_EXPANDEDNODEID_NUMERIC(0, 58);
  ref.nodeId = UA_EXPANDEDNODEID_NUMERIC(1, 10000);
  for (auto i = 0; i < 100; ++i) {

    refs.emplace_back(ref);
  }

  Sink<Result> s{};
  UA_ReferenceDescription typeRef{};
  typeRef.nodeId.nodeId = UA_NODEID_NUMERIC(0, 58);
  TypeFilter<Result> typeFilter{
      std::vector<Result>{Result{UA_NODEID_NUMERIC(0, 58), typeRef}}};
  TypeFilter<Result> typeFilter2{
      std::vector<Result>{Result{UA_NODEID_NUMERIC(0, 58), typeRef}}};

  TakeAllFilter<Result> ta{};

  typeFilter.connect(&typeFilter2);
  typeFilter2.connect(&ta);
  ta.connect(&s);

  for (const auto &r : refs) {
    typeFilter.filter(Result{UA_NODEID_NUMERIC(0, 85), r});
  }

  ASSERT_EQ(s.results().size(), 100);
}

int main(int argc, char **argv) {

  testing::InitGoogleTest(&argc, argv);

  if (!(argc > 1))
    return 1;
  path = argv[1];

  return RUN_ALL_TESTS();
}