#include "extensions/help_functions.hpp"
#include "extensions/flatbuffers_extensions.hpp"
#include "types/file_tree_forest.hpp"
#include "directoryreader.hpp"

#include "external/CLI11/CLI11.hpp"

#include <boost/filesystem.hpp>

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SRCS_FILE_TREE_NAME "srcs_file_tree.bin"
#define TESTS_FILE_TREE_NAME "tests_file_tree.bin"

#define SRCS_AFFECTED_FILE_NAME "srcs_affected.txt"
#define TESTS_AFFECTED_FILE_NAME "tests_affected.txt"

#define TOTAL_AFFECTED_FILE_NAME "total_affected.txt"

#define PROFILE(x) x; prf.step(#x)
#define START_PROFILE Profiler prf(verbal);

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
    CLI::App app{"LazyUT detects tests, affected by the changes in the code.\n"};

    std::string proDirectory;
    std::string srcDirectory;
    std::string testDirectory;
    std::string outDirectory;
    std::string inDirectory;

    std::string exts;
    std::string ignore_substrings;
    std::string extra_dependencies;

    bool verbal = false;
    bool keep_test_main = true;

    // required arguments
    app.add_option("-p,--prodir", proDirectory, "Project root directory");
    app.add_option("-s,--srcdir", srcDirectory, "Directory with library source files")->required();
    app.add_option("-t,--testdir", testDirectory, "Directory with tests source files")->required();
    app.add_option("-o,--outdir", outDirectory, "Output directory")->required();
    // optional arguments
    app.add_option("-d,--deps", extra_dependencies, "Path to the JSON file with extra dependencies");
    app.add_option("-i,--indir", inDirectory, "Input directory");
    app.add_option("-e,--extensions", exts, "Source files extensions, separated by comma (,)");
    app.add_option("--ignore", ignore_substrings, "Substrings of the ignored paths, separated by comma (,)");
    app.add_flag("-m,--main", keep_test_main, "Allways keep test source file with main() implementation");
    app.add_flag("-v,--verbal", verbal, "Verbal mode");
    //

    CLI11_PARSE(app, argc, argv);

    if (!exts.empty())
        DirectoryReader::_sourceFileExtensions = split(exts, ",");
    if (!ignore_substrings.empty())
        DirectoryReader::_ignore_substrings = split(ignore_substrings, ",");

    START_PROFILE;

    SplittedPath outDirectorySP(outDirectory,
                                SplittedPath::unixSep());

    SplittedPath inDirectorySP(inDirectory,
            SplittedPath::unixSep());

    if (inDirectory.empty())
        inDirectorySP = outDirectorySP;

    SplittedPath srcsDumpInSP = inDirectorySP;
    srcsDumpInSP.append(std::string(SRCS_FILE_TREE_NAME));

    SplittedPath testsDumpInSP = inDirectorySP;
    testsDumpInSP.append(std::string(TESTS_FILE_TREE_NAME));

    SplittedPath srcsDumpOutSP = outDirectorySP;
    srcsDumpOutSP.append(std::string(SRCS_FILE_TREE_NAME));

    SplittedPath testsDumpOutSP = outDirectorySP;
    testsDumpOutSP.append(std::string(TESTS_FILE_TREE_NAME));

    SplittedPath srcsAffectedSP = outDirectorySP;
    srcsAffectedSP.append(std::string(SRCS_AFFECTED_FILE_NAME));

    SplittedPath testsAffectedSP = outDirectorySP;
    testsAffectedSP.append(std::string(TESTS_AFFECTED_FILE_NAME));

    SplittedPath totalAffectedSP = outDirectorySP;
    totalAffectedSP.append(std::string(TOTAL_AFFECTED_FILE_NAME));

    SplittedPath spProDirectory(proDirectory,
                                SplittedPath::unixSep());

    SplittedPath spSrcDirectory = absolute_path(SplittedPath(srcDirectory,
                                                             SplittedPath::unixSep()),
                                                spProDirectory);
    SplittedPath spTestDirectory = absolute_path(SplittedPath(testDirectory,
                                                              SplittedPath::unixSep()),
                                                 spProDirectory);

    FileTreeForest trees;
    trees.setProjectDirectory(spProDirectory);

    PROFILE(FileTreeFunc::readDirectory(trees.srcTree, spSrcDirectory.joint()));
    PROFILE(FileTreeFunc::readDirectory(trees.testTree, spTestDirectory.joint()));

    trees.installIncludeSources();
    trees.installExtraDependencies(extra_dependencies);

    PROFILE(FileTreeFunc::parsePhase(trees.srcTree, srcsDumpInSP.joint()));
    PROFILE(FileTreeFunc::parsePhase(trees.testTree, testsDumpInSP.joint()));

    PROFILE(FileTreeFunc::analyzePhase(trees.srcTree));
    PROFILE(FileTreeFunc::analyzePhase(trees.testTree));

    if (keep_test_main)
        FileTreeFunc::labelMainAffected(trees.testTree);

//    return 0;
    boost::filesystem::create_directories(outDirectorySP.joint());

    SplittedPath srcModifiedSP = outDirectorySP;
    srcModifiedSP.append(std::string("src_modified.txt"));
    SplittedPath testModifiedSP = outDirectorySP;
    testModifiedSP.append(std::string("test_modified.txt"));

    FileTreeFunc::writeModified(trees.srcTree, srcModifiedSP.joint());
    FileTreeFunc::writeModified(trees.testTree, testModifiedSP.joint());

    trees.installAffectedFiles();

    FileTreeFunc::writeAffected(trees.srcTree, srcsAffectedSP.joint());
    FileTreeFunc::writeAffected(trees.testTree, testsAffectedSP.joint());

    if (verbal) {
        FileTreeFunc::printAffected(trees.srcTree);
        FileTreeFunc::printAffected(trees.testTree);
        trees.srcTree.printModified();
        trees.testTree.printModified();

        std::cout << "write lazyut files to "
                  << outDirectorySP.joint() << std::endl;
    }

    PROFILE(FileTreeFunc::serialize(trees.srcTree, srcsDumpOutSP.joint()));
    PROFILE(FileTreeFunc::serialize(trees.testTree, testsDumpOutSP.joint()));

    return 0;
}
