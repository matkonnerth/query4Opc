#pragma once
#include "Sink.h"
#include <memory>
#include <open62541/server.h>
#include <vector>
#include <optional>

struct Result
{
   UA_NodeId parentId;
   UA_ReferenceDescription target;
};

template <typename T>
class AbstractFilter
{
public:
   virtual void filter(T&& input) = 0;
   virtual AbstractFilter<T>& append(AbstractFilter<T>& filter) = 0;
   virtual void append(Sink<T>& sink) = 0;
   virtual ~AbstractFilter() = default;
};

template <typename T, typename Derived>
class Filter : public AbstractFilter<T>
{
public:
   void filter(T&& input) override
   {
      if (match(input))
      {
         if (m_nextFilter)
         {
            m_nextFilter->filter(std::move(input));
         }
         else
         {
            m_sink->emplace_back(std::move(input));
         }
      }
   }

   /*
     append a filter, returns the appended filter
   */
   AbstractFilter<T>& append(AbstractFilter<T>& filter)
   {
      m_nextFilter = &filter;
      return *m_nextFilter;
   }

   void append(Sink<T>& s)
   {
      m_sink = &s;
   }

private:
   bool match(const T& input)
   {
      Derived* d = static_cast<Derived*>(this);
      return d->match(input);
   }
   AbstractFilter<T>* m_nextFilter{ nullptr };
   Sink<T>* m_sink{ nullptr };
};

template <typename T>
class TakeAllFilter : public Filter<T, TakeAllFilter<T>>
{
public:
   bool match(const T&)
   {
      return true;
   }
};

/*
  matches nodes which have one of the given typedefinitions
  is a special case of a ReferenceFilter
*/
template <typename T>
class TypeFilter : public Filter<T, TypeFilter<T>>
{
public:
   /*
     construct TypeFilter with a single typeId
   */
   TypeFilter(const UA_NodeId& typeId)
   {
      UA_ReferenceDescription typeRef{};
      typeRef.nodeId.nodeId = typeId;
      m_types.emplace_back(Result{ UA_NODEID_NUMERIC(0, 0), typeRef });
   }
   TypeFilter(const std::vector<T>& types)
   : m_types{ types }
   {}

   bool match(const T& input)
   {
      for (const auto& type : m_types)
      {
         if (UA_NodeId_equal(&type.target.nodeId.nodeId, &input.target.typeDefinition.nodeId))
         {
            return true;
         }
      }
      return false;
   }

private:
   std::vector<T> m_types;
};

/*
  matches nodes which have references with a certain type

  impl:
  much of the referenceDescriptionMatcher should be moved to the browse request
*/

class ReferenceDescriptionMatcher
{
public:
   virtual bool match(const UA_ReferenceDescription& ref) = 0;
   virtual ~ReferenceDescriptionMatcher() = default;
};

class MatchAnyReference : public ReferenceDescriptionMatcher
{
public:
   bool match(const UA_ReferenceDescription&) override
   {
      return true;
   }
};

class MatchTargetNodeId : public ReferenceDescriptionMatcher
{
public:
   MatchTargetNodeId(const UA_NodeId targetId)
   : m_targetId{ targetId } {};
   bool match(const UA_ReferenceDescription& ref) override
   {
      return UA_NodeId_equal(&m_targetId, &ref.nodeId.nodeId);
   }

private:
   UA_NodeId m_targetId;
};

class MatchNodeClass: public ReferenceDescriptionMatcher
{
public:
   MatchNodeClass(UA_NodeClass nodeClass)
   : m_nodeClass{ nodeClass } {};
   bool match(const UA_ReferenceDescription& ref) override
   {
      return ref.nodeClass == m_nodeClass;
   }

private:
   UA_NodeClass m_nodeClass;
};

/*
   referenceType: the reference to match, UA_NODEID_NULL for any referenceType
*/
struct PathElement
{
   UA_NodeId referenceType;
   UA_NodeClass nodeClass;
   std::optional<UA_NodeId> targetId;
   //unused
   UA_BrowseDirection direction;
};

template <typename T>
class ReferenceFilter : public Filter<T, ReferenceFilter<T>>
{
public:
   ReferenceFilter(UA_Server* server, const std::vector<PathElement>& path)
   : m_server{ server }
   , m_path{ path }
   {}

   bool match(const T& input)
   {
      return followPath(input.target.nodeId.nodeId);
   }

private:
   bool followPath(const UA_NodeId& startId)
   {
      // check, if we are finished
      if (idx >= m_path.size())
      {
         idx -= 1;
         return true;
      }
      UA_BrowseDescription bd;
      UA_BrowseDescription_init(&bd);
      bd.browseDirection = UA_BROWSEDIRECTION_BOTH;
      bd.includeSubtypes = true;
      bd.referenceTypeId = m_path[idx].referenceType;
      bd.resultMask = UA_BROWSERESULTMASK_NONE;
      bd.nodeId = startId;
      bd.nodeClassMask = m_path[idx].nodeClass;
      UA_BrowseResult br = UA_Server_browse(m_server, 1000, &bd);
      auto result = false;
      if (br.statusCode != UA_STATUSCODE_GOOD)
      {
         UA_BrowseResult_clear(&br);
         return false;
      }

      UA_NodeId nextStartId = UA_NODEID_NULL;

      if(m_path[idx].targetId)
      {
         for (const auto* ref = br.references; ref != br.references + br.referencesSize; ++ref)
         {
            if(UA_NodeId_equal(&m_path[idx].targetId.value(), &ref->nodeId.nodeId))
            {
               nextStartId = *m_path[idx].targetId;
               result = true;
               break;
            }
         }
      }
      else
      {
         //we have no clear startId
         if(br.referencesSize>0)
         {
            for (const auto* ref = br.references; ref != br.references + br.referencesSize; ++ref)
            {
               nextStartId = ref->nodeId.nodeId;
               idx += 1;
               result = followPath(nextStartId);
               //we have a matching subPath
               if(result)
               {
                  break;
               }
            }
         }
      }
      UA_BrowseResult_clear(&br);
      idx-=1;
      return result;
   }

   UA_Server* m_server;
   std::vector<PathElement> m_path;
   size_t idx {0};
};
