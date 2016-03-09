/*
    lqGraphix    : SWI-Prolog and Qt Graphics Framework

    Author       : Carlo Capelli
    E-mail       : cc.carlo.cap@gmail.com
    Copyright (C): 2016

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "lqGraphixView.h"
#include "PREDICATE.h"
#include "SwiPrologEngine.h"

#include <QMessageBox>
#include <QDebug>

lqGraphixView::lqGraphixView(QWidget *parent) : QGraphicsView(parent)
{
    setScene(new lqGraphixScene);
}

lqGraphixView::lqGraphixView(const lqGraphixView &) : QGraphicsView() {
    qDebug() << "lqGraphixView";
}
lqGraphixView::~lqGraphixView() {
    qDebug() << "~lqGraphixView";
}

predicate1(consult)

bool lqGraphixView::loadScript(QString script) {
    SwiPrologEngine::in_thread it;
    try {
        if (consult(A(script))) {
            //PlCall("list_metatypes");
            return true;
        }
        QMessageBox::warning(this,
           tr("Failure"),
           tr("Script '%1' has no proper interface").arg(script));
    }
    catch(PlException &e) {
         QMessageBox::warning(this,
            tr("Error"),
            tr("Script '%1' cannot be loaded:\n%2").arg(script, CCP(e)));
    }
    return false;
}
