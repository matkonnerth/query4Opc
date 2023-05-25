#pragma once
#include <graph/PathResult.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace graph{

void to_json(json& j, const path_element_t& e)
{
    UA_String idString{};
    UA_NodeId_print(&e.nodeId.nodeId, &idString);
    std::string standardString{};
    standardString.assign((char*)idString.data, idString.length);
    j = json{{"id", standardString}};
}

void to_json(json& j, const path_t& path)
{
    json obj;
    for(const auto& e: path)
    {
        json je = e;
        obj.push_back(je);
    }
    j=obj;
    
}

void to_json(json& j, const PathResult& result)
{
    json obj{};
    for(const auto& p: result.paths())
    {
        json jp = p;
        obj.push_back(jp);
    }
    j = obj;
}

std::string json_encode(const PathResult& result)
{
    json obj = result;
    return obj.dump();
}

}