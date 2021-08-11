#pragma once
#include "Ast.h"

namespace cypher {
class Parser
{
 public:
    std::optional<Query> parse(const std::string& queryString);
};
} // namespace cypher
