#pragma once
#include <variant>
#include "Path.h"

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