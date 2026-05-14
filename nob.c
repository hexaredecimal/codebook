#define NOB_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "nob.h"

typedef struct {
  char** items;
  uint64_t count;
  uint64_t capacity;
} StringList;


#define SOURCE_FILE_COUNT 15 // Modify this when adding a new files to the list of files

bool to_files(const char*[SOURCE_FILE_COUNT], StringList*, const char*, const char*);
bool build_object_files(const char*[SOURCE_FILE_COUNT], StringList, StringList*);
bool build_library(StringList);
bool build_example(void);

int main(int argc, char** argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  if (!build_example()) {
    return 1;
  }

  return 0;
}

bool build_example(void) {
  char* raylib_home = getenv("RAYLIB_HOME");
  if (raylib_home == NULL) {
    nob_log(NOB_ERROR, "RAYLIB_HOME is not set");
    return false;
  }

  char raylib_includes[1024];
  char includes[1024];

  sprintf(raylib_includes, "-I%s/include/", raylib_home);
  sprintf(includes, "-Iinclude/");

  char libs[1024];
  sprintf(libs, "-L%s/lib/", raylib_home);

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "g++");
  nob_cmd_append(&cmd, "-ggdb");
  nob_cmd_append(&cmd, "-std=c++17");
  nob_cmd_append(&cmd, raylib_includes);
  nob_cmd_append(&cmd, includes);
  nob_cmd_append(&cmd, "src/main.cpp");
  nob_cmd_append(&cmd, "-o", "main.exe");
  nob_cmd_append(&cmd, libs, "-Lbuild/");
  nob_cmd_append(&cmd,
    "-lraylib", "-lm", "-lwinmm",
    "-lgdi32", "-lopengl32", "-lole32", "-lcomdlg32");

  nob_log(NOB_INFO, "Building example");
  if (!nob_cmd_run(&cmd)) {
    nob_log(NOB_ERROR, "Failed to build example");
    return false;
  }

  nob_log(NOB_INFO, "Example built successfully");
  return true;
}

bool build_library(StringList object_files) {
  const char* static_object_name = "build/librasen.a";
#if defined(WIN32)
  const char* shared_object_name = "build/rasen.dll";
  const char* shared_lib_name = ".DLL";
#else
  const char* shared_object_name = "build/rasen.so";
  const char* shared_lib_name = ".SO";
#endif

  char* raylib_home = getenv("RAYLIB_HOME");
  if (raylib_home == NULL) {
    nob_log(NOB_ERROR, "RAYLIB_HOME is not set");
    return false;
  }

  char libs[1024];
  sprintf(libs, "-L%s/lib/", raylib_home);

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "g++");
  nob_cmd_append(&cmd, "-shared");

  for (uint64_t i = 0; i < object_files.count; ++i) {
    nob_cmd_append(&cmd, object_files.items[i]);
  }

  nob_cmd_append(&cmd, "-o", shared_object_name);
  nob_cmd_append(&cmd, libs);
  nob_cmd_append(&cmd, "-lraylib", "-lm", "-lwinmm", "-lgdi32", "-lopengl32",
    "-lgdi32", "-lopengl32", "-lole32", "-lcomdlg32");


  nob_log(NOB_INFO, "Creating .a file [%s]", static_object_name);

  if (!nob_cmd_run(&cmd)) {
    return false;
  }

  cmd.count = 0;

  nob_cmd_append(&cmd, "ar");
  nob_cmd_append(&cmd, "rcs");
  nob_cmd_append(&cmd, static_object_name);

  for (uint64_t i = 0; i < object_files.count; ++i) {
    nob_cmd_append(&cmd, object_files.items[i]);
  }

  nob_log(NOB_INFO, "Creating %s file [%s]", shared_lib_name, shared_object_name);

  if (!nob_cmd_run(&cmd)) {
    return false;
  }

  return true;
}

bool build_object_files(const char* names[SOURCE_FILE_COUNT], StringList source_files, StringList* object_files) {
  if (!to_files(names, object_files, "build", "o")) {
    nob_log(NOB_ERROR, "Failed to name object files");
  }

  char* raylib_home = getenv("RAYLIB_HOME");
  if (raylib_home == NULL) {
    nob_log(NOB_ERROR, "RAYLIB_HOME is not set");
    return false;
  }

  char raylib_includes[1024];
  char includes[1024];

  sprintf(raylib_includes, "-I%s/include/", raylib_home);
  sprintf(includes, "-Iinclude/");

  char libs[1024];
  sprintf(libs, "-L%s/lib/", raylib_home);

  Nob_Proc procs = {0};

  for (uint64_t i = 0; i < SOURCE_FILE_COUNT; ++i) {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "g++");
    nob_cmd_append(&cmd, "-o");
    nob_cmd_append(&cmd, object_files->items[i]);
    nob_cmd_append(&cmd, "-c");
    nob_cmd_append(&cmd, source_files.items[i]);
    nob_cmd_append(&cmd, raylib_includes);
    nob_cmd_append(&cmd, includes);

    nob_log(NOB_INFO, "Compiling [%s] to [%s]", source_files.items[i], object_files->items[i]);
    if (!nob_cmd_run(&cmd)) {
      return false;
    }

  }

  return true;
}


bool to_files(const char* names[SOURCE_FILE_COUNT], StringList* source_files, const char* dir, const char* ext) {
  source_files->count = 0;
  for (uint64_t i = 0; i < SOURCE_FILE_COUNT; i++) {
    const char* name = names[i];
    char buffer[1024];
    sprintf(buffer, "%s/%s.%s", dir, name, ext);
    nob_da_append(source_files, strdup(buffer));
  }

  assert(SOURCE_FILE_COUNT == source_files->count);
  return true;
}

