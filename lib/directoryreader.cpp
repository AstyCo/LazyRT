#include "directoryreader.hpp"
#include "extensions/error_reporter.hpp"
#include "extensions/help_functions.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


using namespace boost::filesystem;

std::vector<std::string> initSourceFileExtensions()
{
    typedef const char CharArray[5];
    const CharArray exts_c[] = { ".c", ".cpp", ".h", ".hpp" };
    int size = sizeof(exts_c) / sizeof(CharArray);

    std::vector<std::string> exts;

    for (int i = 0; i < size; ++i)
        exts.push_back(exts_c[i]);

    return exts;
}

const std::vector<std::string> DirectoryReader::_sourceFileExtensions = initSourceFileExtensions();

DirectoryReader::DirectoryReader()
{
}

void DirectoryReader::readDirectory(const char *directory_path)
{
    readDirectory(BoostPath(directory_path));
}

void DirectoryReader::readDirectory(const BoostPath &directory_path)
{
    _fileTree.clean();
    _fileTree._rootPath = directory_path.string();
    _currenDirectory = nullptr;

    readDirectoryRecursively(directory_path, directory_path);
    _fileTree._state = FileTree::Filled;

    removeEmptyDirectories();
    _fileTree.calculateFileHashes();

}

void DirectoryReader::readDirectoryRecursively(const BoostPath &directory_path, const BoostPath &dir_base)
{
    try {
        if (!exists(directory_path)) {
            errors() << directory_path.string() << "does not exist\n";
            return;
        }

        auto rel_path = boost::filesystem::relative(directory_path, dir_base);
        if (is_regular_file(directory_path)) {
            if (!isSourceFile(directory_path))
                return;
            MY_ASSERT(_currenDirectory);
            _currenDirectory->addChild(new FileNode(rel_path.string(), FileRecord::RegularFile));
//            parseFile(directory_path.string().c_str());
        }
        else if (is_directory(directory_path)) {
            FileNode *directoryNode = new FileNode(rel_path.string(), FileRecord::Directory);
            if (_currenDirectory == nullptr)
                _fileTree.setRootDirectoryNode(directoryNode);
            else
                _currenDirectory->addChild(directoryNode);

            directory_iterator it(directory_path);
            directory_iterator end;
            while (it != end) {
                _currenDirectory = directoryNode;
                readDirectoryRecursively(*it, dir_base);
                ++it;
            }
        }

        else {
            errors() << directory_path.string() << "exists, but is neither a regular file nor a directory\n";
        }
    }
    catch (const boost::filesystem::filesystem_error &exc) {
        errors() << "Exception:" << exc.what();
        return;
    }
}

void DirectoryReader::removeEmptyDirectories()
{
    _fileTree.removeEmptyDirectories();
}

bool DirectoryReader::isSourceFile(const path &file_path) const
{
    MY_ASSERT(boost::filesystem::is_regular_file(file_path));
    return std::find(_sourceFileExtensions.begin(),
                     _sourceFileExtensions.end(),
                     boost::filesystem::extension(file_path).c_str())
            != _sourceFileExtensions.end();
}


