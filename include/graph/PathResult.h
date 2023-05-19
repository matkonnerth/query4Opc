#pragma once
#include "Types.h"
#include <vector>

namespace graph {

/* the raw result of a query, a vector of all path_t */
class PathResult final
{
 public:
    PathResult() = default;
    PathResult(size_t columnCount)
    {
        for (auto i = 0u; i < columnCount; ++i)
        {
            data.emplace_back(std::vector<path_element_t>{});
        }
    }

    void emplace(path_t&& p)
    {
        assert(p.size() == data.size());
        size_t c = 0u;
        for (auto&& e : p)
        {
            data[c].emplace_back(std::move(e));
            ++c;
        }
    }

    const column_t* col() const
    {
        return col(0);
    }

    const column_t* col(size_t idx) const
    {
        if (idx < data.size())
        {
            return &data[idx];
        }
        return nullptr;
    }

    std::vector<path_t> paths() const
    {
        std::vector<path_t> res;
        for (auto row = 0u; row < data[0].size(); ++row)
        {
            path_t p;
            for (auto col = 0u; col < data.size(); ++col)
            {
                p.emplace_back(data[col][row]);
            }
            res.emplace_back(std::move(p));
        }
        return res;
    }

    column_t& operator[](size_t idx)
    {
        assert(idx < data.size());
        return data[idx];
    }

 private:
    std::vector<column_t> data;
};

} // namespace graph