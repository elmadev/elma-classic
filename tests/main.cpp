#include "main.h"
#include "api/api.h"
#include "log.h"
#include "tests.h"
#include <cstring>

void quit() {
    eol_api::cleanup();
    exit(0);
}

// Program immediately exits upon calling internal_error since it is a [[no_return]] function
void internal_error(const std::string& message, std::source_location loc) {
    if (ExpectedError && *ExpectedError == message) {
        exit(0);
    }

    logger::instance().write(LogLevel::Fatal, loc,
                             std::format("Sorry, internal error. {}", message));
    exit(1);
}

#define TEST(name)                                                                                 \
    if (strcmp(#name, argv[1]) == 0) {                                                             \
        extern void name();                                                                        \
        name();                                                                                    \
        exit(0);                                                                                   \
    }

int main(int argc, char* argv[]) {
    if (argc < 2) {
        LOG_FATAL("Invalid commandline");
        exit(2);
    }

    LOG_FATAL("Test not found: {}", argv[1]);
    exit(2);
}
