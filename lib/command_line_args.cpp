#include "command_line_args.hpp"
#include "directoryreader.hpp"

#include "external/CLI11/CLI11.hpp"

std::string CommandLineArgs::_rootFTreeFilename = "file_tree.bin";
std::string CommandLineArgs::_srcsAffectedFileName = "srcs_affected.txt";
std::string CommandLineArgs::_testsAffectedFileName = "tests_affected.txt";
std::string CommandLineArgs::_testsFileName = "tests_files.txt";
std::string CommandLineArgs::_totalAffectedFileName = "total_affected.txt";

CommandLineArgs clargs;

CommandLineArgs::CommandLineArgs()
    : _verbal(false), _isNoMain(false), _verbosityLevel(0), _retCode(0)
{
}

void CommandLineArgs::parseArguments(int argc, char *argv[])
{
    CLI::App app{
        "Description:\n\t"
        "LazyUT detects tests, affected by the changes in the code.\n"};

    std::string rootDir;
    std::string outDirectory;
    std::string inDirectory;

    std::string srcBase;
    std::string testBase;

    std::string exts;
    std::string extra_dependencies;

    std::string ignoredOutput;

    // required arguments
    app.add_option("-r,--root", rootDir,
                   "File tree Root directory, every listed file should be "
                   "relative to this")
        ->required();
    app.add_option("-o,--outdir", outDirectory, "Output directory")->required();

    // optional arguments
    app.add_option("-s,--src-dirs", _srcDirectories,
                   "Directories with source files, separated by comma (,),"
                   "relative to Root, by default \"\"");
    app.add_option("-t,--test-dirs", _testDirectories,
                   "Directories with test files, separated by comma (,),"
                   "relative to Root, by default \"\"");
    app.add_option("--test-patterns", _testPatterns,
                   "Patterns for test file names, separated by comma (,)");
    app.add_option("-i,--indir", inDirectory, "Input directory");
    app.add_option("-d,--deps", extra_dependencies,
                   "Path to the file with extra dependencies");
    app.add_option("-e,--extensions", exts,
                   "Source files extensions, separated by comma (,)");
    app.add_option("--ignore", _ignoredSubstrings,
                   "Substrings of the ignored paths, separated by comma (,) ");
    app.add_option(
        "--ignore-out", ignoredOutput,
        "Substrings of the paths which will not be included in output,"
        "separated by comma (,) ");
    app.add_option(
        "--test-base", testBase,
        "Directory relative to which the test source files are displayed");
    app.add_option("--src-base", srcBase,
                   "Directory relative to which the project source files "
                   "are displayed");
    app.add_option("--include-paths", _includePaths,
                   "Extra include paths both for source and test, "
                   "relative to project directory,"
                   "separated by comma (,)");
    app.add_option("--verbosity-level", _verbosityLevel,
                   "From 0 to 2, the higher is more verbose");

    app.add_flag("-m,--no-main", _isNoMain,
                 "Don't keep test source file with main() implementation");
    app.add_flag("-v,--verbal", _verbal, "Verbal mode");
    //

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        _status = Failure;
        _retCode = app.exit(e);
        return;
    }

    if (!exts.empty())
        DirectoryReader::_sourceFileExtensions = split(exts, ",");

    _rootDirectory = SplittedPath(rootDir, SplittedPath::unixSep());

    _outDirectory = SplittedPath(outDirectory, SplittedPath::unixSep());
    _inDirectory = SplittedPath(inDirectory, SplittedPath::unixSep());

    if (_inDirectory.empty())
        _inDirectory = _outDirectory;
    if (_outDirectory.empty())
        _outDirectory = _inDirectory;

    _ftreeDumpIn = _inDirectory;
    _ftreeDumpIn.append(std::string(_rootFTreeFilename));

    _ftreeDumpOut = _outDirectory;
    _ftreeDumpOut.append(std::string(_rootFTreeFilename));

    _srcsAffected = _outDirectory;
    _srcsAffected.append(std::string(_srcsAffectedFileName));

    _testsAffected = _outDirectory;
    _testsAffected.append(std::string(_testsAffectedFileName));

    _totalAffected = _outDirectory;
    _totalAffected.append(std::string(_totalAffectedFileName));

    _extraDependencies =
        SplittedPath(extra_dependencies, SplittedPath::unixSep());

    if (!srcBase.empty())
        _srcBase = SplittedPath(srcBase, SplittedPath::unixSep());
    else
        _srcBase = SplittedPath("", SplittedPath::unixSep());

    if (!testBase.empty())
        _testBase = SplittedPath(testBase, SplittedPath::unixSep());
    else
        _testBase = SplittedPath("", SplittedPath::unixSep());

    _ignoredOutput = split(ignoredOutput, ",");

    _status = Success;
    _retCode = 0;
}

SplittedPath CommandLineArgs::testFilesPath() const
{
    auto tmp = _outDirectory;
    tmp.append(_testsFileName);
    return tmp;
}

std::vector< SplittedPath > CommandLineArgs::srcDirectories() const
{
    auto tmp = splittedPaths(_srcDirectories);
    if (tmp.empty())
        tmp.push_back(SplittedPath("", SplittedPath::unixSep()));
    return tmp;
}

std::vector< SplittedPath > CommandLineArgs::testDirectories() const
{
    auto tmp = splittedPaths(_testDirectories);
    if (tmp.empty())
        tmp.push_back(SplittedPath("", SplittedPath::unixSep()));
    return tmp;
}

CommandLineArgs::StringVector CommandLineArgs::testPatterns() const
{
    return split(_testPatterns, ",");
}

std::vector< SplittedPath > CommandLineArgs::includePaths() const
{
    return splittedPaths(_includePaths);
}

CommandLineArgs::StringVector CommandLineArgs::ignoredSubstrings() const
{
    return split(_ignoredSubstrings, ",");
}

const CommandLineArgs::StringVector &CommandLineArgs::ignoredOutputs() const
{
    return _ignoredOutput;
}

std::vector< SplittedPath >
CommandLineArgs::splittedPaths(const std::string &str)
{
    auto strVector = split(str, ",");
    std::vector< SplittedPath > result;
    result.reserve(strVector.size());

    std::transform(strVector.begin(), strVector.end(),
                   std::back_inserter(result), [](const std::string &s) {
                       return SplittedPath(s, SplittedPath::unixSep());
                   });
    return result;
}
