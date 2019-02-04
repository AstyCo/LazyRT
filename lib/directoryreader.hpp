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

    static std::vector< std::string > _sourceFileExtensions;
    static std::vector< std::string > _ignore_substrings;

    bool exists(const SplittedPath &sp) const;

    bool readSources(FileTree &fileTree, const SplittedPath &relPath);

private:
    void readDirectoryRecursively(FileTree &fileTree, const BoostPath &path,
                                  const SplittedPath &sp_base);
    void removeEmptyDirectories(FileTree &fileTree);

    bool isSourceFile(const boost::filesystem::path &file_path) const;
    bool isIgnored(const std::string &path) const;

private:
    FileNode *_currenDirectory;
};

#endif // DIRECTORY_READER_HPP
