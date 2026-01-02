#include "file.h"
#include "unity.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// --- Test Support ---

static char SANDBOX_ROOT[PATH_MAX];
static char JAIL_DIR[PATH_MAX];
static char OUTSIDE_DIR[PATH_MAX];

void setUp(void) {
  // 1. Create a sandbox directory in /tmp
  snprintf(SANDBOX_ROOT, sizeof(SANDBOX_ROOT), "/tmp/test_safe_path_%d",
           getpid());
  mkdir(SANDBOX_ROOT, 0700);

  // 2. Create the "Jail" (Authorized Root)
  snprintf(JAIL_DIR, sizeof(JAIL_DIR), "%s/www", SANDBOX_ROOT);
  mkdir(JAIL_DIR, 0700);

  // 3. Create a directory "Outside" the jail (Unauthorized)
  snprintf(OUTSIDE_DIR, sizeof(OUTSIDE_DIR), "%s/secret", SANDBOX_ROOT);
  mkdir(OUTSIDE_DIR, 0700);
}

void tearDown(void) {
  // Recursive delete using system is safest/simplest for test teardown to avoid
  // implementing nftw() complexity in test code.
  char cmd[PATH_MAX * 2];
  snprintf(cmd, sizeof(cmd), "rm -rf %s", SANDBOX_ROOT);
  int ret = system(cmd);
  (void)ret; // Silence unused result warning
}

// Helper to create files quickly
void create_test_file(const char *path) {
  FILE *f = fopen(path, "w");
  if (f) {
    fprintf(f, "test");
    fclose(f);
  }
}

// --- Test Cases ---

void test_GetSafePath_ValidFile_ReturnsAbsolutePath(void) {
  char file_path[PATH_MAX];
  snprintf(file_path, sizeof(file_path), "%s/index.html", JAIL_DIR);
  create_test_file(file_path);

  char *result = get_safe_path(JAIL_DIR, "index.html");

  TEST_ASSERT_NOT_NULL_MESSAGE(result, "Should resolve valid file");
  TEST_ASSERT_EQUAL_STRING(file_path, result);

  free(result);
}

void test_GetSafePath_TraversalAttempt_ReturnsNull(void) {
  // Create a secret file outside
  char secret_file[PATH_MAX];
  snprintf(secret_file, sizeof(secret_file), "%s/passwd", OUTSIDE_DIR);
  create_test_file(secret_file);

  // Attack: "../secret/passwd" relative to jail
  char *result = get_safe_path(JAIL_DIR, "../secret/passwd");

  TEST_ASSERT_NULL_MESSAGE(result, "Should block traversal to outside file");
  // Note: realpath would resolve this, but the prefix check must fail it.
}

void test_GetSafePath_SymlinkToOutside_ReturnsNull(void) {
  // Create actual secret file
  char secret_file[PATH_MAX];
  snprintf(secret_file, sizeof(secret_file), "%s/data.db", OUTSIDE_DIR);
  create_test_file(secret_file);

  // Create symlink INSIDE jail pointing OUTSIDE
  char link_path[PATH_MAX];
  snprintf(link_path, sizeof(link_path), "%s/link_to_db", JAIL_DIR);
  if (symlink(secret_file, link_path) != 0) {
    TEST_IGNORE_MESSAGE("Failed to create symlink, skipping test");
  }

  char *result = get_safe_path(JAIL_DIR, "link_to_db");

  TEST_ASSERT_NULL_MESSAGE(result,
                           "Should block symlink resolving outside root");
}

void test_GetSafePath_PartialDirectoryMatch_ReturnsNull(void) {
  // Create a directory that shares a prefix but is different
  // Jail: .../www
  // Trap: .../www-secret
  char trap_dir[PATH_MAX];
  snprintf(trap_dir, sizeof(trap_dir), "%s-secret", JAIL_DIR);
  mkdir(trap_dir, 0700);

  char trap_file[PATH_MAX];
  snprintf(trap_file, sizeof(trap_file), "%s/config", trap_dir);
  create_test_file(trap_file);

  // The attack: "../www-secret/config"
  // If we only checked if "starts with .../www", this might pass if logic is
  // flawed.
  char *result = get_safe_path(JAIL_DIR, "../www-secret/config");

  TEST_ASSERT_NULL_MESSAGE(result,
                           "Should block partial directory prefix match");
}

void test_GetSafePath_NestedDirectories_ReturnsValid(void) {
  char sub_dir[PATH_MAX];
  snprintf(sub_dir, sizeof(sub_dir), "%s/images", JAIL_DIR);
  mkdir(sub_dir, 0700);

  char file_path[PATH_MAX];
  snprintf(file_path, sizeof(file_path), "%s/logo.png", sub_dir);
  create_test_file(file_path);

  char *result = get_safe_path(JAIL_DIR, "images/logo.png");

  TEST_ASSERT_NOT_NULL(result);
  TEST_ASSERT_EQUAL_STRING(file_path, result);
  free(result);
}

void test_GetSafePath_NullInputs_ReturnsNull(void) {
  TEST_ASSERT_NULL(get_safe_path(NULL, "file"));
  TEST_ASSERT_NULL(get_safe_path(JAIL_DIR, NULL));
}

void test_GetSafePath_NonExistentFile_ReturnsNull(void) {
  // realpath fails if file doesn't exist, which is implicit safety here
  char *result = get_safe_path(JAIL_DIR, "ghost_file.txt");
  TEST_ASSERT_NULL(result);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_GetSafePath_ValidFile_ReturnsAbsolutePath);
  RUN_TEST(test_GetSafePath_NestedDirectories_ReturnsValid);
  RUN_TEST(test_GetSafePath_TraversalAttempt_ReturnsNull);
  RUN_TEST(test_GetSafePath_SymlinkToOutside_ReturnsNull);
  RUN_TEST(test_GetSafePath_PartialDirectoryMatch_ReturnsNull);
  RUN_TEST(test_GetSafePath_NullInputs_ReturnsNull);
  RUN_TEST(test_GetSafePath_NonExistentFile_ReturnsNull);
  return UNITY_END();
}
