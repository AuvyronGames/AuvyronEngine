/*
 * @project AuvyronEngine
 * @author  D4NTE / Auvyron Games
 * @file    src/core/LuaDiagnostics.hpp
 * @license MIT
 */

#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace auv {
enum class DiagnosticSeverity { Error, Warning };
struct LuaDiagnostic {
    int line = 1;
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::string message;
};
// Checks syntax in an isolated Lua VM; never executes user scripts.
// Additionally checks known AuvyronEngine API calls, excluding strings/comments.
std::vector<LuaDiagnostic> diagnoseLua(std::string_view source);
}
