#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "port/fs.h"

static char root[MAX_PATH];
static char seen[40][32];
static int count;

const char *host_user_dir(void) { return root; }
const char *host_game_dir(void) { return root; }
const char *host_exe_dir(void) { return root; }
void harness_note_file(const char *path) { (void)path; }
int mods_get_int(const char *key, int def) { (void)key; return def; }
void mods_save_opened(const char *path) { (void)path; }
void mods_save_written(const char *path) { (void)path; }
void living_written(const char *path) { (void)path; }

static void collect(void *ctx, const char *name, int is_dir, unsigned size) {
  (void)ctx;
  (void)size;
  assert(!is_dir && count < 40);
  snprintf(seen[count++], sizeof seen[0], "%s", name);
}

int main(int argc, char **argv) {
  assert(argc == 2 && strlen(argv[1]) < sizeof root);
  snprintf(root, sizeof root, "%s", argv[1]);
  char dir[MAX_PATH + 32];
  snprintf(dir, sizeof dir, "%s/Saved Games", root);
  assert(CreateDirectoryA(dir, NULL) || GetLastError() == ERROR_ALREADY_EXISTS);
  for (int i = 0; i < 40; i++) {
    char path[MAX_PATH + 48];
    snprintf(path, sizeof path, "%s/s%02d.sve", dir, i);
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(h != INVALID_HANDLE_VALUE);
    ULARGE_INTEGER stamp;
    stamp.QuadPart = 132000000000000000ULL + (uint64_t)(i == 21 ? 20 : i) * 10000000ULL;
    FILETIME ft = {stamp.LowPart, stamp.HighPart};
    assert(SetFileTime(h, NULL, NULL, &ft));
    CloseHandle(h);
  }
  assert(fs_list("Saved Games\\*.sve", collect, NULL) == 40);
  assert(count == 40);
  int pos = 0;
  for (int i = 39; i >= 22; i--) {
    char want[32]; snprintf(want, sizeof want, "s%02d.sve", i);
    assert(!strcmp(seen[pos++], want));
  }
  assert(!strcmp(seen[pos++], "s20.sve"));
  assert(!strcmp(seen[pos++], "s21.sve"));
  for (int i = 19; i >= 0; i--) {
    char want[32]; snprintf(want, sizeof want, "s%02d.sve", i);
    assert(!strcmp(seen[pos++], want));
  }
  assert(pos == 40);
  puts("40 save files: newest first, alphabetical ties, correct count");
  return 0;
}
