
#include "extensions/help_functions.hpp"
#include "extensions/flatbuffers_extensions.hpp"

#include "directoryreader.hpp"
#include "dependency_analyzer.hpp"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers_schemes/file_tree_generated.h"

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SRCS_DIR "..\\..\\LazyUT\\src_files"
#define TESTS_DIR "..\\..\\LazyUT\\test_files"

#define SRCS_FILE_TREE_NAME "srcs_file_tree.bin"
#define TESTS_FILE_TREE_NAME "tests_file_tree.bin"

#define SRCS_AFFECTED_FILE_NAME "srcs_affected.txt"
#define TESTS_AFFECTED_FILE_NAME "tests_affected.txt"

#define TOTAL_AFFECTED_FILE_NAME "total_affected.txt"

#define PROFILE(x) x; prf.step(#x);
#define START_PROFILE PROFILE(Profiler prf;)

static void concatFiles(const char *fin1, const char *fin2, const char *fout)
{
    std::ifstream if_a(fin1, std::ios_base::binary);
    std::ifstream if_b(fin2, std::ios_base::binary);
    std::ofstream of_c(fout, std::ios_base::binary);

    of_c << if_a.rdbuf() << if_b.rdbuf();
}

int main(int /*argc*/, const char */*argv*/[])
{
    START_PROFILE

    FileTree srcsTree;
    FileTree testTree;

    PROFILE(FileTreeFunc::readDirectory(srcsTree, SRCS_DIR);)
    PROFILE(FileTreeFunc::readDirectory(testTree, TESTS_DIR);)

    testTree._includePaths.push_back(srcsTree._rootDirectoryNode);

    PROFILE(FileTreeFunc::parsePhase(srcsTree, SRCS_FILE_TREE_NAME);)
    PROFILE(FileTreeFunc::parsePhase(testTree, TESTS_FILE_TREE_NAME);)
    PROFILE(FileTreeFunc::analyzePhase(srcsTree);)
    PROFILE(FileTreeFunc::analyzePhase(testTree);)

    FileTreeFunc::printAffected(srcsTree);
    FileTreeFunc::printAffected(testTree);

    PROFILE(FileTreeFunc::writeAffected(srcsTree, SRCS_AFFECTED_FILE_NAME);)
    PROFILE(FileTreeFunc::writeAffected(testTree, TESTS_AFFECTED_FILE_NAME);)

    DEBUG(
        // make total_affected_files.txt
        concatFiles(SRCS_AFFECTED_FILE_NAME,
                    TESTS_AFFECTED_FILE_NAME,
                    TOTAL_AFFECTED_FILE_NAME);
    )

    PROFILE(FileTreeFunc::serialize(srcsTree, SRCS_FILE_TREE_NAME); )
    PROFILE(FileTreeFunc::serialize(testTree, TESTS_FILE_TREE_NAME);)

    return 0;
}
