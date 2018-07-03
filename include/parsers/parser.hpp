#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <vector>

#include <boost/filesystem.hpp>
#include "types/file_tree.hpp"

class Parser
{
public:
    Parser();

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

#endif // PARSER_HPP
