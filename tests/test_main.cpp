#include "main.h"
#include "tests.h"

// Test the test suite itself
void test_main_internal_error() { THROWS_ERROR(internal_error("Test error"), "Test error"); }
