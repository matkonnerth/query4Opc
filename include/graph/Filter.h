#pragma once
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
   virtual void append(std::vector<T>& sink) = 0;
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

   void append(std::vector<T>& s)
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
   std::vector<T>* m_sink{ nullptr };
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
