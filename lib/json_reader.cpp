#include "json_reader.hpp"
#include "types/file_tree.hpp"

#include "extensions/error_reporter.hpp"

#include "external/rapidjson/document.h"
#include "external/rapidjson/error/en.h"

#include <sstream>

#define STR_ROOT_DIRECTORY "root_path"

#define CHECK_STRING(x)                                                        \
    if (!(x).IsString()) {                                                     \
        error_wrong_type(path_to_json, );

using namespace rapidjson;

static const char *kTypeNames[] = {"Null",  "False",  "True",  "Object",
                                   "Array", "String", "Number"};

static std::string error_missing_argument(const std::string &path_to_json,
                                          const std::string &arg)
{
    std::stringstream ss;
    ss << "JSON file" << path_to_json << "missing key" << arg;
    return ss.str();
}

template < typename TValue >
static std::string get_string_json(const TValue &value, const char *key = NULL)
{
    if (!value.IsString()) {
        if (key) {
            char buff[1000];
            snprintf(buff, sizeof(buff),
                     "JSON file Error: string value expected for key \"%s\"",
                     key);
        }
        return std::string();
    }
    return std::string(value.GetString());
}

void JsonReader::read_extra_dependencies(const std::string &path_to_json,
                                         FileTree &tree)
{
    try {
        auto data_pair = readFile(path_to_json.c_str(), "r");
        char *data = data_pair.first;
        if (!data) {
            errors() << "Failed to open the file" << '\'' + path_to_json + '\'';
            return;
        }
        Document document;
        document.Parse(data);

        if (document.HasParseError()) {
            char buff[1000];
            snprintf(buff, sizeof(buff),
                     "Error parsing JSON file %s\n(offset %u): %s\n",
                     path_to_json.c_str(), (unsigned)document.GetErrorOffset(),
                     GetParseError_En(document.GetParseError()));

            errors() << buff;
            //        return;
        }

        if (!document.HasMember(STR_ROOT_DIRECTORY))
            throw error_missing_argument(path_to_json, STR_ROOT_DIRECTORY);

        std::string sRootDir =
            get_string_json(document[STR_ROOT_DIRECTORY], STR_ROOT_DIRECTORY);
        SplittedPath spRootDir(sRootDir, SplittedPath::unixSep());
        spRootDir.setOsSeparator();
        SplittedPath tmp = tree._projectDirectory + spRootDir;
        std::cout << STR_ROOT_DIRECTORY << " " << tmp.joint() << std::endl;
        tree.setRootPath(tmp);
        for (Value::ConstMemberIterator it_members = document.MemberBegin();
             it_members != document.MemberEnd(); ++it_members) {
            if (!strcmp(it_members->name.GetString(), STR_ROOT_DIRECTORY))
                continue;

            const auto &v = it_members->value;
            if (v.IsString()) {
                SplittedPath fp(v.GetString(), SplittedPath::unixSep());
                fp.setOsSeparator();
                SplittedPath tmpFullPath = tree.rootPath() + fp;
                FileNode *file = tree.addFile(fp);
                printf("%s depends on %s\n", it_members->name.GetString(),
                       tmpFullPath.joint().c_str());
            }
            else if (v.IsArray()) {
                for (Value::ConstValueIterator it_array = v.Begin();
                     it_array != v.End(); ++it_array) {
                    if (it_array->IsString()) {
                        SplittedPath fp(it_array->GetString(),
                                        SplittedPath::unixSep());
                        fp.setOsSeparator();
                        FileNode *file = tree.addFile(fp);
                        SplittedPath tmpFullPath = tree.rootPath() + fp;
                        printf("%s depends on %s\n",
                               it_members->name.GetString(),
                               tmpFullPath.joint().c_str());
                    }
                    else {
                        errors()
                            << "Warning: JSON file wrong type:"
                            << "SKIPPED key:" << it_members->name.GetString()
                            << "type:" << kTypeNames[it_array->GetType()]
                            << "(array item)\n";
                    }
                }
            }
            else {
                errors() << "Warning: JSON file wrong type:"
                         << "SKIPPED key:" << it_members->name.GetString()
                         << "type:" << kTypeNames[v.GetType()] << "\n";
            }
        }
    }
    catch (const std::string &msg) {
        errors() << msg;
        tree.setState(FileTree::Error);
        return;
    }

    tree.setState(FileTree::Filled);
}
