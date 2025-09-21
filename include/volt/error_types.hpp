#pragma once

#include "pch.hpp"
#include "util/types.hpp"

namespace volt
{
struct lexer_error
{
    i32_t       column;
    i32_t       row;
    std::string meta;
};

enum class error_type
{
    LEXER,
    PARSER,
    AST_GENERATOR,
    SYMANTIC_ANALYSIS,
    COMPILER,
    LINKER
};

struct error_data
{
    error_type                type;
    std::vector<std::string>  parsed_error_string;
    std::variant<lexer_error> error_v;
};
}  // namespace volt
