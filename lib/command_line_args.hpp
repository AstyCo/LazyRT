#ifndef COMMAND_LINE_ARGS_HPP
#define COMMAND_LINE_ARGS_HPP

#include "types/splitted_string.hpp"

class CommandLineArgs
{
public:
    enum State { Success = 0, Failure = 1 };

public:
    CommandLineArgs();

    void parseArguments(int argc, char *argv[]);

    const SplittedPath &rootDirectory() const { return _rootDirectory; }

    std::vector< SplittedPath > srcDirectories() const;
    std::vector< SplittedPath > testDirectories() const;

    const SplittedPath &outDir() const { return _outDirectory; }
    const SplittedPath &inDir() const { return _inDirectory; }
    const std::string &deps() const { return _extraDependencies; }

    bool verbal() const { return _verbal; }
    bool isNoMain() const { return _isNoMain; }

    const SplittedPath &ftreeDumpIn() const { return _ftreeDumpIn; }
    const SplittedPath &ftreeDumpOut() const { return _ftreeDumpOut; }
    const SplittedPath &srcsAffected() const { return _srcsAffected; }
    const SplittedPath &testsAffected() const { return _testsAffected; }
    const SplittedPath &totalAffected() const { return _totalAffected; }
    const SplittedPath &srcsModified() const { return _srcsModified; }
    const SplittedPath &testsModified() const { return _testsModified; }

    int status() const { return _status; }
    int retCode() const { return _retCode; }

    std::vector< std::string > ignoredSubstrings() const;
    std::vector< SplittedPath > includePaths() const;

    const SplittedPath &srcBase() const { return _srcBase; }
    const SplittedPath &testBase() const { return _testBase; }

    static std::vector< SplittedPath > splittedPaths(const std::string &str);

private:
    SplittedPath _rootDirectory;
    std::string _srcDirectories;
    std::string _testDirectories;
    SplittedPath _outDirectory;
    SplittedPath _inDirectory;
    std::string _extraDependencies;

    std::string _exts;

    std::string _ignoredSubstrings;
    std::string _includePaths;

    SplittedPath _srcBase;
    SplittedPath _testBase;

    bool _verbal;
    bool _isNoMain;

    static std::string _rootFTreeFilename;
    static std::string _srcsAffectedFileName;
    static std::string _testsAffectedFileName;
    static std::string _totalAffectedFileName;

    // derived data
    SplittedPath _ftreeDumpIn;
    SplittedPath _ftreeDumpOut;
    SplittedPath _srcsAffected;
    SplittedPath _testsAffected;
    SplittedPath _totalAffected;
    SplittedPath _srcsModified;
    SplittedPath _testsModified;

    int _status;
    int _retCode;
};

extern CommandLineArgs clargs;

#endif // COMMAND_LINE_ARGS_HPP
