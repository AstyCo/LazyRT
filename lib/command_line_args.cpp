#include "command_line_args.hpp"
#include "directoryreader.hpp"

#include "external/CLI11/CLI11.hpp"

std::string CommandLineArgs::_srcsFileTreeName = "srcs_file_tree.bin";
std::string CommandLineArgs::_testsFileTreeName = "tests_file_tree.bin";
std::string CommandLineArgs::_srcsAffectedFileName = "srcs_affected.txt";
std::string CommandLineArgs::_testsAffectedFileName = "tests_affected.txt";
std::string CommandLineArgs::_totalAffectedFileName = "total_affected.txt";
std::string CommandLineArgs::_srcsModifiedFileName = "src_modified.txt";
std::string CommandLineArgs::_testsModifiedFileName = "test_modified.txt";

CommandLineArgs::CommandLineArgs(int argc, char *argv[])
    : _verbal(false), _keepTestMain(false), _retCode(0)
{
    _retCode = parseArguments(argc, argv);
}

int CommandLineArgs::parseArguments(int argc, char *argv[])
{
    CLI::App app {
        "Description:\n\t"
        "LazyUT detects tests, affected by the changes in the code.\n"};

    std::string proDirectory;
    std::string srcDirectory;
    std::string testDirectory;
    std::string outDirectory;
    std::string inDirectory;

    std::string exts;
    std::string ignore_substrings;
    std::string extra_dependencies;

    bool verbal = false;
    bool dont_keep_test_main = true;

    // required arguments
    app.add_option("-p,--prodir", proDirectory, "Project root directory");
    app.add_option("-s,--srcdir", srcDirectory, "Directory with library source files")->required();
    app.add_option("-t,--testdir", testDirectory, "Directory with tests source files")->required();
    app.add_option("-o,--outdir", outDirectory, "Output directory")->required();
    // optional arguments
    app.add_option("-d,--deps", extra_dependencies, "Path to the JSON file with extra dependencies");
    app.add_option("-i,--indir", inDirectory, "Input directory");
    app.add_option("-e,--extensions", exts, "Source files extensions, separated by comma (,)");
    app.add_option("--ignore", ignore_substrings, "Substrings of the ignored paths, separated by comma (,)");
    app.add_flag("-m,--main", dont_keep_test_main, "Allways keep test source file with main() implementation");
    app.add_flag("-v,--verbal", verbal, "Verbal mode");
    //

    try {                                                                                                              \
        app.parse(argc, argv);                                                                                   \
    }
    catch(const CLI::ParseError &e) {
        _status = Failure;
        return app.exit(e);;
    }

    if (!exts.empty())
        DirectoryReader::_sourceFileExtensions = split(exts, ",");
    if (!ignore_substrings.empty())
        DirectoryReader::_ignore_substrings = split(ignore_substrings, ",");

    _outDirectory = SplittedPath(outDirectory, SplittedPath::unixSep());
    if (inDirectory.empty())
        _inDirectory = _outDirectory;
    else
        SplittedPath(inDirectory, SplittedPath::unixSep());

    _outDirectory = SplittedPath(outDirectory, SplittedPath::unixSep());
    _outDirectory = SplittedPath(outDirectory, SplittedPath::unixSep());

    _srcsDumpIn = _inDirectory;
    _srcsDumpIn.append(std::string(_srcsFileTreeName));

    _testsDumpIn = _inDirectory;
    _testsDumpIn.append(std::string(_testsFileTreeName));

    _srcsDumpOut = _outDirectory;
    _srcsDumpOut.append(std::string(_srcsFileTreeName));

    _testsDumpOut = _outDirectory;
    _testsDumpOut.append(std::string(_testsFileTreeName));

    _srcsAffected = _outDirectory;
    _srcsAffected.append(std::string(_srcsAffectedFileName));

    _testsAffected = _outDirectory;
    _testsAffected.append(std::string(_testsAffectedFileName));

    _totalAffected = _outDirectory;
    _totalAffected.append(std::string(_totalAffectedFileName));

    _proDirectory = SplittedPath(proDirectory, SplittedPath::unixSep());
    _srcDirectory = absolute_path(SplittedPath(srcDirectory,
                                               SplittedPath::unixSep()),
                                  _proDirectory);

    _testDirectory = absolute_path(SplittedPath(testDirectory,
                                                SplittedPath::unixSep()),
                                   _proDirectory);
    _srcsModified = _outDirectory;
    _srcsModified.append(std::string(_srcsModifiedFileName));
    _testsModified = _outDirectory;
    _testsModified.append(std::string(_testsModifiedFileName));

    _status = Success;
    return 0;
}
