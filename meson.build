project('elma', 'cpp', version: '1.4', meson_version: '>=1.2.0', default_options: ['cpp_std=c++20'])

add_project_arguments('-DELMA_VERSION="@0@"'.format(meson.project_version()), language: 'cpp')

if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
    add_project_arguments('-DDEBUG', language: 'cpp')
endif

elma_sources = files(
    'abc8.cpp',
    'physics_init.cpp',
    'anim.cpp',
    'physics_move.cpp',
    'best_times.cpp',
    'menu_controls.cpp',
    'editor_dialog.cpp',
    'menu_dialog.cpp',
    'timer.cpp',
    'editor_canvas.cpp',
    'DOBOZ.CPP',
    'D_PIC.CPP',
    'ECSET.CPP',
    'EDITHELP.CPP',
    'EDITMENU.CPP',
    'EDITPLAY.CPP',
    'EDITTOLT.CPP',
    'EDITTOOL.CPP',
    'EDITUJ.CPP',
    'ED_CHECK.CPP',
    'flagtag.cpp',
    'ball_handler.cpp',
    'ball.cpp',
    'ball_collision.cpp',
    'polygon.cpp',
    'H_WAV.CPP',
    'JATEKOS.CPP',
    'KIRAJ320.CPP',
    'affine_pic.cpp',
    'grass.cpp',
    'platform_utils.cpp',
    'LEJATSZO.CPP',
    'LEPTET.CPP',
    'LGRFILE.CPP',
    'LOAD.CPP',
    'main.cpp',
    'MAINMENU.CPP',
    'menu_pic.cpp',
    'menu_options.cpp',
    'PEXTMENU.CPP',
    'pic8.cpp',
    'piclist.cpp',
    'PLAY.CPP',
    'qopen.cpp',
    'RECORDER.CPP',
    'platform_sdl.cpp',
    'SKIP.CPP',
    'transparency.cpp',
    'state.cpp',
    'segments.cpp',
    'TELJES.CPP',
    'TOPOL.CPP',
    'UTKOZES.CPP',
    'UTKOZES2.CPP',
    'menu_nav.cpp',
    'vect2.cpp',
    'eol_settings.cpp',
    'hhigh.cpp',
    'keys.cpp',
    'fs_utils.cpp',
    'object.cpp',
)

cpp = meson.get_compiler('cpp')
link_args = []
cpp_args = []

# Required for nlohmann_json on Apple clang < 12
if cpp.get_id() == 'clang' and cpp.version().version_compare('<12.0')
    add_project_arguments('-DJSON_HAS_CPP_17', language: 'cpp')
endif

win_subsystem = 'windows'
if cpp.get_id() == 'msvc'
    if get_option('buildtype') == 'debug'
        # Allows printf when debugging
        win_subsystem = 'console'
    endif

    if win_subsystem == 'windows'
        # Override windows subsystem entrypoint to main instead of WinMain
        link_args += ['/ENTRY:mainCRTStartup']
    endif

    if host_machine.cpu_family() == 'x86'
        # winmm.lib is automatically linked by meson when 64-bit, but not 32-bit
        link_args += ['winmm.lib']
    endif
else
    cpp_args += ['-Wno-deprecated-declarations']
endif

sdl2_dep = dependency('sdl2',
    static: build_machine.system() == 'windows',
    default_options: {
        'test': false,
        'with_main': false,
    }
)

nlohmann_json_dep = dependency('nlohmann_json')

elma = executable(
    'elma',
    elma_sources,
    cpp_args: cpp_args,
    link_args: link_args,
    dependencies: [nlohmann_json_dep, sdl2_dep],
    win_subsystem: win_subsystem,
)
