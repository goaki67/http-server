#include "string_utils.h"
#include "unity.h"
#include <stdbool.h>
#include <stddef.h>

/* --- Setup/Teardown --- */
void setUp(void) {}
void tearDown(void) {}

/* ========================================================================= */
/* BASIC FUNCTIONALITY                                                       */
/* ========================================================================= */

// 1. Exact Match at the start
void test_StartsWith_Should_ReturnTrue_When_PrefixMatches(void) {
  TEST_ASSERT_TRUE(str_starts_with("Hello World", "Hello"));
  TEST_ASSERT_TRUE(str_starts_with("Integration Testing", "Integration"));
}

// 2. Mismatch
void test_StartsWith_Should_ReturnFalse_When_PrefixMismatch(void) {
  TEST_ASSERT_FALSE(str_starts_with("Hello World", "Bye"));
}

// 3. Match exists but not at the start (Substring check)
// Ensure we are checking "starts with", not "contains".
void test_StartsWith_Should_ReturnFalse_When_PrefixIsOnlySubstring(void) {
  TEST_ASSERT_FALSE(str_starts_with("Hello World", "World"));
  TEST_ASSERT_FALSE(str_starts_with("Documentation", "cument"));
}

// 4. Case Sensitivity (Standard C behavior is case-sensitive)
void test_StartsWith_Should_BeCaseSensitive(void) {
  TEST_ASSERT_FALSE(str_starts_with("Hello", "hello"));
}

// 5. Exact Identity (Whole string matches prefix)
void test_StartsWith_Should_ReturnTrue_When_PrefixEqualsString(void) {
  TEST_ASSERT_TRUE(str_starts_with("Exact", "Exact"));
}

/* ========================================================================= */
/* LENGTH EDGE CASES                                                         */
/* ========================================================================= */

// 6. Prefix is longer than the string
// Logic: A short string cannot start with a longer string.
void test_StartsWith_Should_ReturnFalse_When_PrefixLongerThanString(void) {
  TEST_ASSERT_FALSE(str_starts_with("Hi", "Hello"));
}

// 7. Empty Prefix
// Logic: Mathematically/Logically, every string starts with the empty set.
// This is the standard behavior for most languages (Python, JS, C++).
void test_StartsWith_Should_ReturnTrue_When_PrefixIsEmpty(void) {
  TEST_ASSERT_TRUE(str_starts_with("Anything", ""));
}

// 8. Empty String
void test_StartsWith_Should_ReturnFalse_When_StringEmpty_And_PrefixNot(void) {
  TEST_ASSERT_FALSE(str_starts_with("", "a"));
}

// 9. Both Empty
void test_StartsWith_Should_ReturnTrue_When_BothAreEmpty(void) {
  TEST_ASSERT_TRUE(str_starts_with("", ""));
}

/* ========================================================================= */
/* SAFETY CHECKS                                                             */
/* ========================================================================= */

// 10. NULL Handling
// A safe implementation should return false rather than crashing.
void test_StartsWith_Should_ReturnFalse_When_InputsAreNull(void) {
  TEST_ASSERT_FALSE(str_starts_with(NULL, "prefix"));
  TEST_ASSERT_FALSE(str_starts_with("string", NULL));
  TEST_ASSERT_FALSE(str_starts_with(NULL, NULL));
}

/* --- Main Runner --- */

int main(void) {
  UNITY_BEGIN();

  // Functional
  RUN_TEST(test_StartsWith_Should_ReturnTrue_When_PrefixMatches);
  RUN_TEST(test_StartsWith_Should_ReturnFalse_When_PrefixMismatch);
  RUN_TEST(test_StartsWith_Should_ReturnFalse_When_PrefixIsOnlySubstring);
  RUN_TEST(test_StartsWith_Should_BeCaseSensitive);
  RUN_TEST(test_StartsWith_Should_ReturnTrue_When_PrefixEqualsString);

  // Lengths
  RUN_TEST(test_StartsWith_Should_ReturnFalse_When_PrefixLongerThanString);
  RUN_TEST(test_StartsWith_Should_ReturnTrue_When_PrefixIsEmpty);
  RUN_TEST(test_StartsWith_Should_ReturnFalse_When_StringEmpty_And_PrefixNot);
  RUN_TEST(test_StartsWith_Should_ReturnTrue_When_BothAreEmpty);

  // Safety
  RUN_TEST(test_StartsWith_Should_ReturnFalse_When_InputsAreNull);

  return UNITY_END();
}
