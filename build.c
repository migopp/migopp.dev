#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define BOLD "\x1b[1m"
#define RESET "\x1b[0m"

#define log(body, ...)                                                         \
  fprintf(stdout,                                                              \
          BOLD BLUE "[.dev build] " RESET body "\n"__VA_OPT__(, __VA_ARGS__))
#define log_war(body, ...)                                                     \
  fprintf(stdout, BOLD YELLOW "[.dev build] " RESET body                       \
                              "\n"__VA_OPT__(, __VA_ARGS__))
#define log_succ(body, ...)                                                    \
  fprintf(stdout,                                                              \
          BOLD GREEN "[.dev build] " RESET body "\n"__VA_OPT__(, __VA_ARGS__))
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

// Builds a given directory structure in `target/`
//
// To simply build `target`: 	`build_target_structure(".")`
// To build `target/a/b`: 		`build_target_structure("a/b")`
int build_target_structure(const char *f) {
  // Check if `target` already exists, and if it does
  // there is no need to remake it.
  log("Checking `target/%s` structure.", f);
  char fp[1000];
  int rc = snprintf(fp, 1000, "target/%s", f);
  if (rc > 1000 || rc < 0) {
    log_err("Failed to construct filepath for `%s`", fp);
    return -1;
  }
  printf("FP: %s\n", fp);
  struct stat sb;
  if (stat(fp, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    log("`%s` already exists... Continuing.", fp);
    return 0;
  } else {
    // Build `target/f`.
    log_war("`%s` doesn't exist... Building.", fp);
    char cmd[1000];
    int rc = snprintf(cmd, 1000, "mkdir -p %s", fp);
    if (rc > 1000 || rc < 0) {
      log_err("Failed to construct `mkdir` cmd for `%s`", fp);
      return -1;
    }
    rc = system(cmd);
    if (rc < 0) {
      log_err("Failed to build `%s`. Aborting.", fp);
      return -1;
    }
    return 0;
  }
}

// Cuts extraneous parts of the given file path for the purposes of
// relocating the file post-compilation.
//
// Examples:
// `src/index.md`	 		-> `index`
// `src/notes/notes.md` 	-> `notes/` `notes`
// `src/notes/os/vmem.md` 	-> `notes/os/` `vmem`
// ...
//
// NOTE: This populates the `cut_fp` buffer with the desired file path.
int cut_md_file_path(const char *md_fp, char *cut_p, char *cut_f) {
  // Find start index.
  //
  // This is the first character in the string following `src/`.
  // NOTE: Assumes that the path begins with exactly `src/`.
  size_t i = 4;
  const char *start = &md_fp[i];
  const char *src = "src/";
  for (int t = 0; t < 4; ++t) {
    assert(md_fp[t] == src[t]);
  }

  // Now we find the end index.
  // NOTE: Assumes that the path ends with exactly `.md`.
  //
  // Back up 1 so that our loop is prettier.
  i--;
  while (md_fp[++i] != 0)
    ;
  const char *end = &md_fp[i - 3];
  const char *md = ".md";
  for (int t = 0; t < 3; ++t) {
    assert(md_fp[i - 3 + t] == md[t]);
  }

  // Find the beginning of the file name.
  //
  // This is everything from `end` to the previous `/`.
  // e.g., `vmem` in the last example of the doc comment.
  while (md_fp[--i] != '/')
    ;
  const char *f = &md_fp[i + 1];

  // Now `memcpy`.
  //
  // First we do the file name itself.
  size_t f_len = end - f;
  memcpy(cut_f, f, f_len);
  cut_f[f_len] = 0;
  // Now we do the parent directory.
  if (f == start) {
    cut_p[0] = 0;
  } else {
    size_t p_len = f - 1 - start;
    memcpy(cut_p, start, p_len);
    cut_p[p_len] = 0;
  }
  return 0;
}

#define PANDOC " pandoc "
#define PANDOC_FORMAT_FLAGS " -f markdown -t html "
#define PANDOC_MAIN_TMPL_FLAG " --template=tmpl/main.tmpl "

// Compiles a single `md` file to `html`.
//
// `./src/.../example.md` -> `./target/.../example.html`
//
// NOTE: This currently just uses the `main.tmpl` file for templating.
// This should be customized in the future if multiple templates arise.
//
// TODO: It might be kinda cool to only build an `html` file again if
// the `.md` was edited after the `.html` was. Kinda like `make`.
int file_md_to_html(const char *md_fp) {
  int rc;

  // Format build command.
  //
  // First we must cut out the extraneous parts of the `.md` file path.
  // Then, we can build the file path to the `.html` target.
  char cut_md_p[100];
  char cut_md_f[100];
  rc = cut_md_file_path(md_fp, cut_md_p, cut_md_f);
  if (rc < 0) {
    log_err("Attempt to cut file path `%s` failed. Skipping.", md_fp);
    return -1;
  }
  log("Split `md_fp` into folder `%s` and basename `%s`", cut_md_p, cut_md_f);
  char html_fp[100];
  rc = snprintf(html_fp, 100, "target/%s/%s.html", cut_md_p, cut_md_f);
  if (rc > 100 || rc < 0) {
    log_err("Attempt to form file path to `.html` for `%s` failed.", md_fp);
    return -1;
  }

  // Before we actually compile, we should create the `target/cut_md_p`
  // directory if it does not already exist.
  rc = build_target_structure(cut_md_p);
  if (rc < 0) {
    log_err("Attempt to build target structure for directory `%s` failed.",
            cut_md_p);
    return -1;
  }

  // Now we can safely build.
  char cmd[200];
  rc = snprintf(cmd, 200,
                PANDOC PANDOC_FORMAT_FLAGS "%s -o %s" PANDOC_MAIN_TMPL_FLAG,
                md_fp, html_fp);
  if (rc > 200 || rc < 0) {
    log_err("Attempt to form `pandoc` command for `%s` failed.", md_fp);
    return -1;
  }

  // Run build command.
  rc = system(cmd);
  if (rc < 0) {
    log_err("Build with `pandoc` failed. Aborting build of `%s`.", md_fp);
    return -1;
  }
  return 0;
}

// Builds the website html out of the `.md` files in `src`.
//
// Or, more generally, `dir`. Since this is most intuitively
// written as a recursive function.
int build_website_html(const char *fp) {
  log("`build_website_html` called on `%s`", fp);
  DIR *dir;
  struct dirent *ent;
  dir = opendir(fp);
  if (dir) {
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type == DT_REG) {
        log("Found file `%s`", ent->d_name);

        // Build the full file path.
        const char *ent_fn = ent->d_name;
        char ent_fp[100];
        int rc = snprintf(ent_fp, 100, "%s/%s", fp, ent_fn);
        if (rc > 100 || rc < 0) {
          log_err("Failed to build file path to `%s` in `%s`", ent_fn, fp);
          return -1;
        }

        // Now we can build the file.
        log("Building `.html` file from `%s`", ent_fp);
        rc = file_md_to_html(ent_fp);
        if (rc < 0) {
          return -1;
        }
      } else if (ent->d_type == DT_DIR) {
        // Check to make sure this is not `.` or `..`
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
          continue;
        }
        log("Found directory `%s`", ent->d_name);

        // Build everything in the subdirectory.
        //
        // First, we have to get the full file path again.
        const char *ent_dn = ent->d_name;
        char ent_fp[100];
        int rc = snprintf(ent_fp, 100, "%s/%s", fp, ent_dn);
        if (rc > 100 || rc < 0) {
          log_err("Failed to build file path to `%s` in `%s`", ent_dn, fp);
          return -1;
        }

        // TODO: Error handling?
        //
        // Not really sure what there is to handle, though.
        // If we to make something, we can just say so and move on.
        build_website_html(ent_fp);
      } else {
        log_war("Found unknown entry (%d)", ent->d_type);
      }
    }
  }
  return 0;
}

#define try(fn)                                                                \
  ({                                                                           \
    if (fn < 0) {                                                              \
      exit(-1);                                                                \
    }                                                                          \
  })

int main(void) {
  try(build_target_structure("."));
  try(build_website_html("src"));
  return 0;
}
