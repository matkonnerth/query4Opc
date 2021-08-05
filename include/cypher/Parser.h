#pragma once
#include "Ast.h"

class Parser
{
public:
    std::optional<Query> parse(const std::string& queryString);
};