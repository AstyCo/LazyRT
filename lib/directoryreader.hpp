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

    void readDirectory(const char *directory_path);
    void readDirectory(const BoostPath &directory_path);
    
    FileTree &fileTree() { return _fileTree;}
    const FileTree &fileTree() const { return _fileTree;}

private:
    void readDirectoryRecursively(const BoostPath &directory_path, const BoostPath &dir_base);
    void removeEmptyDirectories();

    bool isSourceFile(const boost::filesystem::path &file_path) const;

    FileTree _fileTree;
    static const std::vector<std::string> _sourceFileExtensions;
private:
    FileNode *_currenDirectory;
};


#endif // DIRECTORY_READER_HPP
