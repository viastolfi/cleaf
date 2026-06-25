#include "compiler_setup.h"

compiler_resources_t* single_file_setup(int argc, char** argv)
{
  log_verbosity_t verbosity = LOG_VERBOSE;
  const char* output = NULL;
  char* filename = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-V") == 0)
      verbosity = LOG_DUMP;
    else if (strcmp(argv[i], "-o") == 0) {
      if (++i >= argc) {
        error_report_general(
            ERROR_SEVERITY_ERROR, "missing argument for '-o'");
        return NULL;
      }
      output = argv[i];
    }
    else if (argv[i][0] != '-')
      filename = argv[i];
    else {
      error_report_general(
          ERROR_SEVERITY_ERROR, "unknown flag '%s'", argv[i]);
      fprintf(
          stderr, "usage: %s [-V] [-o <output>] <file.clf>\n", 
          argv[0]);
      return NULL;
    }
  }

  log_set_verbosity(verbosity);

  if (!filename) {
    error_report_general(
        ERROR_SEVERITY_ERROR, "no input file provided");
    fprintf(
        stderr, "usage: %s [-v|-V] [-o <output>] <file.clf>\n", 
        argv[0]);
    return NULL;
  }

  compiler_resources_t* res = 
    calloc(1, sizeof(compiler_resources_t));

  res->output = output;
  da_append(&(res->files), strdup(filename));
  return res;
}

compiler_resources_t* build_setup() 
{
  log_verbosity_t verbosity = LOG_VERBOSE;
  log_set_verbosity(verbosity);

  compiler_resources_t* res = 
    calloc(1, sizeof(compiler_resources_t));

  res->files = find_source_files();  

  return res;
}

