#ifndef DIRECTORY_PARSER_HPP
#define DIRECTORY_PARSER_HPP

#include "types/file_tree.hpp"

#include <boost/filesystem.hpp>

#include <iostream>
#include <vector>

class DirectoryParser
{
public:
    DirectoryParser();

    void parseDirectory(const char *directory_path);
    void parseDirectory(const BoostPath &directory_path);
    
    FileTree &fileTree() { return _fileTree;}
    const FileTree &fileTree() const { return _fileTree;}

private:
    void parseDirectoryRecursively(const BoostPath &directory_path, const BoostPath &dir_base);
    void removeEmptyDirectories();

    bool isSourceFile(const boost::filesystem::path &file_path) const;

    FileTree _fileTree;
    static const std::vector<std::string> _sourceFileExtensions;
private:
    FileNode *_currenDirectory;
};


#endif // DIRECTORY_PARSER_HPP
