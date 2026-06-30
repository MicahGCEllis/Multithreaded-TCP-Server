#include "../include/Security.hpp"

std::string Security::DecodeURL(const std::string& url)
{
    std::string decoded_path;

    for (size_t i = 0; i < url.length(); ++i)
    {
        // INdentify hex-encoded character and verify bounds to prevent over-reading
        if (url[i] == '%' && i + 2 < url.length())
        {
            std::string hex = url.substr(i + 1, 2);
            int code = std::stoi(hex, nullptr, 16); // Convert base-16 string to integer
            decoded_path += static_cast<char>(code);
            i += 2; // Leapfrog over the translated hex characters

        }
        // Handle explicit spaces encoded as plus signs
        else if (url[i] == '+')
        {
            decoded_path += ' ';
        }

        else
        {
            decoded_path += url[i];
        }
    }
    return decoded_path;
}

bool Security::isPathSafe(const std::string &path)
{
    // Decode first to prevent hackers bypassing the check using URL encoding
    std::string decoded_path = DecodeURL(path);

    // Thread 1: Directory Traveral (e.g., ../)
    if (decoded_path.find("..") != std::string::npos)
    {
        return false;
    }
    // Thread 2: Drive Letter Access (e.g., C:\)
    else if (decoded_path.find(":") != std::string::npos)
    {
        return false;
    }
    // Thread 3: Absolute Root Injection (e.g., /Windows or \Windows)
    else if (!decoded_path.empty() && (decoded_path[0] =='/' || decoded_path[0] == '\\'))
    {
        return false;
    }
    else
    {
        return true; // path passsed all security constraints
    }
}