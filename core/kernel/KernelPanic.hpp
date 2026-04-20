#pragma once
#include <cstdlib>
#include <string>

inline void panic_impl(std::string reason, const char* file, int line)
{
    std::cerr << "[KERNEL PANIC] Message: \"" << reason << "\" (" << file << ":" << line << ")" << std::endl;
    std::exit(EXIT_FAILURE);
}

#define PANIC(reason) panic_impl(reason, __FILE__, __LINE__)