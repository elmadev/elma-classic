project('elma', 'c', 'cpp', version: '1.4', meson_version: '>=1.2.0', default_options: ['cpp_std=c++20'])

add_project_arguments('-DELMA_VERSION="@0@"'.format(meson.project_version()), language: 'cpp')

if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
    add_project_arguments('-DDEBUG', language: 'cpp')
endif

if get_option('profile_performance') == true
    add_project_arguments('-DPROFILE_PERFORMANCE', language: 'cpp')
endif

if get_option('curl_verbose') == true
    add_project_arguments('-DCURL_VERBOSE', language: 'cpp')
endif

cpp = meson.get_compiler('cpp')

elma_sources = files(
    'src/debug/profiler.cpp',
    'src/editor/canvas.cpp',
    'src/editor/dialog.cpp',
    'src/editor/editor.cpp',
    'src/editor/help.cpp',
    'src/editor/screen_pic.cpp',
    'src/editor/tool.cpp',
    'src/editor/topology.cpp',
    'src/editor/window.cpp',
    'src/eol/battle.cpp',
    'src/eol/checkpoint.cpp',
    'src/eol/clickable.cpp',
    'src/eol/console.cpp',
    'src/eol/eol_table.cpp',
    'src/eol/eol.cpp',
    'src/eol/kuski.cpp',
    'src/eol/settings.cpp',
    'src/eol/status_messages.cpp',
    'src/game/camera.cpp',
    'src/game/driver.cpp',
    'src/game/fps.cpp',
    'src/game/game.cpp',
    'src/game/input.cpp',
    'src/game/level_load.cpp',
    'src/game/qopen.cpp',
    'src/game/recorder.cpp',
    'src/game/state.cpp',
    'src/glad/glad.c',
    'src/level/level.cpp',
    'src/level/object.cpp',
    'src/level/polygon.cpp',
    'src/level/segments.cpp',
    'src/level/sprite.cpp',
    'src/log.cpp',
    'src/main.cpp',
    'src/menu/ball_collision.cpp',
    'src/menu/ball_handler.cpp',
    'src/menu/ball.cpp',
    'src/menu/best_times.cpp',
    'src/menu/controls.cpp',
    'src/menu/dialog.cpp',
    'src/menu/external.cpp',
    'src/menu/intro.cpp',
    'src/menu/main.cpp',
    'src/menu/nav.cpp',
    'src/menu/options.cpp',
    'src/menu/pic.cpp',
    'src/menu/play.cpp',
    'src/menu/player.cpp',
    'src/menu/rec_list.cpp',
    'src/menu/replay_cache.cpp',
    'src/menu/replay.cpp',
    'src/menu/skip.cpp',
    'src/physics/collision.cpp',
    'src/physics/flagtag.cpp',
    'src/physics/forces.cpp',
    'src/physics/init.cpp',
    'src/physics/move.cpp',
    'src/physics/pacer.cpp',
    'src/pic/abc8.cpp',
    'src/pic/anim.cpp',
    'src/pic/lgr.cpp',
    'src/pic/pic8.cpp',
    'src/pic/piclist.cpp',
    'src/pic/surface.cpp',
    'src/platform/sdl/gl_renderer.cpp',
    'src/platform/sdl/keyboard.cpp',
    'src/platform/sdl/scancode.cpp',
    'src/platform/sdl/sdl.cpp',
    'src/platform/text_input.cpp',
    'src/platform/utils.cpp',
    'src/renderer/affine.cpp',
    'src/renderer/canvas.cpp',
    'src/renderer/grass.cpp',
    'src/renderer/object_overlay.cpp',
    'src/renderer/render.cpp',
    'src/renderer/timer.cpp',
    'src/sound/engine.cpp',
    'src/sound/wav.cpp',
    'src/util/file_iter.cpp',
    'src/util/util.cpp',
    'src/vect2.cpp',
)

if cpp.get_id() != 'emscripten'
    elma_sources += files(
        'src/platform/native/api/api.cpp',
        'src/platform/native/api/wrapper.cpp',
    )
else
    elma_sources += files(
        'src/platform/emscripten/api/api.cpp',
    )
endif

inc = include_directories('include')

link_args = []
cpp_args = []
deps = []

# Required for nlohmann_json on Apple clang < 12
if cpp.get_id() == 'clang' and cpp.version().version_compare('<12.0')
    add_project_arguments('-DJSON_HAS_CPP_17', language: 'cpp')
endif

