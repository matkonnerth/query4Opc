#pragma once
#include "Source.h"

namespace graph {
class SourceColumn : public Source
{
 public:
    SourceColumn(const column_t& c);

    void generate(const std::function<void(path_element_t&&)>& filter) const override;
    std::string explain() const override;

 private:
    const column_t& col;
};
} // namespace graph