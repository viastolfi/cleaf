#include "compiler_setup.h"

compiler_resources_t* single_file_setup(int argc, char** argv)
{
  log_verbosity_t verbosity = LOG_VERBOSE;
  const char* output = NULL;
  const char* filename = NULL;
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

  log_phase("compiling", "'%s'", filename);

  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    error_report_general(
        ERROR_SEVERITY_ERROR, "cannot open file '%s'", filename);
    return NULL;
  }

  compiler_resources_t* res = 
    calloc(1, sizeof(compiler_resources_t));

  res->output = output;
  res->filename = filename;
  res->text = (char *) malloc(1 << 20);
  int len = (int) fread(res->text, 1, 1 << 20, f);
  fclose(f);
  if (len < 0) {
    error_report_general(
        ERROR_SEVERITY_ERROR, "failed to read file '%s'", res->filename);
    compiler_resources_free(res);
    return NULL;
  }

  res->len = len;
  return res;
}

