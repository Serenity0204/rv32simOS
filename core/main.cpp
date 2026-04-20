#include "Kernel.hpp"
#include "KernelInstance.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>
#include <vector>

static std::vector<std::string> parseArgs(int argc, char* argv[])
{
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) args.push_back(std::string(argv[i]));
    return args;
}

static bool parseEnableLogs(const std::vector<std::string>& args)
{
    for (const std::string& arg : args)
    {
        if (arg == "-logs")
            return true;
    }
    return false;
}

static std::vector<std::string> parseElfFiles(const std::vector<std::string>& args)
{
    std::vector<std::string> elfFiles;
    for (const std::string& arg : args)
    {
        if (arg.size() >= 4 && arg.substr(arg.size() - 4) == ".elf")
            elfFiles.push_back(arg);
    }
    return elfFiles;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ./rv32umos <elf_file 1> <elf_file 2> <elf_file 3> ... <-logs>\n";
        return 1;
    }

    std::vector<std::string> args = parseArgs(argc, argv);

    bool printLogs = parseEnableLogs(args);
    std::vector<std::string> filenames = parseElfFiles(args);
    for (const std::string& filename : filenames)
    {
        bool ok = kernel.createProcess(filename);
        if (!ok)
        {
            std::cout << "Create Process with filename:" << filename << " failed. " << std::endl;
            return 1;
        }
    }
    kernel.init();

    if (printLogs) SHOW_LOGS();
    STATS.printSummary();
    return 0;
    return 0;
}
