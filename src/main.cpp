#include <iostream>
#include "parsers/parser.hpp"
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
    Parser parser;
    double cpu0 = getCpuTime();

    parser.parseDirectory("D:\\Study\\smart-sort");
//    parser.parseDirectory("D:\\Jobs\\pro");
    double cpu1 = getCpuTime();

    FileTree &tree = parser.fileTree();
    tree.print();

    if (FileTree *restoredTree = restoreFileTree(TEST_FNAME)) {
        restoredTree->print();
        tree.parseModifiedFiles(restoredTree);

        delete restoredTree;
    }


    // Create a `FlatBufferBuilder`, which will be used to create our
    // monsters' FlatBuffers.
    {
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


    std::cout << "\nCPU time: " << cpu1 - cpu0 << std::endl;

//    int x;
//    std::cin >> x;
    return 0;
}
