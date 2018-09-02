
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

#define TEST_FNAME "file_tree_dump.bin"

void pushFiles(flatbuffers::FlatBufferBuilder &builder,
               std::vector<flatbuffers::Offset<UTestRunner::FileRecord> > &records, const FileNode *node)
{
    if (node->isRegularFile()) {
        auto frecord = node->record();
        auto fbs_frecord = UTestRunner::CreateFileRecord(
                    builder,
                    builder.CreateString(frecord._path.joint()),
                    builder.CreateVector(frecord._hashArray, 16));
        records.push_back(fbs_frecord);
    }

    const FileNode::ListFileNode &childs = node->childs();
    FileNode::FileNodeConstIterator it(childs.cbegin());
    while (it != childs.cend()) {
        pushFiles(builder, records, *it);

        ++it;
    }
}



int main(int /*argc*/, const char */*argv*/[])
{
    DirectoryReader dirReader;
    Profiler prf;
    std::string srcDirName("D:\\Study\\LazyUT\\test_files");
    dirReader.readDirectory(srcDirName);
    prf.step("parsed directory " + srcDirName);

//    DirectoryParser testsParser;
//    std::string testDirName("C:\\experiments\\test");
//    FileTree &testTree = testsParser.fileTree();
//    testTree._includePaths.push_back(parser.fileTree()._rootDirectoryNode);
//    testsParser.parseDirectory(testDirName);
//    testTree.parseFiles();
//    testTree.installIncludeNodes();
//    testTree.print();

    FileTree &tree = dirReader.fileTree();
    tree.parseFiles();

    DependencyAnalyzer dep;
    dep.setRoot(tree._rootDirectoryNode);
    dep.print();
    tree.print();

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
                                                         builder.CreateString(tree._rootPath.joint()),
                                                         builder.CreateVector(fileRecords));

        builder.Finish(fbs_file_tree);
        uint8_t *data = builder.GetBufferPointer();
        MY_ASSERT(data != nullptr);
        writeBinaryFile(TEST_FNAME, data, builder.GetSize());
    }
    prf.step("stored binary " + srcDirName);

    return 0;
}
