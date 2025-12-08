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
    'H_DUMMY.CPP',
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
    'README.CPP',
    'RECORDER.CPP',
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
    'stub.cpp',
)

cpp = meson.get_compiler('cpp')
if cpp.get_id() == 'msvc'
    cpp_args = ['/Zc:strictStrings-', '/D_CRT_SECURE_NO_WARNINGS']
else
    cpp_args = ['-Wno-write-strings', '-Wno-deprecated-declarations']

    if cpp.get_id() == 'clang'
        cpp_args += ['-Wno-nonportable-include-path']
    endif
endif

elma = executable('elma', elma_sources, cpp_args: cpp_args)
