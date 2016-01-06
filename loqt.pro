#--------------------------------------------------
# loqt.pro: Logic / Qt interface
#--------------------------------------------------
# Collection of Qt components to efficiently
# interface logic languages
#--------------------------------------------------
# Author        : Carlo Capelli
# Copyright (C) : 2013,2014,2015

TEMPLATE = subdirs

SUBDIRS += \
    fdqueens \
    lqUty \
    lqXDot \
    lqXDot_test \
    pqConsole \
    pqGraphviz \
    pqXml \
    pqSource \
    pqSourceTest \
    spqr \
    qPrologPad \
    testKeyboardMacros

OTHER_FILES += \
    loqt.pri \
    README.md \
    img/cluster.png \
    img/cluster-dot.png
