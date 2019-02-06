#include "directoryreader.hpp"
#include "extensions/error_reporter.hpp"
#include "extensions/help_functions.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

std::vector< std::string > initSourceFileExtensions()
{
    using CharArray = const char[5];
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

bool DirectoryReader::exists(const SplittedPath &sp) const
{
    return boost::filesystem::exists(sp.jointOs());
}

void DirectoryReader::readSources(const SplittedPath &relPath,
                                  FileTree &filetree)
{
    assert(filetree.rootNode());
    readSources(relPath, filetree.rootNode());
}

void DirectoryReader::readSources(const SplittedPath &relPath, FileNode *parent)
{
    SplittedPath fullpath = parent->fullPath() + relPath;
    if (isIgnored(fullpath))
        return;
    if (!exists(fullpath)) {
        errors() << "warning: file " << fullpath.joint() << " doesn't exists";
        return;
    }
    const auto &splitted = relPath.splitted();
    if (splitted.empty()) {
        boost::filesystem::path dirPath = parent->fullPath().jointOs();
        if (!boost::filesystem::is_directory(dirPath))
            return;
        boost::filesystem::directory_iterator it(dirPath);
        boost::filesystem::directory_iterator end;
        while (it != end) {
            std::string filePath = (*it).path().string();
            SplittedPath spInnerFile(filePath, SplittedPath::osSep());
            SplittedPath spRelative(spInnerFile.last(),
                                    SplittedPath::unixSep());

            readSources(spRelative, parent);
            ++it;
        }
        return;
    }
    FileNode *child =
        parent->findOrNewChild(splitted.front(), getFileType(fullpath));
    assert(child);
    if (child->isRegularFile() && isSourceFile(fullpath))
        child->setSourceFile();

    SplittedPath nextRelPath = relPath;
    nextRelPath.removeFirst();
    readSources(nextRelPath, child);
}

void DirectoryReader::removeEmptyDirectories(FileTree &fileTree)
{
    fileTree.removeEmptyDirectories();
}

bool DirectoryReader::isSourceFile(const SplittedPath &sp) const
{
    assert(boost::filesystem::is_regular_file(sp.jointOs()));
    return std::find(_sourceFileExtensions.begin(), _sourceFileExtensions.end(),
                     boost::filesystem::extension(sp.jointOs()).c_str()) !=
           _sourceFileExtensions.end();
}

bool DirectoryReader::isIgnored(const SplittedPath &sp) const
{
    auto pathUnixSep = sp.jointUnix();

    for (auto &ignore_substring : _ignore_substrings) {
        if (pathUnixSep.find(ignore_substring) != std::string::npos)
            return true;
    }
    // not found
    return false;
}

bool DirectoryReader::isIgnoredOsSep(const std::string &path) const
{
    // check if path is ignored
    return isIgnored(SplittedPath(path, SplittedPath::osSep()));
}

FileRecord::Type DirectoryReader::getFileType(const SplittedPath &sp) const
{
    if (boost::filesystem::is_directory(sp.jointOs()))
        return FileRecord::Directory;
    return FileRecord::RegularFile;
}
