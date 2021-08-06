#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct Node
{
   std::optional<std::string> identifier;
   std::optional<std::string> label; // Object, Variable, ObjectType
   std::unordered_map<std::string, std::string> properties;

   const std::string* NodeId() const
   {
      auto it = properties.find("NodeId");
      if (it != properties.end())
      {
         return &it->second;
      }
      return nullptr;
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
      auto relIndex = static_cast<int>(idx) - 1;
      if (relIndex < 0)
      {
         return nullptr;
      }
      return &m_path.relations[relIndex];
   }

   const Relationship* nextRel() const
   {
      auto relIndex = idx;
      if (relIndex >= m_path.relations.size())
      {
         return nullptr;
      }
      return &m_path.relations[relIndex];
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

struct SimplePath
{
   Node m_nodeA;
   Node m_nodeB;
   Relationship m_rel;
};

struct AppendedPath
{
   Node m_node;
   Relationship m_rel;
};

struct EmptyPath
{
   Node m_node;
};

struct SplittedPaths
{
   std::optional<SimplePath> simplePath;
   std::optional<EmptyPath> emptyPath;
   std::vector<AppendedPath> appendedPaths;
};

std::optional<SplittedPaths> splitPaths(const Path& p);