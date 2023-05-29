#include <graph/SourceColumn.h>

using graph::SourceColumn;

SourceColumn::SourceColumn(const column_t& c)
: col{ c }
{}

void SourceColumn::generate(const std::function<void(path_element_t&&)>& filter) const
{
    // TODO: unnecessary copy?
    for (auto pe : col)
    {
        filter(std::move(pe));
    }
}

std::string SourceColumn::explain() const
{
    std::string explanation{ "SourceColumn\n" };
    return explanation;
}