win_subsystem = 'windows'
if cpp.get_id() == 'msvc'
    # Conforming preprocessor required for __VA_OPT__
    cpp_args += ['/Zc:preprocessor']

    if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized' or get_option('profile_performance') == true
        # Allows printf when debugging
        win_subsystem = 'console'
    endif

    if win_subsystem == 'windows'
        # Override windows subsystem entrypoint to main instead of WinMain
        link_args += ['/ENTRY:mainCRTStartup']
    endif
elif cpp.get_id() == 'clang'
    cpp_args += ['-Wno-deprecated-declarations', '-Wunreachable-code']
elif cpp.get_id() == 'emscripten'
    cpp_args += ['-sUSE_SDL=2']
    link_args += ['-sUSE_SDL=2', '-ooutput.html', '--emrun', '--embed-file','../data@/', '-sENVIRONMENT=web', '-sALLOW_MEMORY_GROWTH', '-sASYNCIFY', '-sASYNCIFY_STACK_SIZE=16384', '-sDISABLE_EXCEPTION_CATCHING=0']
    if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
        cpp_args += ['-g']
        link_args += ['-sEXCEPTION_DEBUG', '-g']
    else
        cpp_args += ['-O3', '-flto']
        link_args += ['-flto', '-g2']
    endif
endif

gl_dep = dependency('gl', required: true)

libcurl_dep = dependency('libcurl',
    version : '>=8.12.1',
    include_type: 'system',
    static: host_machine.system() == 'windows',
    required: cpp.get_id() != 'emscripten',
    default_options: {
        # See meson_options.txt, meson.build, CURL-DISABLE.md and INSTALL-CMAKE.md in libcurl
        # See also https://curl.se/libcurl/c/curl_version_info.html

        # In order of meson_options.txt:

        # Build
        'tests': 'disabled',
        'unittests': 'disabled',
        'tool': 'disabled',

        # CA
        'ca_bundle': 'auto', # Linux
        'ca_path': 'auto', # Linux
        'ca_fallback': false,

        # Features
        'bindlocal': 'disabled', # CURLOPT_INTERFACE
        'brotli': 'disabled', # CURLOPT_ACCEPT_ENCODING
        'cookies': 'enabled',
        'doh': 'disabled', # CURLOPT_DOH_URL
        'form-api': 'disabled', # deprecated curl_formadd()/curl_formget()
        'getoptions': 'disabled', # curl_easy_option_by_name()/curl_easy_option_by_id()
        'gsasl': 'disabled', # IMAP/SMTP auth - https://gitlab.com/gsasl/gsasl
        'http2': 'auto', # missing .wrap file on Windows https://github.com/nghttp2/nghttp2
        'ipv6': 'disabled',
        'libcurl-option': 'disabled', # curl --libcurl file.c https://example.com
        'libz': 'disabled', # CURLOPT_ACCEPT_ENCODING
        'mime': 'enabled', # Mime Headers
        'netrc': 'disabled', # CURLOPT_NETRC
        'parsedate': 'disabled', # curl_getdate()
        'progress-meter': 'disabled', # command-line progress meter
        'proxy': 'disabled', # CURLOPT_PROXY
        'psl': 'disabled', # https://publicsuffix.org/, https://curl.se/libcurl/security.html
        'sha512_256': 'disabled', # only used in CURLAUTH_DIGEST
        'shuffle-dns': 'disabled', # CURLOPT_DNS_SHUFFLE_ADDRESSES
        'socketpair': 'enabled', # curl_multi_poll()
        'sspi': host_machine.system() == 'windows' ? 'enabled' : 'disabled', # required for Schannel (SSL)
        'unixsockets': 'disabled', # CURLOPT_UNIX_SOCKET_PATH
        'verbose-strings': 'enabled', # Error messages
        'websockets': 'disabled', # API: Websocket
        'zstd': 'disabled', # CURLOPT_ACCEPT_ENCODING

        # Asynchronous DNS
        'asynchdns': 'enabled', # faster multi interface

        # Authentication
        'aws': 'disabled',
        'basic-auth': 'disabled',
        'bearer-auth': 'disabled',
        'digest-auth': 'disabled',
        'kerberos-auth': 'disabled',
        'negotiate-auth': 'disabled',

        # GSS API
        'gss-api': 'disabled',

        # IDN
        'idn': 'disabled', # Internationalized domain name

        # HTTP
        'alt-svc': 'disabled', # elma.online provides HTTP3 via alt-svc, but HTTP3 is not currently supported in 8.12.1 via meson.build
        'headers-api': 'enabled', # curl_easy_header()
        'hsts': 'disabled', # CURLOPT_HSTS_CTRL
        'http-auth': 'disabled', # CURLOPT_HTTPAUTH

        # NTLM
        'ntlm': 'disabled',

        # SSH
        'ssh': 'disabled',

        # SSL
        'ssl': 'enabled',
        'tls-srp': 'disabled', # deprecated CURLOPT_TLSAUTH_TYPE

        # Protocols
        'dict': 'disabled',
        'file': 'disabled',
        'ftp': 'disabled',
        'gopher': 'disabled',
        'http': 'enabled',
        'imap': 'disabled',
        'ipfs': 'disabled',
        'ldap': 'disabled',
        'ldaps': 'disabled',
        'mqtt': 'disabled',
        'pop3': 'disabled',
        'rtmp': 'disabled',
        'rtsp': 'disabled',
        'smb': 'disabled',
        'smtp': 'disabled',
        'telnet': 'disabled',
        'tftp': 'disabled',
    }
)

