#include "parser.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "error_reporter.hpp"

enum ParsingState
{
    LookingForMacro,
    MacroDirective,
    IncludeMacroDirective,
    ReadingFilename
};

void Parser::parse(const char *fname)
{
    static const auto BUFFER_SIZE = 16*1024;

    int fd = open(fname, O_RDONLY);
    if(fd == -1) {
        errors() << "Failed to open file" << fname;
        return;
    }

#   ifdef __linux__
    /* Advise the kernel of our access pattern.  */
    posix_fadvise(fd, 0, 0, 1);  // FDADVICE_SEQUENTIAL
#   endif

    uintmax_t lines = 0;
    ParsingState parsingState = LookingForMacro;
    static char buf[BUFFER_SIZE + 1];

    while(size_t bytes_read = read(fd, buf, BUFFER_SIZE))
    {
        if(bytes_read == (size_t)-1) {
            errors() << "Read failed";
            return;
        }
        if (!bytes_read)
            break;

        for(char *p = buf; (p = (char*) memchr(p, '#', (buf + bytes_read) - p)); ++p) {
            // start looking for include
            parsingState = MacroDirective;
            p = skipSpaces(buf, p);
        }
    }

    return lines;
}
