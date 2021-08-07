#pragma once
#include <open62541/server.h>
#include <open62541/types.h>
#include <optional>
#include <vector>
#include "Filter.h"

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

class PathMatcher
{
public:
   PathMatcher(UA_Server* server, const std::vector<PathElement>& path)
   : m_server{ server }
   , m_path{ path }
   {
       //add the result vectors
       if(path.empty())
       {
           m_results.emplace_back(std::vector<UA_ReferenceDescription>{});
       }
       else
       {
           //+1 for the start node
           for(auto i =0u; i<path.size()+1; i++)
           {
              m_results.emplace_back(std::vector<UA_ReferenceDescription>{});
           }
       }
   }

   void match(const UA_ReferenceDescription& startNode)
   {
      std::vector<UA_ReferenceDescription> actPath;
      actPath.push_back(startNode);
      // empty Path
      if (m_path.empty())
      {
         m_results[0].emplace_back(startNode);
         return;
      }

      if (checkPath(actPath))
      {
         // add the path to the results
         for(auto i=0u; i< actPath.size(); ++i)
         {
             m_results[i].emplace_back(std::move(actPath[i]));
         }
      }
   }

   const std::vector<UA_ReferenceDescription>* results() const
   {
      return &m_results[0];
   }

private:
   bool checkPath(std::vector<UA_ReferenceDescription>& actPath)
   {
      for (const auto& e : m_path)
      {
         UA_BrowseDescription bd;
         UA_BrowseDescription_init(&bd);
         bd.browseDirection = e.direction;
         bd.includeSubtypes = true;
         bd.referenceTypeId = e.referenceType;
         bd.resultMask = UA_BROWSERESULTMASK_NONE;
         bd.nodeId = actPath.back().nodeId.nodeId;
         bd.nodeClassMask = e.nodeClass;
         UA_BrowseResult br = UA_Server_browse(m_server, 1000, &bd);
         if (br.statusCode != UA_STATUSCODE_GOOD)
         {
            UA_BrowseResult_clear(&br);
            return false;
         }

         auto result = false;
         if (e.targetId)
         {
            for (const auto* ref = br.references; ref != br.references + br.referencesSize; ++ref)
            {
               if (UA_NodeId_equal(&e.targetId.value(), &ref->nodeId.nodeId))
               {
                  actPath.push_back(*ref);
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
               actPath.push_back(*ref);
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
   size_t idx{ 0 };

   std::vector<std::vector<UA_ReferenceDescription>> m_results;
};