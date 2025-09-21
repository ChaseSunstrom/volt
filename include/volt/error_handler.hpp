#pragma once

#include "error_types.hpp"
#include "pch.hpp"
#include "util/types.hpp"

namespace volt
{

class error_handler
{
   public:
    static void        error(error_type type, const std::string& error);
    static void        parse_error_string(const std::string& string);
    static error_data& get_error_data(const std::string& string);
    static std::vector<std::reference_wrapper<error_data>> get_errors_by_reg(
        const std::string& pattern);

   private:
    // Keyed by the error string
    static std::unordered_map<std::string, std::unique_ptr<error_data>> s_errors;
};
}  // namespace volt
   // error_handler m_error_handler
