#pragma once
#include <optional>
#include <string>
#include <unordered_map>

struct Node
{
    std::optional<std::string> identifier; 
    std::optional<std::string> label; // Object, Variable, ObjectType
    std::unordered_map<std::string, std::string> properties;

    const std::string* NodeId() const
    {
        auto it = properties.find("NodeId");
        if(it!=properties.end())
        {
            return &it->second;
        }
        return nullptr;
    }
};

struct Relationship
{
    std::optional<std::string> type; // HasTypeDefinition, HasProperty
};

struct SimplePath
{
    Node m_nodeA;
    Node m_nodeB;
    Relationship m_rel;
};
