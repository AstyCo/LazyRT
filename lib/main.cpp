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

#define PROFILE(x) x; prf.step(#x)
#define START_PROFILE Profiler prf(cla.verbal());

enum LazyUTErrors
{
    WRONG_CMD_LINE_OPTIONS = 1
};

static void concatFiles(const std::string &fin1, const std::string &fin2, const std::string &fout)
{
    std::ifstream if_a(fin1, std::ios_base::binary);
    std::ifstream if_b(fin2, std::ios_base::binary);
    std::ofstream of_c(fout, std::ios_base::binary);

    of_c << if_a.rdbuf() << if_b.rdbuf();
}

int main(int argc, char *argv[])
{
    CommandLineArgs cla(argc, argv);
    if (cla.status() != CommandLineArgs::Success)
        return cla.retCode();

    START_PROFILE;

    FileTreeForest trees;
    trees.setProjectDirectory(cla.proDir());

    PROFILE(FileTreeFunc::readDirectory(trees.srcTree, cla.srcDir().joint()));
    PROFILE(FileTreeFunc::readDirectory(trees.testTree, cla.testDir().joint()));

    trees.installIncludeSources();
    trees.installExtraDependencies(cla.deps());

    PROFILE(FileTreeFunc::parsePhase(trees.srcTree, cla.srcsDumpIn().joint()));
    PROFILE(FileTreeFunc::parsePhase(trees.testTree, cla.testsDumpIn().joint()));

    PROFILE(FileTreeFunc::analyzePhase(trees.srcTree));
    PROFILE(FileTreeFunc::analyzePhase(trees.testTree));

    if (cla.keepTestMain())
        FileTreeFunc::labelMainAffected(trees.testTree);

    boost::filesystem::create_directories(cla.outDir().joint());

    FileTreeFunc::writeModified(trees.srcTree, cla.srcsModified().joint());
    FileTreeFunc::writeModified(trees.testTree, cla.testsModified().joint());

    trees.installAffectedFiles();

    FileTreeFunc::writeAffected(trees.srcTree, cla.srcsAffected().joint());
    FileTreeFunc::writeAffected(trees.testTree, cla.testsAffected().joint());

    if (cla.verbal()) {
        FileTreeFunc::printAffected(trees.srcTree);
        FileTreeFunc::printAffected(trees.testTree);
        trees.srcTree.printModified();
        trees.testTree.printModified();

        std::cout << "write lazyut files to "
                  << cla.outDir().joint() << std::endl;
    }

    PROFILE(FileTreeFunc::serialize(trees.srcTree, cla.srcsDumpOut().joint()));
    PROFILE(FileTreeFunc::serialize(trees.testTree, cla.testsDumpOut().joint()));

    return 0;
}
