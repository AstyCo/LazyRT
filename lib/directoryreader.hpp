#ifndef DIRECTORY_READER_HPP
#define DIRECTORY_READER_HPP

#include "types/file_tree.hpp"

#include <boost/filesystem.hpp>

#include <iostream>
#include <vector>

class DirectoryReader
{
public:
    DirectoryReader();

    void readDirectory(FileTree &fileTree, const char *directory_path);
    void readDirectory(FileTree &fileTree, const BoostPath &directory_path);
    
private:
    void readDirectoryRecursively(FileTree &fileTree, const BoostPath &directory_path, const BoostPath &dir_base);
    void removeEmptyDirectories(FileTree &fileTree);

    bool isSourceFile(const boost::filesystem::path &file_path) const;

    static const std::vector<std::string> _sourceFileExtensions;
private:
    FileNode *_currenDirectory;
};


#endif // DIRECTORY_READER_HPP
