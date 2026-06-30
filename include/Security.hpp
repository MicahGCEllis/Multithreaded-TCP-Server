#pragma once

#include <string>

// A standalone security module designed to sanitize inputs and block malicious payloads
class Security
{
    public:
        // Translates encouded hexadecimal chracters (e.g., %20) back to the standard ASCII
        static std::string DecodeURL(const std::string& url);

        // Evaluates a path for directory traversal or absolute root injection
        static bool isPathSafe(const std::string& path);
};