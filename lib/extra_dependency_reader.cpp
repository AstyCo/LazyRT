#include "extra_dependency_reader.hpp"

#include "types/file_tree.hpp"
#include "types/splitted_string.hpp"

#include "extensions/error_reporter.hpp"

#include <sstream>
#include <array>

static std::string error_missing_argument(const std::string &path_to_json,
                                          const std::string &arg)
{
    std::stringstream ss;
    ss << "JSON file" << path_to_json << "missing key" << arg;
    return ss.str();
}

static void file_error(const std::string &fname, unsigned lineNumber,
                       unsigned charNumber, const std::string &message)
{
    errors() << "FILE:" << fname << "\nLINE:" << ntos(lineNumber)
             << "\nOFFSET:" << ntos(charNumber) << "\nERROR:" << message
             << "(line skipped)";
}

static char *end_of_line(char *p, std::size_t size)
{
    auto pEnd = static_cast< char * >(memchr(p, '\n', size));
    if (pEnd == nullptr)
        return p + size;
    return pEnd;
}

static char *read_path(char *pBegin, std::size_t size, std::string &path)
{
    char *pWord = nullptr;
    for (char *p = pBegin; p - pBegin < size; ++p) {
        char ch = *p;
        if (isspace(ch)) {
            if (pWord == nullptr)
                continue; // skip leading spaces
            path = std::string(pWord, p - pWord);
            return p;
        }
        else {
            if (pWord == nullptr)
                pWord = p; // install begin of the word
        }
    }
    if (pWord)
        path = std::string(pWord, size - (pWord - pBegin));
    return pBegin + size;
}

static bool parse_line(char *pLine, std::size_t size,
                       ExtraDependencyReader::Dependencies &deps,
                       const std::string &fname, unsigned lineNumber)
{
    std::string firstPath;
    std::string dependencyPath;
    char *p = read_path(pLine, size, firstPath);
    if (firstPath.empty())
        return true; // empty line

    p = read_path(p, size - (p - pLine), dependencyPath);
    if (dependencyPath.empty()) {
        file_error(fname, lineNumber, size - 1, "second path expected");
        return false;
    }
    for (; p - pLine < size; ++p) {
        if (!isspace(*p)) {
            file_error(fname, lineNumber, p - pLine,
                       "exactly two path expected");
            return false;
        }
    }
    ExtraDependencyReader::DependencyRecord record;
    record.filePath = SplittedPath(firstPath, SplittedPath::unixSep());
    record.dependencyPath =
        SplittedPath(dependencyPath, SplittedPath::unixSep());

    deps.push_back(record);
    return true;
}

ExtraDependencyReader::Dependencies
ExtraDependencyReader::read_extra_dependencies(
    const SplittedPath &path_to_extra_deps)
{
    std::string fname = path_to_extra_deps.jointOs();
    auto p = readFile(fname.c_str(), "r");
    char *data = p.first;
    auto size = p.second;
    if (!data)
        return Dependencies();

    Dependencies result;
    unsigned lineNumber = 0; // for nice error messages
    char *pLine = data;
    char *pWord = nullptr;
    unsigned word_number_in_string = 0;
    std::array< std::string, 2 > line;

    char *pEndOfLine = data;
    for (auto p = data; pEndOfLine - data < size;
         ++lineNumber, p = pEndOfLine + 1) {
        pEndOfLine = end_of_line(p, size - (p - data));
        parse_line(p, pEndOfLine - p, result, fname, lineNumber);
    }
    return result;
}

void ExtraDependencyReader::set_extra_dependencies(
    const SplittedPath &path_to_extra_deps, FileTree &tree)
{
    auto deps = read_extra_dependencies(path_to_extra_deps);
    for (const auto &dep : deps) {
        FileNode *file = tree.searchInRoot(dep.filePath);
        FileNode *depNode = tree.searchInRoot(dep.dependencyPath);
        if (!file) {
            errors() << "failed to set dependency, file"
                     << dep.filePath.jointOs() << "not found";
            continue;
        }
        if (!file->isRegularFile()) {
            errors() << "failed to set dependency, file"
                     << dep.filePath.jointOs() << "must be regular file";
            continue;
        }

        if (!depNode) {
            errors() << "failed to set dependency, file"
                     << dep.dependencyPath.jointOs() << "not found";
            continue;
        }
        auto fs = depNode->getFiles();
        for (FileNode *depFile : fs)
            file->addExplicitDep(depFile);
    }
}
