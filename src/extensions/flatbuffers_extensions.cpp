#include "extensions/flatbuffers_extensions.hpp"

FileTree *restoreFileTree(const char *fname)
{
    FileTree *tree = NULL;

    char *file_tree_dump = readBinaryFile(fname).first;
    if (file_tree_dump) {
        auto file_tree = UTestRunner::GetFileTree(file_tree_dump);
        tree = new FileTree();

        tree->_state = FileTree::Restored;
        tree->_rootPath = file_tree->rootPath()->str();
        std::cout << "\nFile tree '" << tree->_rootPath.string()
                  << "' is read:" << std::endl;
        auto &records = *file_tree->records();
        for (auto record : records) {
            FileNode *fnode = tree->createNode(record->path()->str());
            fnode->record().setHash(record->md5()->data());
            // TODO
        }

        delete file_tree_dump;
    }

    return tree;
}
