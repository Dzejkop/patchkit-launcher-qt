win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../patchkit-launcher-qt-src/release/ -lLauncherLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../patchkit-launcher-qt-src/debug/ -lLauncherLib
else:unix: LIBS += -L$$OUT_PWD/../patchkit-launcher-qt-src/ -lLauncherLib

INCLUDEPATH += $$PWD/../patchkit-launcher-qt-src
DEPENDPATH += $$PWD/../patchkit-launcher-qt-src

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../patchkit-launcher-qt-src/release/libLauncherLib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../patchkit-launcher-qt-src/debug/libLauncherLib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../patchkit-launcher-qt-src/release/LauncherLib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../patchkit-launcher-qt-src/debug/LauncherLib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../patchkit-launcher-qt-src/libLauncherLib.a

win32 {
    !contains(QMAKE_TARGET.arch, x86_64) {
        Debug:LIBS += $$PWD/lib/zlib/win_i386/zlibstaticd_md.lib
        Release:LIBS += $$PWD/lib/zlib/win_i386/zlibstatic_mt.lib

        Debug:LIBS += $$PWD/lib/quazip/win_i386/quazip_staticd_md.lib
        Release:LIBS += $$PWD/lib/quazip/win_i386/quazip_static_mt.lib

    } else {
        Debug:LIBS += $$PWD/lib/zlib/win_x86_64/zlibstaticd_md.lib
        Release:LIBS += $$PWD/lib/zlib/win_x86_64/zlibstatic_mt.lib

        Debug:LIBS += $$PWD/lib/quazip/win_x86_64/quazip_staticd_md.lib
        Release:LIBS += $$PWD/lib/quazip/win_x86_64/quazip_static_mt.lib
    }
}

unix:!macx {
    !contains(QMAKE_HOST.arch, x86_64) {
        debug:LIBS += $$PWD/lib/quazip/linux_i386/libquazipd.a -lz
        release:LIBS += $$PWD/lib/quazip/linux_i386/libquazip.a -lz
    } else {
        debug:LIBS += $$PWD/lib/quazip/linux_x86_64/libquazipd.a -lz
        release:LIBS += $$PWD/lib/quazip/linux_x86_64/libquazip.a -lz
    }
}

macx {
    debug:LIBS += $$PWD/lib/quazip/macosx_x86_64/libquazipd.a -lz
    release:LIBS += $$PWD/lib/quazip/macosx_x86_64/libquazip.a -lz
}

win32: LIBS += -lshell32
