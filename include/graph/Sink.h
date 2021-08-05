#pragma once
#include <vector>

template <typename T>
class Sink
{
public:
   inline void emplace_back(T&& r)
   {
      res.emplace_back(r);
   }

   inline const auto& results() const
   {
      return res;
   }

   inline auto& results()
   {
      return res;
   }

private:
   std::vector<T> res;
};
