#ifndef TEST_TESTS_H
#define TEST_TESTS_H

#include <optional>
#include <string>

inline std::optional<std::string> ExpectedError;

// Program immediately exits upon calling internal_error since it is a [[no_return]] function
#define THROWS_ERROR(call, error)                                                                  \
    do {                                                                                           \
        ExpectedError = error;                                                                     \
        call;                                                                                      \
        ExpectedError.reset();                                                                     \
        internal_error("Test did not terminate with an internal error!");                          \
    } while (0);

#endif
