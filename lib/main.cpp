#include "extensions/help_functions.hpp"
#include "extensions/flatbuffers_extensions.hpp"
#include "types/file_tree.hpp"
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

    std::string srcDirectory;
    std::string testDirectory;
    std::string outDirectory;
    std::string inDirectory;

    std::string exts;
    std::string ignore_substrings;

    bool verbal = false;
    bool keep_test_main = true;

    // required arguments
    app.add_option("-s,--srcdir", srcDirectory, "Directory with library source files")->required();
    app.add_option("-t,--testdir", testDirectory, "Directory with tests source files")->required();
    app.add_option("-o,--outdir", outDirectory, "Output directory")->required();
    // optional arguments
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

//    std::cout << "ignore str " << ignore_substrings << std::endl;
//    for (auto &i: DirectoryReader::_ignore_substrings)
//        std::cout << "ignore " << i << std::endl;
    START_PROFILE;

    SplittedPath outDirectorySP = outDirectory;
    outDirectorySP.setOsSeparator();

    SplittedPath inDirectorySP = inDirectory;
    inDirectorySP.setOsSeparator();
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

    FileTree srcsTree;
    FileTree testTree;

    PROFILE(FileTreeFunc::readDirectory(srcsTree, srcDirectory));
    PROFILE(FileTreeFunc::readDirectory(testTree, testDirectory));


    testTree._includePaths.push_back(srcsTree._rootDirectoryNode);

    PROFILE(FileTreeFunc::parsePhase(srcsTree, srcsDumpInSP.joint()));
    PROFILE(FileTreeFunc::parsePhase(testTree, testsDumpInSP.joint()));


    PROFILE(FileTreeFunc::analyzePhase(srcsTree));
    PROFILE(FileTreeFunc::analyzePhase(testTree));

    PROFILE(
    if (keep_test_main)
        FileTreeFunc::labelMainAffected(testTree);
    );
//    testTree.print();

    if (verbal) {
        FileTreeFunc::printAffected(srcsTree);
        FileTreeFunc::printAffected(testTree);
        srcsTree.printModified();
        testTree.printModified();

        std::cout << "write lazyut files to "
                  << outDirectorySP.joint() << std::endl;
    }

    boost::filesystem::create_directories(outDirectorySP.joint());

    SplittedPath srcModifiedSP = outDirectorySP;
    srcModifiedSP.append(std::string("src_modified.txt"));
    SplittedPath testModifiedSP = outDirectorySP;
    testModifiedSP.append(std::string("test_modified.txt"));

    PROFILE(FileTreeFunc::writeModified(srcsTree, srcModifiedSP.joint()));
    PROFILE(FileTreeFunc::writeModified(testTree, testModifiedSP.joint()));

    PROFILE(FileTreeFunc::writeAffected(srcsTree, srcsAffectedSP.joint()));
    PROFILE(FileTreeFunc::writeAffected(testTree, testsAffectedSP.joint()));

    DEBUG(
        // make total_affected_files.txt
        concatFiles(srcsAffectedSP.joint(),
                    testsAffectedSP.joint(),
                    totalAffectedSP.joint());
    );

    PROFILE(FileTreeFunc::serialize(srcsTree, srcsDumpOutSP.joint()));
    PROFILE(FileTreeFunc::serialize(testTree, testsDumpOutSP.joint()));

    return 0;
}
