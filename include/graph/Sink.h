#pragma once
#include "Types.h"
#include <string>
#include "PathResult.h"

namespace graph {

class Sink
{
 public:
    virtual void filter(path_element_t&& element) = 0;
    virtual const PathResult& results() const =  0;
    virtual ~Sink() = default;

    
};

class DefaultSink final : public Sink
{

    void filter(path_element_t&& element) override
    {
        std::vector<path_element_t> p{element};
        m_results.emplace(std::move(p));
    }


    const PathResult& results() const override
    {
        return m_results;
    }

private:
    PathResult m_results{1u};
};

} // namespace graph