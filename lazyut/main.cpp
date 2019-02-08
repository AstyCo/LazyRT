#include <command_line_args.hpp>
#include <extensions/flatbuffers_extensions.hpp>
#include <extensions/help_functions.hpp>

#include <iostream>

#define PROFILE(x)                                                             \
    prf.step();                                                                \
    x;                                                                         \
    prf.step(#x)
#define START_PROFILE Profiler prf(clargs.verbal());

int main(int argc, char *argv[])
{
    clargs.parseArguments(argc, argv);
    if (clargs.status() != CommandLineArgs::Success)
        return clargs.retCode();

    START_PROFILE;

    FileTree rootTree;
    rootTree.setRootPath(clargs.rootDirectory());

    PROFILE(rootTree.readFiles(clargs));

    rootTree.addIncludePaths(clargs.includePaths());

    rootTree.installExtraDependencies(clargs.extraDeps());

    PROFILE(rootTree.parsePhase(clargs.ftreeDumpIn()));

    PROFILE(rootTree.analyzePhase());

    if (!clargs.isNoMain())
        rootTree.labelTestMain();

    rootTree.writeAffectedFiles(clargs);

    if (clargs.verbal())
        rootTree.printAll();

    PROFILE(FileTreeFunc::serialize(rootTree, clargs.ftreeDumpOut()));

    return 0;
}
