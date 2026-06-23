#ifndef BUILD_SCANNER_H
#define BUILD_SCANNER_H

#include <dirent.h>
#include <sys/types.h>

#include "compiler/definition/compiler_definition.h"

compiled_files_array find_source_files();
bool read_files_in_dir(
    compiled_files_array* files, char* path);

#endif // BUILD_SCANNER_H
