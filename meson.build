project('elma', 'cpp', version: '1.4', meson_version: '>=1.2.0', default_options: ['cpp_std=c++20'])

add_project_arguments('-DELMA_VERSION="@0@"'.format(meson.project_version()), language: 'cpp')

if get_option('buildtype') == 'debug'
    add_project_arguments('-DDEBUG', language: 'cpp')
endif

elma_sources = files(
    'abc8.cpp',
    'physics_init.cpp',
    'anim.cpp',
    'physics_move.cpp',
    'best_times.cpp',
    'CUSTOM.CPP',
    'DIALOG.CPP',
    'DIALOG32.CPP',
    'DIGIT.CPP',
    'DISPLAY.CPP',
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
    'FOGOCSKA.CPP',
    'ball_handler.cpp',
    'ball.cpp',
    'ball_collision.cpp',
    'GYURU.CPP',
    'H_WAV.CPP',
    'JATEKOS.CPP',
    'KIRAJ320.CPP',
    'KISKEP.CPP',
    'KOVETO.CPP',
    'KOZOS.CPP',
    'LEJATSZO.CPP',
    'LEPTET.CPP',
    'LGRFILE.CPP',
    'LOAD.CPP',
    'MAIN.CPP',
    'MAINMENU.CPP',
    'MENUKEP.CPP',
    'OPTIONS.CPP',
    'PEXTMENU.CPP',
    'pic8.cpp',
    'PICLIST.CPP',
    'PLAY.CPP',
    'QOPEN.CPP',
    'RECORDER.CPP',
    'SDL.CPP',
    'SKIP.CPP',
    'SPRITE.CPP',
    'state.cpp',
    'SZAKASZ.CPP',
    'TELJES.CPP',
    'TOPOL.CPP',
    'UTKOZES.CPP',
    'UTKOZES2.CPP',
    'VALASZT2.CPP',
    'VEKT2.CPP',
    'hhigh.cpp',
    'input.cpp',
)

link_args = []
win_subsystem = 'console'
needs_stub = true

cpp = meson.get_compiler('cpp')
if cpp.get_id() == 'msvc'
    cpp_args = []
    if host_machine.cpu_family() == 'x86'
        win_subsystem = 'windows'
        needs_stub = false
        elma_sources += files(
            'F_WIN/W_V.CPP',
            'dxinc/getdxver.CPP',
        )
        link_args += [
            '/ENTRY:mainCRTStartup',
            'winmm.lib',
        ]
    endif
else
    cpp_args = ['-Wno-deprecated-declarations']
endif

if needs_stub
    elma_sources += files('stub.cpp')
endif

sdl2_dep = dependency('sdl2',
    static: build_machine.system() == 'windows',
    default_options: {
        'test': false,
        'with_main': false,
    }
)

elma = executable(
    'elma',
    elma_sources,
    cpp_args: cpp_args,
    link_args: link_args,
    dependencies: [sdl2_dep],
    win_subsystem: win_subsystem,
)
