#include <anari/anari.h>

#include <iostream>
#include <stdexcept>

int main()
{
    auto anLib = anariLoadLibrary("helide");
    if (!anLib)
    {
        throw std::runtime_error("Failed to load \"helide\" ANARI library");
    }
    std::cout << "Successfully loaded \"helide\" ANARI library" << std::endl;

    return 0;
}