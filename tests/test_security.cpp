#include "Security.hpp"
#include <iostream>
#include <cassert>

int main ()
{
    assert(Security::isPathSafe("index.html"));
    assert(!Security::isPathSafe("../secrets.txt"));
    assert(!Security::isPathSafe("/etc/passwd"));
    assert(!Security::isPathSafe("C:/Windows"));
    assert(Security::isPathSafe(""));
    assert(!Security::isPathSafe("%2E%2E/secrets.txt"));
    
    std::cout << "Security Checkpoint Success";
    return 0;
}