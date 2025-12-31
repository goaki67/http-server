#include "file.h"
#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// Include unistd.h for symlink() on POSIX systems (Linux/macOS)
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
#define IS_POSIX 1
#endif

/* --- Constants --- */
#define TEMP_FILE_PATH "test_temp_file.txt"
#define TEMP_LINK_PATH "test_symlink.txt"
#define NON_EXISTENT_FILE "this_file_does_not_exist.txt"

/* --- Helper Functions --- */

void create_temp_file(const char *content) {
  FILE *fp = fopen(TEMP_FILE_PATH, "w");
  if (fp) {
    if (content) {
      fputs(content, fp);
    }
    fclose(fp);
  }
}

void create_binary_temp_file(const char *data, size_t len) {
  FILE *fp = fopen(TEMP_FILE_PATH, "wb");
  if (fp) {
    fwrite(data, 1, len, fp);
    fclose(fp);
  }
}

void remove_artifacts() {
  remove(TEMP_FILE_PATH);
  remove(TEMP_LINK_PATH); // Clean up symlink if it exists
}

/* --- Unity Setup and Teardown --- */

void setUp(void) { remove_artifacts(); }

void tearDown(void) { remove_artifacts(); }

/* --- Standard Tests --- */

void test_GetFileContents_Should_ReturnContents_When_FileExists(void) {
  const char *expected_content = "Hello, Unity!";
  create_temp_file(expected_content);

  char *actual_content = get_file_contents(TEMP_FILE_PATH);

  TEST_ASSERT_NOT_NULL(actual_content);
  TEST_ASSERT_EQUAL_STRING(expected_content, actual_content);
  free(actual_content);
}

void test_GetFileContents_Should_ReturnEmptyString_When_FileIsEmpty(void) {
  create_temp_file("");
  char *actual_content = get_file_contents(TEMP_FILE_PATH);

  TEST_ASSERT_NOT_NULL(actual_content);
  TEST_ASSERT_EQUAL_STRING("", actual_content);
  free(actual_content);
}

void test_GetFileContents_Should_ReturnNull_When_FileDoesNotExist(void) {
  remove(NON_EXISTENT_FILE); // Paranoia
  char *actual_content = get_file_contents(NON_EXISTENT_FILE);
  TEST_ASSERT_NULL_MESSAGE(actual_content,
                           "Should return NULL for missing file");
}

// 1. Input Validation: NULL Path
void test_GetFileContents_Should_ReturnNull_When_InputPathIsNull(void) {
  char *actual_content = get_file_contents(NULL);
  TEST_ASSERT_NULL_MESSAGE(actual_content,
                           "Should return NULL safely for NULL input");
}

// 2. Input Validation: Path is a Directory
// The function should detect that this is a folder, not a file, and fail
// gracefully.
void test_GetFileContents_Should_ReturnNull_When_PathIsDirectory(void) {
  // We use the current directory "."
  char *actual_content = get_file_contents(".");
  TEST_ASSERT_NULL_MESSAGE(actual_content,
                           "Should return NULL if path is a directory");
}

// 3. Binary Files: Embedded Nulls
// Ensures the function reads the WHOLE file, even if a null byte is in the
// middle.
void test_GetFileContents_Should_ReadFullContent_EvenWithEmbeddedNulls(void) {
  // Data: "A" + \0 + "B"
  char binary_data[] = {'A', '\0', 'B'};
  create_binary_temp_file(binary_data, 3);

  char *actual_content = get_file_contents(TEMP_FILE_PATH);

  TEST_ASSERT_NOT_NULL(actual_content);

  // Validate first char
  TEST_ASSERT_EQUAL_CHAR('A', actual_content[0]);
  // Validate the null byte is actually there in memory
  TEST_ASSERT_EQUAL_CHAR('\0', actual_content[1]);
  // Validate the char AFTER the null byte is loaded
  TEST_ASSERT_EQUAL_CHAR('B', actual_content[2]);

  // Optional: Ensure the buffer is explicitly null-terminated at the END as
  // well logic: 3 bytes read -> index 3 should be \0
  TEST_ASSERT_EQUAL_CHAR('\0', actual_content[3]);

  free(actual_content);
}

// 4. Permission Denied (POSIX specific)
#ifdef IS_POSIX
void test_GetFileContents_Should_ReturnNull_When_PermissionDenied(void) {
  if (getuid() == 0) {
    TEST_IGNORE_MESSAGE(
        "Skipping permission test: Running as root ignores permissions");
  }

  create_temp_file("Secret");
  chmod(TEMP_FILE_PATH, 0000); // Remove all permissions

  char *actual_content = get_file_contents(TEMP_FILE_PATH);

  TEST_ASSERT_NULL_MESSAGE(actual_content,
                           "Should return NULL when unreadable");

  chmod(TEMP_FILE_PATH, 0777); // Restore so tearDown can delete it
}
#endif

// 5. Symbolic Links (POSIX specific)
#ifdef IS_POSIX
void test_GetFileContents_Should_FollowSymlinks(void) {
  const char *content = "Linked Content";
  create_temp_file(content);

  // Create symlink: TEMP_LINK_PATH points to TEMP_FILE_PATH
  if (symlink(TEMP_FILE_PATH, TEMP_LINK_PATH) != 0) {
    TEST_FAIL_MESSAGE("Failed to create symlink for test setup");
  }

  char *actual_content = get_file_contents(TEMP_LINK_PATH);

  TEST_ASSERT_NOT_NULL(actual_content);
  TEST_ASSERT_EQUAL_STRING(content, actual_content);

  free(actual_content);
}
#endif

/* --- Main Runner --- */

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_GetFileContents_Should_ReturnContents_When_FileExists);
  RUN_TEST(test_GetFileContents_Should_ReturnEmptyString_When_FileIsEmpty);
  RUN_TEST(test_GetFileContents_Should_ReturnNull_When_FileDoesNotExist);

  // New Tests
  RUN_TEST(test_GetFileContents_Should_ReturnNull_When_InputPathIsNull);
  RUN_TEST(test_GetFileContents_Should_ReturnNull_When_PathIsDirectory);
  RUN_TEST(test_GetFileContents_Should_ReadFullContent_EvenWithEmbeddedNulls);

#ifdef IS_POSIX
  RUN_TEST(test_GetFileContents_Should_ReturnNull_When_PermissionDenied);
  RUN_TEST(test_GetFileContents_Should_FollowSymlinks);
#endif

  return UNITY_END();
}