nlohmann_json_dep = dependency('nlohmann_json')

sdl2_dep = dependency('sdl2',
    static: host_machine.system() == 'windows' or get_option('static_sdl'),
    required: cpp.get_id() != 'emscripten',
    default_options: {
        # See this overview: https://wiki.libsdl.org/SDL3/QuickReference
        # Also see SDL's meson_options.txt
        'test': false,
        'with_main': false,

        # Uninitialized subsystems (https://wiki.libsdl.org/SDL2/SDL_Init)
        'use_sensor': 'disabled',
        'use_haptic': 'disabled',
        'use_joystick': 'disabled',
        'use_joystick_dinput': 'disabled',
        'use_joystick_virtual': 'disabled',
        'use_joystick_wgi': 'disabled',
        'use_joystick_xinput': 'disabled',

        # Unused hidden subsystems (#define SDL_SUBSYSTEM_DISABLED)
        'use_cpuinfo': 'disabled', # https://wiki.libsdl.org/SDL2/CategoryCPUInfo
        'use_filesystem': 'disabled', # https://wiki.libsdl.org/SDL2/CategoryFilesystem
        'use_hidapi': 'disabled', # Bluetooth, https://wiki.libsdl.org/SDL2/CategoryHIDAPI
        'use_locale': 'disabled', # https://wiki.libsdl.org/SDL2/CategoryLocale
        'use_power': 'disabled', # https://wiki.libsdl.org/SDL2/CategoryPower
    }
)

deps += [gl_dep, libcurl_dep, nlohmann_json_dep, sdl2_dep]

executable(
    'elma',
    elma_sources,
    include_directories: inc,
    cpp_args: cpp_args,
    link_args: link_args,
    dependencies: deps,
    win_subsystem: win_subsystem,
)

clang_tidy = find_program('clang-tidy', 'clang-tidy-21', required: false)
lint_files = run_command('git', '-C', meson.project_source_root(), 'ls-files', '*.cpp', '*.c', 'include/*.h', check: false)
if clang_tidy.found() and lint_files.returncode() == 0
    filtered_files = []
    foreach f : lint_files.stdout().strip().split('\n')
        if not f.contains('emscripten')
            filtered_files += f
        endif
    endforeach

    clang_tidy_args = ['--allow-no-checks', '--warnings-as-errors=*', '-quiet', '-p', meson.project_build_root()]

    # clang-cl rejects the debug CRT together with -fsanitize=address, so
    # analyze with the release CRT (last /M flag wins). Appending extra args
    # also leaves /c unused, which must not count as a finding.
    if cpp.get_id() in ['msvc', 'clang-cl']
        clang_tidy_args += ['--extra-arg=/MD', '--extra-arg=-Qunused-arguments']
    endif

    # Homebrew clang-tidy doesn't know where the Xcode SDK lives
    if host_machine.system() == 'darwin'
        sdk_path = run_command('xcrun', '--show-sdk-path', check: false)
        if sdk_path.returncode() == 0
            clang_tidy_args += '--extra-arg=-isysroot' + sdk_path.stdout().strip()
        endif
    endif

    foreach f : filtered_files
        test(
            f,
            clang_tidy,
            args: clang_tidy_args + [meson.project_source_root() / f],
            suite: 'clang-tidy',
            timeout: 90,
        )
    endforeach
endif
