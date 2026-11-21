#include "api/api.h"
#include "eol/eol.h"
#include "menu/intro.h"
#include "platform/implementation.h"
#include "runtime.h"
#include <filesystem>

int main() {
    std::filesystem::create_directory("lev");
    std::filesystem::create_directory("rec");

    runtime::init_settings();
    platform_init();
    runtime::init_data();

    EolClient->connect();
    eol_api::init();

    menu_intro();
}
