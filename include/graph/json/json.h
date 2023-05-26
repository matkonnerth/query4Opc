#pragma once
#include <graph/PathResult.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

void to_json(json& j, const UA_ReferenceDescription& e)
{
    UA_String idString{};
    UA_NodeId_print(&e.nodeId.nodeId, &idString);
    std::string standardString{};
    standardString.assign((char*)idString.data, idString.length);
    j["id"] = standardString;
}

void to_json(json& j, const std::vector<UA_ReferenceDescription>& path)
{
    json nodes{};
    for (const auto& e : path)
    {
        nodes.push_back(e);
    }
    j["nodes"] = nodes;
}
namespace graph{




void to_json(json& j, const PathResult& result)
{
    for(const auto& p: result.paths())
    {
        j["paths"].push_back(p);
    }
}

std::string json_encode(const PathResult& result)
{
    json obj = result;
    return obj.dump(2);
}

}