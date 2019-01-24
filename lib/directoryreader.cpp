#include "directoryreader.hpp"
#include "extensions/error_reporter.hpp"
#include "extensions/help_functions.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

using namespace boost::filesystem;

std::vector< std::string > initSourceFileExtensions()
{
    typedef const char CharArray[5];
    const CharArray exts_c[] = {".c", ".cpp", ".cc", ".C", ".cxx", ".c++",
                                ".h", ".hpp", ".hh", ".H", ".hxx", ".h++"};
    int size = sizeof(exts_c) / sizeof(CharArray);

    std::vector< std::string > exts;

    for (int i = 0; i < size; ++i)
        exts.push_back(exts_c[i]);

    return exts;
}

std::vector< std::string > DirectoryReader::_sourceFileExtensions =
    initSourceFileExtensions();
std::vector< std::string > DirectoryReader::_ignore_substrings =
    std::vector< std::string >();

DirectoryReader::DirectoryReader() {}

void DirectoryReader::readDirectory(FileTree &fileTree,
                                    const char *directory_path)
{
    readDirectory(fileTree, BoostPath(directory_path));
}

void DirectoryReader::readDirectory(FileTree &fileTree,
                                    const BoostPath &directory_path)
{
    SplittedPath sp_base(directory_path.string(), SplittedPath::osSep());

    fileTree.clean();
    fileTree.setRootPath(sp_base);
    _currenDirectory = nullptr;

    readDirectoryRecursively(fileTree, directory_path, sp_base);
    fileTree._state = FileTree::Filled;

    removeEmptyDirectories(fileTree);
    fileTree.calculateFileHashes();
}

void DirectoryReader::readDirectoryRecursively(FileTree &fileTree,
                                               const BoostPath &directory_path,
                                               const SplittedPath &sp_base)
{
    try {
        if (isIgnored(directory_path.string()))
            return;
        if (!exists(directory_path)) {
            errors() << directory_path.string() << "1does not exist\n";
            return;
        }

        SplittedPath spDirectoryPath(directory_path.string(),
                                     SplittedPath::osSep());
        SplittedPath rel_path = my_relative(spDirectoryPath, sp_base);
        if (is_regular_file(directory_path)) {
            if (!isSourceFile(directory_path))
                return;
            MY_ASSERT(_currenDirectory);
            _currenDirectory->addChild(
                new FileNode(rel_path, FileRecord::RegularFile,
                             fileTree));
        }
        else if (is_directory(directory_path)) {
            FileNode *directoryNode =
                new FileNode(rel_path, FileRecord::Directory,
                             fileTree);
            if (_currenDirectory == nullptr)
                fileTree.setRootDirectoryNode(directoryNode);
            else
                _currenDirectory->addChild(directoryNode);

            directory_iterator it(directory_path);
            directory_iterator end;
            while (it != end) {
                _currenDirectory = directoryNode;
                readDirectoryRecursively(fileTree, *it, sp_base);
                ++it;
            }
        }

        else {
            errors()
                << directory_path.string()
                << "exists, but is neither a regular file nor a directory\n";
        }
    }
    catch (const boost::filesystem::filesystem_error &exc) {
        errors() << "Exception:" << exc.what();
        return;
    }
}

void DirectoryReader::removeEmptyDirectories(FileTree &fileTree)
{
    fileTree.removeEmptyDirectories();
}

bool DirectoryReader::isSourceFile(const path &file_path) const
{
    MY_ASSERT(boost::filesystem::is_regular_file(file_path));
    return std::find(_sourceFileExtensions.begin(), _sourceFileExtensions.end(),
                     boost::filesystem::extension(file_path).c_str()) !=
           _sourceFileExtensions.end();
}

bool DirectoryReader::isIgnored(const std::__cxx11::string &path) const
{
    // check if path is ignored
    for (auto &ignore_substring : _ignore_substrings) {
        if (path.find(ignore_substring) != std::string::npos)
            return true;
    }
    // not found
    return false;
}
