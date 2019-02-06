#ifndef DIRECTORY_READER_HPP
#define DIRECTORY_READER_HPP

#include "types/file_tree.hpp"

#include <boost/filesystem.hpp>

#include <iostream>
#include <vector>

class DirectoryReader
{
public:
    static std::vector< std::string > _sourceFileExtensions;
    static std::vector< std::string > _ignore_substrings;

    bool exists(const SplittedPath &sp) const;

    void readSources(const SplittedPath &relPath, FileNode *parent);
    void readSources(const SplittedPath &relPath, FileTree &filetree);

private:
    void removeEmptyDirectories(FileTree &fileTree);

    bool isSourceFile(const SplittedPath &sp) const; /// TODO use
    bool isIgnored(const SplittedPath &sp) const;
    bool isIgnoredOsSep(const std::string &path) const;

    FileRecord::Type getFileType(const SplittedPath &sp) const;
};

#endif // DIRECTORY_READER_HPP
