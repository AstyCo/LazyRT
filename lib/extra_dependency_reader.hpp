#ifndef EXTRA_DEPENDENCY_HPP
#define EXTRA_DEPENDENCY_HPP

#include "types/splitted_string.hpp"

#include <vector>
#include <string>

class FileTree;

class ExtraDependencyReader
{
public:
    struct DependencyRecord
    {
        SplittedPath filePath;
        SplittedPath dependencyPath;
    };
    using Dependencies = std::vector< DependencyRecord >;

public:
    Dependencies
    read_extra_dependencies(const SplittedPath &path_to_extra_deps);
    void set_extra_dependencies(const SplittedPath &path_to_extra_deps,
                                FileTree &tree);
};

#endif // EXTRA_DEPENDENCY_HPP
