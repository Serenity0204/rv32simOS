#include "Kernel.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
using namespace emscripten;

void clearLogsWrapper()
{
    CLEAR_LOGS();
}
void resetStatsWrapper()
{
    STATS.reset();
}

void flushLogsWrapper()
{
    SHOW_LOGS();
}

void printStatsWrapper()
{
    STATS.printSummary();
}
EMSCRIPTEN_BINDINGS(kernel_module)
{
    function("flushLogs", &flushLogsWrapper);
    function("printStats", &printStatsWrapper);
    function("clearLogs", &clearLogsWrapper);
    function("resetStats", &resetStatsWrapper);

    class_<Kernel>("Kernel")
        .constructor<>()
        .function("createProcess", &Kernel::createProcess)
        .function("killProcess", &Kernel::killProcess)
        .function("init", &Kernel::init)
        .function("step", &Kernel::step)
        .function("isRunning", &Kernel::isRunning);
}

int main()
{
    return 0;
}

#else

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ./rv32simOS <elf_file> -<log flag>\n";
        return 1;
    }

    std::string filename = std::string(argv[1]);
    bool printLogs = argc == 3 && std::string(argv[2]) == "-log" ? true : false;

    Kernel kernel;

    bool good = kernel.createProcess(filename);
    if (!good)
    {
        std::cout << "Failed to create process.\n";
        return 1;
    }

    kernel.init();

    while (kernel.isRunning())
        kernel.step();

    if (printLogs) SHOW_LOGS();

    STATS.printSummary();
    return 0;
}

#endif