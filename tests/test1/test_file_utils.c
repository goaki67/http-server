#include "file.h"
#include "unity.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* --- Constants --- */
#define TEMP_FILE_PATH "test_linux_data.bin"
#define TEMP_LINK_PATH "test_linux_link.bin"
#define TEMP_DIR_PATH "test_linux_dir"
#define NON_EXISTENT_FILE "ghost_file.bin"

/* --- Helper Functions --- */

void create_test_file(const void *content, size_t size) {
  FILE *fp = fopen(TEMP_FILE_PATH, "wb");
  if (fp) {
    if (size > 0 && content != NULL) {
      fwrite(content, 1, size, fp);
    }
    fclose(fp);
  }
}

void cleanup_artifacts() {
  unlink(TEMP_FILE_PATH);
  unlink(TEMP_LINK_PATH);
  rmdir(TEMP_DIR_PATH); // Only works if empty, which is fine
}

/* --- Unity Setup/Teardown --- */

void setUp(void) { cleanup_artifacts(); }

void tearDown(void) { cleanup_artifacts(); }

/* --- Tests --- */

// 1. Happy Path: Binary Content
// Linux handles binary files and text files identically, but we test
// strict byte content to ensure no newline conversions happen.
void test_GetFileContents_Should_ReadBinaryFileExact(void) {
  uint8_t binary_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0xFF};
  size_t len = sizeof(binary_data);
  create_test_file(binary_data, len);

  file_t result = get_file_contents(TEMP_FILE_PATH);

  TEST_ASSERT_NOT_NULL(result.data);
  TEST_ASSERT_EQUAL_UINT64(len, result.length);
  TEST_ASSERT_EQUAL_MEMORY(binary_data, result.data, len);

  free(result.data);
}

// 2. Edge Case: Empty File
// On Linux, creating a file with `touch` results in 0 bytes.
void test_GetFileContents_Should_HandleEmptyFile(void) {
  create_test_file("", 0); // Effectively a `touch`

  file_t result = get_file_contents(TEMP_FILE_PATH);

  TEST_ASSERT_EQUAL_UINT64(0, result.length);
  // Implementation choice: return NULL or malloc(0).
  // Usually NULL is preferred for 0 length to avoid malloc overhead.
  if (result.data)
    free(result.data);
}

// 3. Error Case: File Missing
void test_GetFileContents_Should_ReturnZero_When_FileDoesNotExist(void) {
  file_t result = get_file_contents(NON_EXISTENT_FILE);
  TEST_ASSERT_NULL(result.data);
  TEST_ASSERT_EQUAL_UINT64(0, result.length);
}

// 4. Linux Specific: Directory Handling
// Opening a directory with `fopen` on Linux might succeed, but `fread` will
// fail or behave unexpectedly. The function should define check `S_ISDIR`.
void test_GetFileContents_Should_Fail_When_PathIsDirectory(void) {
  mkdir(TEMP_DIR_PATH, 0777);

  file_t result = get_file_contents(TEMP_DIR_PATH);

  TEST_ASSERT_NULL_MESSAGE(result.data,
                           "Should fail (return NULL) if path is a directory");
  TEST_ASSERT_EQUAL_UINT64(0, result.length);
}

// 5. Linux Specific: Symlinks
// The function should resolve the link to the actual file.
void test_GetFileContents_Should_FollowSymlinks(void) {
  const char *content = "Symlink Target";
  create_test_file(content, strlen(content));

  // Create symlink: link -> target
  symlink(TEMP_FILE_PATH, TEMP_LINK_PATH);

  file_t result = get_file_contents(TEMP_LINK_PATH);

  TEST_ASSERT_NOT_NULL(result.data);
  TEST_ASSERT_EQUAL_UINT64(strlen(content), result.length);
  TEST_ASSERT_EQUAL_MEMORY(content, result.data, result.length);

  free(result.data);
}

// 6. Linux Specific: Broken Symlink
// A link pointing to a non-existent file. Should fail like a missing file.
void test_GetFileContents_Should_Fail_When_SymlinkIsBroken(void) {
  symlink("non_existent_target.bin", TEMP_LINK_PATH);

  file_t result = get_file_contents(TEMP_LINK_PATH);

  TEST_ASSERT_NULL(result.data);
  TEST_ASSERT_EQUAL_UINT64(0, result.length);
}

// 7. Linux Specific: Permissions
// Ensure we handle EACCES (Permission Denied).
void test_GetFileContents_Should_Fail_When_PermissionDenied(void) {
  // Root bypasses permissions on Linux
  if (getuid() == 0) {
    TEST_IGNORE_MESSAGE("Skipping permission test: Running as root");
  }

  create_test_file("Secret", 6);
  chmod(TEMP_FILE_PATH, 0000); // chmod 000

  file_t result = get_file_contents(TEMP_FILE_PATH);

  TEST_ASSERT_NULL_MESSAGE(result.data,
                           "Should return NULL when permission is denied");

  chmod(TEMP_FILE_PATH, 0777); // Restore for cleanup
}

// 8. Linux Specific: Character Devices (/dev/null)
// /dev/null is readable but has 0 size.
void test_GetFileContents_Should_HandleDevNull(void) {
  file_t result = get_file_contents("/dev/null");

  TEST_ASSERT_EQUAL_UINT64(0, result.length);
  if (result.data)
    free(result.data);
}

// 9. Input Validation: NULL inputs
void test_GetFileContents_Should_SafeGuardAgainstNullPath(void) {
  file_t result = get_file_contents(NULL);
  TEST_ASSERT_NULL(result.data);
  TEST_ASSERT_EQUAL_UINT64(0, result.length);
}

/* --- Main Runner --- */

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_GetFileContents_Should_ReadBinaryFileExact);
  RUN_TEST(test_GetFileContents_Should_HandleEmptyFile);
  RUN_TEST(test_GetFileContents_Should_ReturnZero_When_FileDoesNotExist);
  RUN_TEST(test_GetFileContents_Should_SafeGuardAgainstNullPath);

  // Linux Specific Scenarios
  RUN_TEST(test_GetFileContents_Should_Fail_When_PathIsDirectory);
  RUN_TEST(test_GetFileContents_Should_FollowSymlinks);
  RUN_TEST(test_GetFileContents_Should_Fail_When_SymlinkIsBroken);
  RUN_TEST(test_GetFileContents_Should_Fail_When_PermissionDenied);
  RUN_TEST(test_GetFileContents_Should_HandleDevNull);

  return UNITY_END();
}
