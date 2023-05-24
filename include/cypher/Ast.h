#pragma once
#include "Path.h"
#include <variant>

namespace cypher {

struct Return
{
    std::string identifier;
};

struct Query
{
    std::vector<Match> matchClauses;
    Return returnClause;
};
} // namespace cypher