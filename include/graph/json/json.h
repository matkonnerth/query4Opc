#pragma once
#include <graph/PathResult.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace graph{

void to_json(json& j, const ReferenceDescription& e)
{
    UA_String idString{};
    UA_NodeId_print(&e.impl().nodeId.nodeId, &idString);
    std::string standardString{ (char*)idString.data, idString.length };
    UA_String_clear(&idString);
    j["id"] = standardString;
}

void to_json(json& j, const std::vector<ReferenceDescription>& path)
{
    json nodes{};
    for (const auto& e : path)
    {
        nodes.push_back(e);
    }
    j["nodes"] = nodes;
}


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