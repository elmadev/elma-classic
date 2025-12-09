project('elma', 'cpp', version: '1.4', default_options: ['cpp_std=c++20'])

elma_sources = files(
    'ABC8.CPP',
    'ADATOK.CPP',
    'ANIM.CPP',
    'BEALLIT.CPP',
    'BESTTIME.CPP',
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
    'GOLYKISZ.CPP',
    'GOLYOK.CPP',
    'GOLYUTK.CPP',
    'GYURU.CPP',
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
    'PIC8.CPP',
    'PICLIST.CPP',
    'PLAY.CPP',
    'QOPEN.CPP',
    'RECORDER.CPP',
    'SDL.CPP',
    'SKIP.CPP',
    'SPRITE.CPP',
    'STATE.CPP',
    'SZAKASZ.CPP',
    'TELJES.CPP',
    'TOPOL.CPP',
    'UTKOZES.CPP',
    'UTKOZES2.CPP',
    'VALASZT2.CPP',
    'VEKT2.CPP',
)

link_args = []
win_subsystem = 'console'
needs_stub = true

cpp = meson.get_compiler('cpp')
if cpp.get_id() == 'msvc'
    cpp_args = ['/Zc:strictStrings-', '/D_CRT_SECURE_NO_WARNINGS']
    if host_machine.cpu_family() == 'x86'
        win_subsystem = 'windows'
        needs_stub = false
        elma_sources += files(
            'F_WIN/W_DI.CPP',
            'F_WIN/W_HHIGH.CPP',
            'F_WIN/W_K.CPP',
            'F_WIN/W_SOUND2.CPP',
            'F_WIN/W_V.CPP',
            'H_WAV.CPP',
            'dxinc/getdxver.CPP',
        )
        link_args += [
            '/ENTRY:mainCRTStartup',
            'winmm.lib',
            meson.project_source_root() + '/dxinc/dinput.lib',
            meson.project_source_root() + '/dxinc/dxguid.lib',
            meson.project_source_root() + '/dxinc/dsound.lib',
        ]
    endif
else
    cpp_args = ['-Wno-write-strings', '-Wno-deprecated-declarations']

    if cpp.get_id() == 'clang'
        cpp_args += ['-Wno-nonportable-include-path']
    endif
endif

if needs_stub
    elma_sources += files('H_DUMMY.CPP', 'stub.cpp')
endif

sdl2_dep = dependency('sdl2', static: build_machine.system() == 'windows')

elma = executable(
    'elma',
    elma_sources,
    cpp_args: cpp_args,
    link_args: link_args,
    dependencies: [sdl2_dep],
    win_subsystem: win_subsystem,
)
