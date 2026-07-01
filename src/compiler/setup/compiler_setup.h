#ifndef COMPILER_SETUP_H
#define COMPILER_SETUP_H

#include "compiler/definition/compiler_definition.h"
#include "thirdparty/log.h"
#include "compiler/build/file_scanner.h"

compiler_resources_t* single_file_setup(int argc, char** argv);
compiler_resources_t* build_setup();

#endif // COMPILER_SETUP_H
