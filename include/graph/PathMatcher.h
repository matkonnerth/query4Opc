#pragma once
#include "Filter.h"
#include <open62541/server.h>
#include <open62541/types.h>
#include <optional>
#include <vector>

/*
   referenceType: the reference to match, UA_NODEID_NULL for any referenceType
*/
struct PathElement
{
   UA_NodeId referenceType;
   UA_NodeClass nodeClass;
   std::optional<UA_NodeId> targetId;
   UA_BrowseDirection direction;
};

using path_t = std::vector<UA_ReferenceDescription>;
class PathMatcher
{
public:
   PathMatcher(UA_Server* server, const std::vector<PathElement>& path, size_t startIndex = 0)
   : m_server{ server }
   , m_path{ path }
   , m_idx{ startIndex }
   {
      // add the result vectors
      //+1 for the start node
      for (auto i = 0u; i < path.size() + 1; i++)
      {
         m_results.emplace_back(std::vector<UA_ReferenceDescription>{});
      }
   }

   PathMatcher(PathMatcher&& other)
   : m_server{ other.m_server }
   , m_path{ other.m_path }
   , m_idx{ other.m_idx }
   , m_results{ other.m_results }
   {}


   PathMatcher& operator=(PathMatcher&& other)
   {
      m_server = other.m_server;
      m_path = std::move(other.m_path);
      m_idx = other.m_idx;
      m_results = std::move(other.m_results);
      return *this;
   }

   void match(const UA_ReferenceDescription& startNode)
   {
      // empty Path
      if (m_path.empty())
      {
         m_results[0].emplace_back(startNode);
         return;
      }

      for (const auto& p : checkPath(startNode))
      {
         // add the path to the results
         for (auto i = 0u; i < p.size(); ++i)
         {
            m_results[i].emplace_back(std::move(p[i]));
         }
      }
   }

   const std::vector<UA_ReferenceDescription>* results() const
   {
      return &m_results[0];
   }

   const std::vector<UA_ReferenceDescription>* results(size_t idx) const
   {
      if(idx<m_results.size())
      {
         return &m_results[idx];
      }
      return nullptr;
   }

private:
   std::vector<path_t> checkPath(const UA_ReferenceDescription& startNode)
   {
      auto result = checkRightSide(startNode);

      std::vector<path_t> endResult{};
      for(auto&p: result)
      {
         if(checkLeftSide(p))
         {
            endResult.emplace_back(p);
         }
      }
      return endResult;
   }

   // returns paths satisfying the right side
   std::vector<path_t> checkRightSide(const UA_ReferenceDescription& start)
   {
      std::vector<path_t> paths;
      path_t actPath{};
      actPath.push_back(start);

      std::optional<UA_ReferenceDescription> lastRef{ std::nullopt };
      std::optional<PathMatcher> lastMatcher{ std::nullopt };

      for (auto it = m_path.cbegin() + static_cast<int>(m_idx); it != m_path.cend(); it++)
      {
         UA_BrowseDescription bd;
         UA_BrowseDescription_init(&bd);
         bd.browseDirection = it->direction;
         bd.includeSubtypes = true;
         bd.referenceTypeId = it->referenceType;
         bd.resultMask = UA_BROWSERESULTMASK_NONE;
         bd.nodeId = actPath.back().nodeId.nodeId;
         bd.nodeClassMask = it->nodeClass;
         UA_BrowseResult br = UA_Server_browse(m_server, 1000, &bd);
         if (br.statusCode != UA_STATUSCODE_GOOD)
         {
            UA_BrowseResult_clear(&br);
            return std::vector<path_t>{};
         }

         if (it->targetId)
         {
            for (const auto* ref = br.references; ref != br.references + br.referencesSize; ++ref)
            {
               if (UA_NodeId_equal(&it->targetId.value(), &ref->nodeId.nodeId))
               {
                  lastRef = *ref;
                  break;
               }
            }
         }
         else
         {
            // we have to run over each possible sub path and check the remaining references
            // current implementation is wrong

            // we have to instantiate a pathMatcher for each of this subPaths
            PathMatcher m{ m_server, std::vector<PathElement>{ it+1, m_path.cend() } };
            for (const auto* ref = br.references; ref != br.references + br.referencesSize; ++ref)
            {
               m.match(*ref);
            }
            lastMatcher.emplace(std::move(m));
         }
         UA_BrowseResult_clear(&br);

         if (lastRef)
         {
            actPath.push_back(*lastRef);
            lastRef = std::nullopt;
         }
         else if(lastMatcher)
         {
            std::vector<path_t> res;
            //add the results
            if(lastMatcher->results())
            {
               size_t maxPaths = 0;
               if(lastMatcher->results())
               {
                  maxPaths = lastMatcher->results()->size();
               }
               for(auto i =0u; i<maxPaths; i++)
               {
                  size_t c=0;
                  path_t newPath{actPath};
                  while (const auto* p = lastMatcher->results(c))
                  {
                     newPath.emplace_back(lastMatcher->results(c)->at(i));
                     c += 1;
                  }
                  res.emplace_back(newPath);
               }
            }
            return res;
         }
         else
         {
            return std::vector<path_t>{};
         }
      }
      paths.emplace_back(actPath);
      actPath.clear();
      return paths;
   }


   bool checkLeftSide(std::vector<UA_ReferenceDescription>& actPath)
   {
      for (auto it = m_path.crbegin() + static_cast<int>(m_path.size() - m_idx); it != m_path.crend(); it++)
      {
         UA_BrowseDescription bd;
         UA_BrowseDescription_init(&bd);
         bd.browseDirection = it->direction;
         bd.includeSubtypes = true;
         bd.referenceTypeId = it->referenceType;
         bd.resultMask = UA_BROWSERESULTMASK_NONE;
         bd.nodeId = actPath.front().nodeId.nodeId;
         bd.nodeClassMask = it->nodeClass;
         UA_BrowseResult br = UA_Server_browse(m_server, 1000, &bd);
         if (br.statusCode != UA_STATUSCODE_GOOD)
         {
            UA_BrowseResult_clear(&br);
            return false;
         }

         auto result = false;
         if (it->targetId)
         {
            for (const auto* ref = br.references; ref != br.references + br.referencesSize; ++ref)
            {
               if (UA_NodeId_equal(&it->targetId.value(), &ref->nodeId.nodeId))
               {
                  actPath.insert(actPath.begin(), *ref);
                  result = true;
                  break;
               }
            }
         }
         else
         {
            // we have no clear startId
            for (const auto* ref = br.references; ref != br.references + br.referencesSize; ++ref)
            {
               actPath.insert(actPath.begin(), *ref);
               result = true;
               break;
            }
         }
         UA_BrowseResult_clear(&br);
         if (!result)
         {
            return false;
         }
      }
      return true;
   }

   UA_Server* m_server;
   std::vector<PathElement> m_path;
   std::vector<std::vector<UA_ReferenceDescription>> m_results;
   size_t m_idx{ 0 };
};