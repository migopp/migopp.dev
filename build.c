#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define RED "\x1b[31m"
#define GRE "\x1b[32m"
#define YEL "\x1b[33m"
#define BLU "\x1b[34m"
#define BOLD "\x1b[1m"
#define RESET "\x1b[0m"

#define log(body, ...)                                                         \
  fprintf(stdout,                                                              \
          BOLD BLU "[.dev build] " RESET body "\n"__VA_OPT__(, __VA_ARGS__))
#define log_war(body, ...)                                                     \
  fprintf(stdout,                                                              \
          BOLD YEL "[.dev build] " RESET body "\n"__VA_OPT__(, __VA_ARGS__))
#define log_succ(body, ...)                                                    \
  fprintf(stdout,                                                              \
          BOLD GRE "[.dev build] " RESET body "\n"__VA_OPT__(, __VA_ARGS__))
#define log_err(body, ...)                                                     \
  fprintf(stderr,                                                              \
          BOLD RED "[.dev build] " RESET body "\n"__VA_OPT__(, __VA_ARGS__))

// Asserts that a condition is met.
//
// NOTE: I'm unsure if statement expressions are portable. Let us hope so.
#define assert(exp)                                                            \
  ({                                                                           \
    if (!(exp)) {                                                              \
      fprintf(stderr,                                                          \
              "BOLD RED [.dev build] ASSERTION " RESET "(%s) FAILED @ %d",     \
              #exp, __LINE__);                                                 \
      return -1;                                                               \
    }                                                                          \
  })

// Sets up directory structure for deployment.
//
// This is just making `target` for now.
int build_target_structure() {
  // Check if `target` already exists, and if it does
  // there is no need to remake it.
  log("Checking `target` structure.");
  struct stat sb;
  if (stat("target", &sb) == 0 && S_ISDIR(sb.st_mode)) {
    log("`target` already exists... Continuing.");
    return 0;
  } else {
    // Build `target`.
    log_war("`target` doesn't exist... Building.");
    int rc = system("mkdir target");
    if (rc < 0) {
      log_err("Failed to build `target`. Aborting.");
      return -1;
    }
    return 0;
  }
}

// Cuts extraneous parts of the given file path for the purposes of
// relocating the file post-compilation.
//
// Examples:
// `./src/index.md` 		-> `index`
// `./src/notes/notes.md` 	-> `notes/notes`
// ...
//
// NOTE: This populates the `cut_fp` buffer with the desired file path.
int cut_md_file_path(const char *md_fp, char *cut_fp) {
  // Find start index.
  //
  // This is the first character in the string following `./src/`.
  size_t i = 0;
  assert(md_fp[0] == '.' || md_fp[0] == 's');
  return 0;
}

#define PANDOC "pandoc "
#define PANDOC_FORMAT_FLAGS "-f markdown -t html "
#define PANDOC_MAIN_TMPL_FLAG "--template=tmpl/main.tmpl "

// Compiles a single `md` file to `html`.
//
// `./src/.../example.md` -> `./target/.../example.html`
//
// NOTE: This currently just uses the `main.tmpl` file for templating.
// This should be customized in the future if multiple templates arise.
int file_md_to_html(const char *md_fp) {
  int rc;

  // Double-check that `fp` file exists.

  // Format build command.
  //
  // First we must get the updated path to the
  char cut_md_fp[100];
  rc = cut_md_file_path(md_fp, cut_md_fp);
  if (rc < 0) {
    log_err("Attempt to cut file path `%s` failed. Skipping.", md_fp);
    return -1;
  }
  char html_fp[100];
  rc = snprintf(html_fp, 100, "");
  if (rc > 100 || rc < 0) {
  }
  char cmd[200];
  rc = snprintf(cmd, 200,
                PANDOC PANDOC_FORMAT_FLAGS "%s -o %s" PANDOC_MAIN_TMPL_FLAG,
                md_fp, html_fp);
  if (rc > 200 || rc < 0) {
  }

  // Run build command.
  rc = system(PANDOC PANDOC_FORMAT_FLAGS "-o " PANDOC_FORMAT_FLAGS);
  if (rc < 0) {
    log_err("Build with `pandoc` failed. Aborting.");
    return -1;
  }
  return 0;
}

#define try(fn)                                                                \
  ({                                                                           \
    if (fn < -1) {                                                             \
      exit(-1);                                                                \
    }                                                                          \
  })

int main(void) {
  try(build_target_structure());
  try(file_md_to_html("./src/index.md"));
  return 0;
}
