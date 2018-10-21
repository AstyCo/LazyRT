#ifndef JSON_READER_HPP
#define JSON_READER_HPP

#include <string>

class FileTree;

class JsonReader
{
public:
    void read_extra_dependencies(const std::string &path_to_json,
                                 FileTree &tree);
};

#endif // JSON_READER_HPP
