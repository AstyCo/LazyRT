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

    const SplittedPath &proDir() const { return _proDirectory; }
    const SplittedPath &srcDir() const { return _srcDirectory; }
    const SplittedPath &testDir() const { return _testDirectory; }
    const SplittedPath &outDir() const { return _outDirectory; }
    const SplittedPath &inDir() const { return _inDirectory; }
    const std::string &deps() const { return _extra_dependencies; }

    bool verbal() const { return _verbal; }
    bool isNoMain() const { return _isNoMain; }

    const SplittedPath &srcsDumpIn() const { return _srcsDumpIn; }
    const SplittedPath &srcsDumpOut() const { return _srcsDumpOut; }
    const SplittedPath &testsDumpIn() const { return _testsDumpIn; }
    const SplittedPath &testsDumpOut() const { return _testsDumpOut; }
    const SplittedPath &srcsAffected() const { return _srcsAffected; }
    const SplittedPath &testsAffected() const { return _testsAffected; }
    const SplittedPath &totalAffected() const { return _totalAffected; }
    const SplittedPath &srcsModified() const { return _srcsModified; }
    const SplittedPath &testsModified() const { return _testsModified; }

    int status() const { return _status; }
    int retCode() const { return _retCode; }
private:
    SplittedPath _proDirectory;
    SplittedPath _srcDirectory;
    SplittedPath _testDirectory;
    SplittedPath _outDirectory;
    SplittedPath _inDirectory;
    std::string _extra_dependencies;

    std::string _exts;
    std::string _ignore_substrings;
    bool _verbal;
    bool _isNoMain;

    static std::string _srcsFileTreeName;
    static std::string _testsFileTreeName;
    static std::string _srcsAffectedFileName;
    static std::string _testsAffectedFileName;
    static std::string _totalAffectedFileName;
    static std::string _srcsModifiedFileName;
    static std::string _testsModifiedFileName;

    // derived data
    SplittedPath _srcsDumpIn;
    SplittedPath _srcsDumpOut;
    SplittedPath _testsDumpIn;
    SplittedPath _testsDumpOut;
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
