#pragma once
#include "Path.h"
#include <variant>

namespace cypher {
struct Match
{
    Path path;
};

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