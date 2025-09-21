#pragma once

#include "pch.hpp"
#include "util/types.hpp"

namespace volt
{

class volt
{
   public:
    void lex();
    void parse();
    void convert_ast();
    void compile();
    void link();

   private:
    std::string m_src_dir;
    // lexer m_lexer;
    // parser m_parser;
    // ast_generator m_ast_generator;
    // compiler m_compiler;
    // linker m_linker;
};

}  // namespace volt
