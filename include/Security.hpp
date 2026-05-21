#include <string>

class Security
{
    public:
    static std::string DecodeURL(const std::string& url);
    static bool isPathSafe(const std::string& path);
};