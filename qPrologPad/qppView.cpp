/*
    qPrologPad   : SWI-Prolog PrologPad in Qt

    Author       : Carlo Capelli
    E-mail       : cc.carlo.cap@gmail.com
    Copyright (C): 2014 Carlo Capelli

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

#include "qppView.h"
#include <QWebFrame>
#include <QDebug>

void qppView::initialize() {
    connect(this, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
}

qppView::qppView(QWidget *parent) :
    QWebView(parent)
{
    initialize();
}

qppView::qppView(const qppView &e) : QWebView(e.parentWidget()) {
    initialize();
}

QString qppView::toPlainText() const {
    return eval("editor.getValue()").toString();
}

void qppView::setPlainText(QString s) {
    text = s;
    run("editor.setValue(proxy.plainText)");
}

void qppView::helpRequest(QString topic) {
    emit helpRequestTopic(topic);
}

void qppView::loadFinished(bool ok) {
    emit userMessage(log, QString("loadFinished %1... (len %2, ok %3)").arg(text.left(20)).arg(text.length()).arg(ok));
    if (ok) {
        frame()->addToJavaScriptWindowObject("proxy", this);
        if (text.length())
            run("editor.setValue(proxy.plainText)");
        run("editor.on(\"change\", function() { proxy.onChange() })");
    }
}

/** just returns the class name
 */
QString qppView::title() const {
    return metaObject()->className();
}

/** callback reflection from JS
 */
void qppView::onChange() { emit textModified(); }

void qppView::show_call(long from, long stop) {
    run(QString("show_call(%1,%2)").arg(from).arg(stop));
}

void qppView::run(QString script) const {
    frame()->evaluateJavaScript(script);
}
QVariant qppView::eval(QString script) const {
    return frame()->evaluateJavaScript(script);
}