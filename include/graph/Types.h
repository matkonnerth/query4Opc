#pragma once
#include <open62541/types.h>
#include <vector>
namespace graph
{
using path_element_t = UA_ReferenceDescription;
using path_t = std::vector<path_element_t>;
using column_t = std::vector<path_element_t>;

}