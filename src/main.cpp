#include <iostream>
#include "directoryparser.hpp"
#include "extensions/help_functions.hpp"

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers_schemes/file_tree_generated.h"
#include "extensions/flatbuffers_extensions.hpp"

#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pushFiles(flatbuffers::FlatBufferBuilder &builder,
               std::vector<flatbuffers::Offset<UTestRunner::FileRecord> > &records, const FileNode *node)
{
    if (node->isRegularFile()) {
        auto frecord = node->record();
        auto fbs_frecord = UTestRunner::CreateFileRecord(builder,
                                                     builder.CreateString(frecord._path.string()),
                                                     builder.CreateVector(frecord._hashArray, 16));
        records.push_back(fbs_frecord);
    }

    const FileNode::child_list &childs = node->childs();
    FileNode::child_const_iterator it(childs.cbegin());
    while (it != childs.cend()) {
        pushFiles(builder, records, *it);

        ++it;
    }
}

#define TEST_FNAME "file_tree_dump.bin"


int main(int /*argc*/, const char */*argv*/[])
{
    DirectoryParser parser;
    Profiler prf;
    std::string srcDirName("D:\\Study\\TestCoveredProjects\\cppcheck\\lib");
    parser.parseDirectory(srcDirName);
    prf.step("parsed directory " + srcDirName);

    DirectoryParser testsParser;
    std::string testDirName("D:\\Study\\TestCoveredProjects\\cppcheck\\test");
    FileTree &testTree = testsParser.fileTree();
    testTree._includePaths.push_back(parser.fileTree()._rootDirectoryNode);
    testsParser.parseDirectory(testDirName);
    testTree.parseFiles();
    testTree.print();

    FileTree &tree = parser.fileTree();
//    tree.print();

//    if (FileTree *restoredTree = restoreFileTree(TEST_FNAME)) {
//        restoredTree->print();
//        tree.parseModifiedFiles(restoredTree);

//        delete restoredTree;
//    }
//    else {
//        tree.parseFiles();
//        tree.print();
//    }
    prf.step("parsed (modified) files " + srcDirName);

    {
        // store to binary file
        flatbuffers::FlatBufferBuilder builder(1024);

        std::vector<flatbuffers::Offset<UTestRunner::FileRecord> > fileRecords;
        pushFiles(builder, fileRecords, tree._rootDirectoryNode);


        auto fbs_file_tree = UTestRunner::CreateFileTree(builder,
                                                         builder.CreateString(tree._rootPath.string()),
                                                         builder.CreateVector(fileRecords));

        builder.Finish(fbs_file_tree);
        uint8_t *data = builder.GetBufferPointer();
        MY_ASSERT(data != NULL);
        writeBinaryFile(TEST_FNAME, data, builder.GetSize());
    }
    prf.step("stored binary " + srcDirName);

    return 0;
}
