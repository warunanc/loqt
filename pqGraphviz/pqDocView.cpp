/*
    pqGraphviz    : interfacing SWI-Prolog and Graphviz library

    Author        : Carlo Capelli
    E-mail        : cc.carlo.cap@gmail.com
    Copyright (C): 2013,2014,2015,2016

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

#include "pqDocView.h"
#include "SwiPrologEngine.h"
#include "pqConsole.h"

#include "PREDICATE.h"
#include <QFutureWatcher>
#include <QMessageBox>
#include <QtConcurrent>
#include <QTime>

pqDocView::pqDocView(QWidget *parent) :
    WEB_VIEW_BASE(parent)
{
    setHtml(QString("<h4>%1</h4>").arg(tr("Please wait, plDoc is starting...")));
    setWindowTitle(tr("Help (courtesy plDoc)"));
}

bool pqDocView::startPlDoc()
{
#ifdef QT_WEBENGINE_LIB
    QTimer::singleShot(1, [this]() {
        QString msg;
        try {
            if (!PlCall("use_module(library(pldoc))"))
                msg = tr("library(pldoc) not available");
            else {
                if (!PlCall(QString("doc_server(%1)").arg(helpDocPort).toUtf8()))
                    msg = tr("cannot start doc_server at port %1").arg(helpDocPort);
            }
        }
        catch(PlException e) {
            msg = t2w(e);
        }
        requests << QString("http://localhost:%1/doc_for?object=root").arg(helpDocPort);
        emit initUrl();
    });
#else
    // start from a background thread
    auto f = [&]() {
        QString msg;
        SwiPrologEngine::in_thread _it;
        try {
            if (true) { //!PlCall("current_module(pldoc)")) {
                if (!PlCall("use_module(library(pldoc))"))
                    msg = tr("library(pldoc) not available");
                else {
                    //if (!PlCall("current_module(pldoc_http)"))
                    if (!PlCall(QString("doc_server(%1)").arg(helpDocPort).toUtf8()))
                        msg = tr("cannot start doc_server at port %1").arg(helpDocPort);
                    /* seems useless by now
                    else if (!PlCall("use_module(library(pldoc/doc_library))") || !PlCall("doc_load_library"))
                        msg = tr("cannot doc_load_library"); */
                }
            }
            else
                msg = "pldoc already running";
        }
        catch(PlException e) {
            msg = t2w(e);
        }
        qDebug() << "msg:" << msg;
    };

    //requests << QString("http://localhost:%1").arg(helpDocPort);
    requests << QString("http://localhost:%1/doc_for?object=root").arg(helpDocPort);

    auto w = new QFutureWatcher<void>;
    connect(w, SIGNAL(finished()), this, SLOT(initUrl()));

    // run the Prolog snippet in background
    w->setFuture(QtConcurrent::run(f));
#endif

    return true;
}

void pqDocView::initUrl()
{
    qDebug() << "initUrl()" << requests.join(" $ ");
    if (requests.count())
        setUrl(requests.takeFirst());
}

void pqDocView::linkHovered(QString link, QString title, QString)
{
    if (title.length())
        statusBar->showMessage(QString("%1 [%2]").arg(title, link));
    else
        statusBar->showMessage(link);
}

void pqDocView::helpTopic(QString topic)
{
    requests << QString("http://localhost:%1/search?for=%2&in=all&match=summary").arg(helpDocPort).arg(topic);
    emit initUrl();
}

void pqDocView::helpFile(QString file)
{
    requests << QString("http://localhost:%1/doc%2").arg(helpDocPort).arg(file);
    emit initUrl();
}

void pqDocView::addFeedback(QToolBar *tbar, QStatusBar *sbar)
{
    toolBar = tbar;
#ifdef QT_WEBENGINE_LIB
    toolBar->addAction(pageAction(QWebEnginePage::Back));
    toolBar->addAction(pageAction(QWebEnginePage::Forward));
    toolBar->addAction(pageAction(QWebEnginePage::Reload));
    toolBar->addAction(pageAction(QWebEnginePage::Stop));
#else
    toolBar->addAction(pageAction(QWebPage::Back));
    toolBar->addAction(pageAction(QWebPage::Forward));
    toolBar->addAction(pageAction(QWebPage::Reload));
    toolBar->addAction(pageAction(QWebPage::Stop));
#endif
    //toolBar->addWidget(locationEdit);

    connect(this, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    connect(this, SIGNAL(loadProgress(int)), SLOT(loadProgress(int)));

    statusBar = sbar;
    connect(this, SIGNAL(titleChanged(QString)), sbar, SLOT(showMessage(QString)));
#ifndef QT_WEBENGINE_LIB
    connect(page(), SIGNAL(linkHovered(QString,QString,QString)), SLOT(linkHovered(QString,QString,QString)));
    connect(page(), SIGNAL(statusBarMessage(QString)), sbar, SLOT(showMessage(QString)));
#endif
}

void pqDocView::loadFinished(bool yn)
{
    statusBar->showMessage(tr("load %1 [%2]").arg(url().toString(), yn ? "ok" : "ko"));
    if (yn)
        setWindowTitle(url().toString());
    update();
}

void pqDocView::loadProgress(int perc)
{
    statusBar->showMessage(tr("load %2 [%1 %%]").arg(url().toString(), perc));
}
