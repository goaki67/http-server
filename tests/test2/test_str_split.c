#include "string_utils.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

/* --- Helper Functions --- */

int count_tokens(char **tokens) {
  if (!tokens)
    return 0;
  int i = 0;
  while (tokens[i] != NULL)
    i++;
  return i;
}

/* --- Setup / Teardown --- */

void setUp(void) {
  // No specific setup needed
}

void tearDown(void) {
  // No specific teardown needed
}

/* ========================================================================= */
/* TESTS: str_split                             */
/* ========================================================================= */

// 1. Happy Path: Normal split
void test_StrSplit_Should_SplitNormally(void) {
  char input[] = "header,body,footer"; // Mutable copy
  char **tokens = str_split(input, ',');

  TEST_ASSERT_NOT_NULL(tokens);
  TEST_ASSERT_EQUAL_STRING("header", tokens[0]);
  TEST_ASSERT_EQUAL_STRING("body", tokens[1]);
  TEST_ASSERT_EQUAL_STRING("footer", tokens[2]);
  TEST_ASSERT_NULL(tokens[3]); // Null terminator

  free(tokens);
}

// 2. Edge Case: Empty String
// Expect: [""] (One token, which is empty)
void test_StrSplit_Should_HandleEmptyString(void) {
  char input[] = "";
  char **tokens = str_split(input, ',');

  TEST_ASSERT_NOT_NULL(tokens);
  TEST_ASSERT_EQUAL_STRING("", tokens[0]);
  TEST_ASSERT_NULL(tokens[1]);

  free(tokens);
}

// 3. Edge Case: Consecutive Delimiters
// Expect: "a,,b" -> ["a", "", "b"]
void test_StrSplit_Should_HandleConsecutiveDelimiters(void) {
  char input[] = "key,,value";
  char **tokens = str_split(input, ',');

  TEST_ASSERT_NOT_NULL(tokens);
  TEST_ASSERT_EQUAL_STRING("key", tokens[0]);
  TEST_ASSERT_EQUAL_STRING("", tokens[1]);
  TEST_ASSERT_EQUAL_STRING("value", tokens[2]);
  TEST_ASSERT_NULL(tokens[3]);

  free(tokens);
}

// 4. Edge Case: String Only Contains Delimiters
// Expect: ",," -> ["", "", ""]
void test_StrSplit_Should_HandleOnlyDelimiters(void) {
  char input[] = ",,";
  char **tokens = str_split(input, ',');

  TEST_ASSERT_NOT_NULL(tokens);
  TEST_ASSERT_EQUAL_INT(3, count_tokens(tokens));
  TEST_ASSERT_EQUAL_STRING("", tokens[0]);
  TEST_ASSERT_EQUAL_STRING("", tokens[1]);
  TEST_ASSERT_EQUAL_STRING("", tokens[2]);

  free(tokens);
}

// 5. Input Safety: NULL Input
void test_StrSplit_Should_ReturnNull_When_InputIsNull(void) {
  char **tokens = str_split(NULL, ',');
  TEST_ASSERT_NULL_MESSAGE(tokens,
                           "Should return NULL if input string is NULL");
}

// 6. Input Safety: Null Delimiter
// Standard str_split behavior typically requires a valid char.
// If passing '\0', it should fail (return NULL) or return original string
// depending on spec. Based on previous prompt: Return NULL and warn.
void test_StrSplit_Should_Fail_When_DelimiterIsNullChar(void) {
  char input[] = "text";
  char **tokens = str_split(input, '\0');
  TEST_ASSERT_NULL_MESSAGE(tokens,
                           "Should return NULL for invalid delimiter \\0");
}

/* ========================================================================= */
/* TESTS: get_line                              */
/* ========================================================================= */

// 1. Happy Path: Retrieve valid indexes
void test_GetLine_Should_ReturnString_AtValidIndex(void) {
  // Manually construct a token array to test get_line in isolation
  char *mock_tokens[] = {"Line 1", "Line 2", "Line 3", NULL};

  TEST_ASSERT_EQUAL_STRING("Line 1", get_line(mock_tokens, 0));
  TEST_ASSERT_EQUAL_STRING("Line 2", get_line(mock_tokens, 1));
  TEST_ASSERT_EQUAL_STRING("Line 3", get_line(mock_tokens, 2));
}

// 2. Edge Case: Index Out of Bounds
// The function should detect the NULL terminator in the array and return NULL
// if the requested index is beyond it.
void test_GetLine_Should_ReturnNull_When_IndexIsOutOfBounds(void) {
  char *mock_tokens[] = {"One", "Two", NULL};

  // Index 2 is the NULL terminator itself -> Should return NULL
  TEST_ASSERT_NULL(get_line(mock_tokens, 2));

  // Index 10 is way out of bounds -> Should return NULL
  TEST_ASSERT_NULL(get_line(mock_tokens, 10));
}

// 3. Input Safety: Tokens Array is NULL
void test_GetLine_Should_ReturnNull_When_TokensArrayIsNull(void) {
  TEST_ASSERT_NULL_MESSAGE(get_line(NULL, 0),
                           "Should handle NULL input array gracefully");
}

// 4. Integration Test: Combine str_split and get_line
void test_Integration_StrSplit_And_GetLine(void) {
  char input[] = "User:Pass:Uid";
  char **tokens = str_split(input, ':');

  TEST_ASSERT_NOT_NULL(tokens);

  // Verify retrieval
  TEST_ASSERT_EQUAL_STRING("User", get_line(tokens, 0));
  TEST_ASSERT_EQUAL_STRING("Pass", get_line(tokens, 1));
  TEST_ASSERT_EQUAL_STRING("Uid", get_line(tokens, 2));

  // Verify OOB
  TEST_ASSERT_NULL(get_line(tokens, 3));

  free(tokens);
}

/* --- Main Runner --- */

int main(void) {
  UNITY_BEGIN();

  // str_split Tests
  RUN_TEST(test_StrSplit_Should_SplitNormally);
  RUN_TEST(test_StrSplit_Should_HandleEmptyString);
  RUN_TEST(test_StrSplit_Should_HandleConsecutiveDelimiters);
  RUN_TEST(test_StrSplit_Should_HandleOnlyDelimiters);
  RUN_TEST(test_StrSplit_Should_ReturnNull_When_InputIsNull);
  RUN_TEST(test_StrSplit_Should_Fail_When_DelimiterIsNullChar);

  // get_line Tests
  RUN_TEST(test_GetLine_Should_ReturnString_AtValidIndex);
  RUN_TEST(test_GetLine_Should_ReturnNull_When_IndexIsOutOfBounds);
  RUN_TEST(test_GetLine_Should_ReturnNull_When_TokensArrayIsNull);
  RUN_TEST(test_Integration_StrSplit_And_GetLine);

  return UNITY_END();
}
