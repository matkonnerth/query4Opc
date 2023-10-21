#pragma once
#include "ReferenceDescription.h"
#include <vector>
namespace graph
{
using path_element_t = ReferenceDescription;
using path_t = std::vector<path_element_t>;
using column_t = std::vector<path_element_t>;

}