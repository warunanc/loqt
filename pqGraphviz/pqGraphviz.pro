#--------------------------------------------------
# pqGraphviz.pro: SWI-Prolog / Graphviz / Qt
#--------------------------------------------------
# interface SWI-Prolog and lqXDot
#--------------------------------------------------
# Author        : Carlo Capelli
# Copyright (C) : 2013,2014,2015,2016

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

include(../loqt.pri)

QT += concurrent

TARGET = pqGraphviz
TEMPLATE = lib

DEFINES += PQGRAPHVIZ_LIBRARY

SOURCES += \
    pqGraphviz.cpp \
    pqDocView.cpp

HEADERS += \
    pqGraphviz.h \
    pqGraphviz_global.h \
    pqDocView.h

OTHER_FILES += \
    prolog/gv_uty.pl \
    test/genealogy/genealogy.pl \
    test/genealogy/familiari.pl \
    test/odbc/odbc_schema.pl \
    test/xref/paths_prefix.pl \
    test/xref/file_xref.pl \
    test/odbc/graph_schema.pl \
    prolog/termtree.pl \
    test/genealogy/familiari.pdf \
    test/genealogy/pqGraphviz_emu.pl \
    test/genealogy/allocator.pl

unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix:!macx {
    # because SWI-Prolog is built from source
    CONFIG += link_pkgconfig

    PKGCONFIG += swipl

    DEFINES += WITH_CGRAPH

    # latest download (2013/11/29) refuses to compile without this useless define
    DEFINES += HAVE_STRING_H

    PKGCONFIG += libcgraph libgvc
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lqXDot/release/ -llqXDot
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lqXDot/debug/ -llqXDot
else:unix: LIBS += -L$$OUT_PWD/../lqXDot/ -llqXDot

INCLUDEPATH += $$PWD/../lqXDot
DEPENDPATH += $$PWD/../lqXDot

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../pqConsole/release/ -lpqConsole
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../pqConsole/debug/ -lpqConsole
else:unix: LIBS += -L$$OUT_PWD/../pqConsole/ -lpqConsole

INCLUDEPATH += $$PWD/../pqConsole
DEPENDPATH += $$PWD/../pqConsole

RESOURCES += \
    pqGraphviz.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lqUty/release/ -llqUty
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lqUty/debug/ -llqUty
else:unix: LIBS += -L$$OUT_PWD/../lqUty/ -llqUty

INCLUDEPATH += $$PWD/../lqUty
DEPENDPATH += $$PWD/../lqUty
