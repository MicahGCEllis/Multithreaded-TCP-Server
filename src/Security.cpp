#include "../include/Security.hpp"

std::string Security::DecodeURL(const std::string& url)
{
    std::string decoded_path;

    for (size_t i = 0; i < url.length(); ++i)
    {
        if (url[i] == '%' && i + 2 < url.length())
        {
            std::string hex = url.substr(i + 1, 2);
            int code = std::stoi(hex, nullptr, 16);
            decoded_path += static_cast<char>(code);
            i += 2;

        }
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
    std::string decoded_path = DecodeURL(path);

    if (decoded_path.find("..") != std::string::npos)
    {
        return false;
    }
    else if (decoded_path.find(":") != std::string::npos)
    {
        return false;
    }
    else if (!decoded_path.empty() && decoded_path[0] =='/' || decoded_path[0] == '\\')
    {
        return false;
    }
    else
    {
        return true;
    }
}