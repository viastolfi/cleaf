#include "compiler/build/file_scanner.h"

bool read_files_in_dir(
    compiled_files_array* files, char* path)
{
  DIR* dir;
  struct dirent* entry;

  if ((dir = opendir(path)) == NULL) {
    return false; 
  }

  while((entry = readdir(dir)) != NULL) {
    if (entry->d_type == 8) {
      char* extension = strrchr(entry->d_name, '.');
      if (!extension)
        continue;

      if (strcmp(extension, ".clf") == 0) {
        char* file = 
          calloc(
              strlen(path) + strlen(entry->d_name) + 1, 
              sizeof(char)); 
        file = 
          strcat(strcat(strcpy(file, path), "/"), entry->d_name);
        da_append(files, file);
      }
    }
    else if (entry->d_type == 4) {
      // for now, we skip some dirs for no other reason than convinience
      // TODO: maybe add `cleaf test` command 
      if (strcmp(entry->d_name, "build") == 0 ||
          strcmp(entry->d_name, "test") == 0 ||
          strcmp(entry->d_name, "docs") == 0 ||
          strcmp(entry->d_name, ".git") == 0 ||
          strcmp(entry->d_name, ".github") == 0 ||
          strcmp(entry->d_name, "..") == 0 ||
          strcmp(entry->d_name, ".") == 0) {
        continue; 
      } 

      char* new = 
        calloc(
            strlen(path) + strlen(entry->d_name) + 1, sizeof(char)); 

      read_files_in_dir(
          files, 
          strcat(strcat(strcpy(new, path), "/"), entry->d_name));
    }
  }

  free(path);
  closedir(dir);
  return true;
}

compiled_files_array find_source_files()
{
  compiled_files_array files = {0};

  char* path = calloc(2, sizeof(char));
  path = strcpy(path, ".");
  if(!read_files_in_dir(&files, path)) {
    return files; 
  }

  return files;
}
