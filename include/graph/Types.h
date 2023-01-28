#pragma once
#include <vector>
#include <open62541/types.h>

namespace graph
{
using path_element_t = UA_ReferenceDescription;
using path_t = std::vector<path_element_t>;
using column_t = std::vector<path_element_t>;
}