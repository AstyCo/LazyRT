#include "extensions/help_functions.hpp"
#include "extensions/flatbuffers_extensions.hpp"
#include "command_line_args.hpp"
#include "types/file_system.hpp"

#include <boost/filesystem.hpp>

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROFILE(x)                                                             \
    x;                                                                         \
    prf.step(#x)
#define START_PROFILE Profiler prf(clargs.verbal());

enum LazyUTErrors { WRONG_CMD_LINE_OPTIONS = 1 };

static void concatFiles(const std::string &fin1, const std::string &fin2,
                        const std::string &fout)
{
    std::ifstream if_a(fin1, std::ios_base::binary);
    std::ifstream if_b(fin2, std::ios_base::binary);
    std::ofstream of_c(fout, std::ios_base::binary);

    of_c << if_a.rdbuf() << if_b.rdbuf();
}

int main(int argc, char *argv[])
{
    clargs.parseArguments(argc, argv);
    if (clargs.status() != CommandLineArgs::Success)
        return clargs.retCode();

    START_PROFILE;

    FileSystem filesystem;
    filesystem.setProjectDirectory(clargs.proDir());
    filesystem.setIncludePaths(clargs.includePaths());

    PROFILE(FileTreeFunc::readDirectory(filesystem.srcTree,
                                        clargs.srcDir().joint(),
                                        clargs.ignoredSubstrings()));
    PROFILE(FileTreeFunc::readDirectory(filesystem.testTree,
                                        clargs.testDir().joint(),
                                        clargs.ignoredSubstrings()));

    filesystem.installIncludeProjectDir();
    filesystem.installExtraDependencies(
        clargs.deps()); // TODO: CHECK EXTRA_DEPS

    PROFILE(FileTreeFunc::parsePhase(filesystem.srcTree,
                                     clargs.srcsDumpIn().joint()));
    PROFILE(FileTreeFunc::parsePhase(filesystem.testTree,
                                     clargs.testsDumpIn().joint()));

    PROFILE(filesystem.analyzePhase());

    if (!clargs.isNoMain())
        FileTreeFunc::labelMainAffected(filesystem.testTree);

    // Create directories to put output files to
    boost::filesystem::create_directories(clargs.outDir().joint());

    FileTreeFunc::writeModified(filesystem.srcTree,
                                clargs.srcsModified().joint());
    FileTreeFunc::writeModified(filesystem.testTree,
                                clargs.testsModified().joint());

    filesystem.installAffectedFiles();

    FileTreeFunc::writeAffected(filesystem.srcTree,
                                clargs.srcsAffected().joint());
    FileTreeFunc::writeAffected(filesystem.testTree,
                                clargs.testsAffected().joint());

    if (clargs.verbal()) {
        //        filesystem.srcTree.print();
        //        filesystem.testTree.print();

        FileTreeFunc::printAffected(filesystem.srcTree);
        FileTreeFunc::printAffected(filesystem.testTree);
        filesystem.srcTree.printModified(clargs.srcBase());
        filesystem.testTree.printModified(clargs.testBase());

        std::cout << "write lazyut files to " << clargs.outDir().joint()
                  << std::endl;
    }

    PROFILE(FileTreeFunc::serialize(filesystem.srcTree,
                                    clargs.srcsDumpOut().joint()));
    PROFILE(FileTreeFunc::serialize(filesystem.testTree,
                                    clargs.testsDumpOut().joint()));

    return 0;
}
