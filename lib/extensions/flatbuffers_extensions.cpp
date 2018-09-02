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
        auto &records = *file_tree->records();
        for (auto record : records) {
            FileNode *fnode = tree->addFile(record->path()->str());
            fnode->record().setHash(record->md5()->data());
            // TODO
        }

        std::cout << "\nFile tree '" << tree->_rootPath.joint()
                  << "' is read:" << std::endl;
        delete file_tree_dump;
    }

    return tree;
}
