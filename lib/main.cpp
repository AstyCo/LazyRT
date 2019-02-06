#include "extensions/help_functions.hpp"

#include "extensions/flatbuffers_extensions.hpp"
#include "command_line_args.hpp"

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

    /// TODO: CHECK EXTRA_DEPS

    PROFILE(rootTree.parsePhase(clargs.ftreeDumpIn()));

    PROFILE(rootTree.analyzePhase());

    if (!clargs.isNoMain())
        rootTree.labelTestMain();

    rootTree.writeAffectedFiles(clargs);

    if (clargs.verbal()) {
        rootTree.print();
        std::cout << "AFFECTED SOURCES" << std::endl;
        rootTree.writeFiles(std::cout, &FileNode::isAffectedSource);
        std::cout << "AFFECTED TESTS" << std::endl;
        rootTree.writeFiles(std::cout, &FileNode::isAffectedTest);
        std::cout << "MODIFIED" << std::endl;
        rootTree.writeFiles(std::cout, &FileNode::isModified);

        std::cout << "write lazyut files to " << clargs.outDir().joint()
                  << std::endl;
    }

    PROFILE(FileTreeFunc::serialize(rootTree, clargs.ftreeDumpOut()));

    return 0;
}
