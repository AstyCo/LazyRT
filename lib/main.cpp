#include "extensions/help_functions.hpp"
#include "extensions/flatbuffers_extensions.hpp"
#include "command_line_args.hpp"
#include "types/file_tree_forest.hpp"

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

    FileTreeForest trees;
    trees.setProjectDirectory(clargs.proDir());

    PROFILE(FileTreeFunc::readDirectory(trees.srcTree, clargs.srcDir().joint(),
                                        clargs.srcIgnoreSubstrings()));
    PROFILE(FileTreeFunc::readDirectory(trees.testTree,
                                        clargs.testDir().joint(),
                                        clargs.testIgnoreSubstrings()));

    trees.installIncludeSources();
    trees.installExtraDependencies(clargs.deps()); // TODO: CHECK EXTRA_DEPS

    PROFILE(
        FileTreeFunc::parsePhase(trees.srcTree, clargs.srcsDumpIn().joint()));
    PROFILE(
        FileTreeFunc::parsePhase(trees.testTree, clargs.testsDumpIn().joint()));

    PROFILE(FileTreeFunc::analyzePhase(trees.srcTree));
    PROFILE(FileTreeFunc::analyzePhase(trees.testTree));

    if (!clargs.isNoMain())
        FileTreeFunc::labelMainAffected(trees.testTree);

    // Create directories to put output files to
    boost::filesystem::create_directories(clargs.outDir().joint());

    FileTreeFunc::writeModified(trees.srcTree, clargs.srcsModified().joint());
    FileTreeFunc::writeModified(trees.testTree, clargs.testsModified().joint());

    trees.installAffectedFiles();

    FileTreeFunc::writeAffected(trees.srcTree, clargs.srcsAffected().joint());
    FileTreeFunc::writeAffected(trees.testTree, clargs.testsAffected().joint());

    if (clargs.verbal()) {
        FileTreeFunc::printAffected(trees.srcTree);
        FileTreeFunc::printAffected(trees.testTree);
        trees.srcTree.printModified(clargs.srcBase());
        trees.testTree.printModified(clargs.testBase());

        std::cout << "write lazyut files to " << clargs.outDir().joint()
                  << std::endl;
    }

    PROFILE(
        FileTreeFunc::serialize(trees.srcTree, clargs.srcsDumpOut().joint()));
    PROFILE(
        FileTreeFunc::serialize(trees.testTree, clargs.testsDumpOut().joint()));

    return 0;
}
