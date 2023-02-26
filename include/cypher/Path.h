#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace cypher
{
struct Node
{
   std::optional<std::string> identifier;
   std::optional<std::string> label; // Object, Variable, ObjectType
   std::unordered_map<std::string, std::string> properties{};

   std::optional<std::string> NodeId() const
   {
      auto it = properties.find("NodeId");
      if (it != properties.end())
      {
         return it->second;
      }
      return std::nullopt;
   }

   std::optional<std::string> TypeDefinitionId() const
   {
       auto it = properties.find("TypeDefinitionId");
       if (it != properties.end())
       {
           return it->second;
       }
       return std::nullopt;
   }
};

struct Relationship
{
   std::optional<std::string> type; // HasTypeDefinition, HasProperty
   int direction{ 0 }; // -1 .. inverse, 0 .. no direction, 1 .. forward
};

struct Path
{
   std::vector<Node> nodes;
   std::vector<Relationship> relations;
};

struct PathIterator
{
   PathIterator(const Path& p)
   : m_path{ p }
   {}

   const Node* prevNode() const
   {
      if (static_cast<int>(idx) - 1 < 0)
      {
         return nullptr;
      }
      return &m_path.nodes[idx - 1];
   }

   const Node* nextNode() const
   {
      if (idx + 1 >= m_path.nodes.size())
      {
         return nullptr;
      }
      return &m_path.nodes[idx + 1];
   }


   const Node* currentNode() const
   {
      if (idx >= m_path.nodes.size())
      {
         return nullptr;
      }
      return &m_path.nodes[idx];
   }

   const Relationship* prevRel() const
   {
      if(idx==0)
      {
         return nullptr;
      }
      return &m_path.relations[idx-1];
   }

   const Relationship* nextRel() const
   {
      if(idx==m_path.nodes.size()-1)
      {
         return nullptr;
      }
      return &m_path.relations[idx];
   }

   void operator++(int)
   {
      idx += 1;
   }

   void operator--(int)
   {
      idx -= 1;
   }

private:
   const Path& m_path;
   size_t idx{ 0 };
};

}
