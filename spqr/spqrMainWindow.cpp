/*
    spqr          : SWI-Prolog Qt Rendering

    Author        : Carlo Capelli
    E-mail        : cc.carlo.cap@gmail.com
    Copyright (C) : 2013,2014 Carlo Capelli

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

#include "spqrMainWindow.h"
#include "lqPreferences.h"
#include "blockSig.h"
#include "file2string.h"

#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QFileDialog>
#include <QDateTime>
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QStatusBar>

/** GUI setup
 */
spqrMainWindow::spqrMainWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent), MruHelper("spqr")
{
    statusBar()->showMessage(tr("spqr Setup"));

    lqPreferences p;
    p.loadGeometry(this);

    con = new ConsoleEdit(argc, argv);
    connect(con, SIGNAL(engine_ready()), this, SLOT(engineReady()));

    make_tabs();

    fileSource = p.value("fileSource").toString();
    lastDir = p.value("lastDir").toString();

    QMenu *m = menuBar()->addMenu(tr("&File"));
    m->addAction(tr("New..."), this, SLOT(newFile()), QKeySequence::New)->setStatusTip(tr("Create a new file"));
    m->addAction(tr("Open..."), this, SLOT(openFile()), QKeySequence::Open)->setStatusTip(tr("Open an existing file"));
    m->addMenu(mruMenu = new QMenu(tr("Recent &Files...")));
    loadMru(p, this);
    m->addSeparator();
    m->addAction(tr("Save"), this, SLOT(saveFile()), QKeySequence::Save)->setStatusTip(tr("Save the document to disk"));
    m->addAction(tr("Save As..."), this, SLOT(saveFileAs()), QKeySequence::SaveAs)->setStatusTip(tr("Save the document under a new name"));

    m->addSeparator();
    m->addAction(tr("E&xit"), qApp, SLOT(quit()), QKeySequence::Quit)->setStatusTip(tr("Exit the application"));

    menuBar()->addSeparator();
    menuBar()->addAction("&Graph", this, SLOT(viewGraph()));
    menuBar()->addAction("&Source", this, SLOT(viewSource()));
    menuBar()->addAction("&Console", this, SLOT(viewConsole()));
    menuBar()->addAction("&Help", this, SLOT(viewHelp()));

    if (argc >= 2)
        fileSource = argv[1];

    if (fileSource.length())
        viewDot();

    connect(&monitorScript, SIGNAL(fileChanged(QString)), this, SLOT(scriptChanged(QString)));
}

/** save settings
 */
spqrMainWindow::~spqrMainWindow() {
    lqPreferences p;
    p.saveGeometry(this);
    p.setValue("fileSource", fileSource);
    p.setValue("lastDir", lastDir);
    //p.setValue("lastMode", lastMode);
    storeMru(p);
    p.save();
    //delete gvsyn;
}

/** handle details on application quit
 */
void spqrMainWindow::closeEvent(QCloseEvent *e) {
    if (isWindowModified()) {
        QMessageBox box(this);
        box.setIcon(box.Warning);
        box.setInformativeText(tr("Current file has been modified!"));
        box.setText(tr("Do you want to save your changes?"));
        box.setStandardButtons(box.Save|box.Discard|box.Cancel);
        box.setDefaultButton(box.Save);
        switch (box.exec()) {
        case box.Save:
            saveFile();
            break;
        case box.Discard:
            break;
        default: //box.Cancel:
            e->ignore();
        }
    }
}

inline QString Prolog_Exts() { return QObject::tr("Prolog (*.lp *.pl *.pro)"); }

/** run selection dialog and run selected file
 */
void spqrMainWindow::openFile() {
    QFileDialog d(this, tr("Open Prolog file"), lastDir, Prolog_Exts());
    if (d.exec()) {
        lastDir = d.directory().path();
        fileSource = d.selectedFiles()[0];
        viewDot();
    }
}

/** save file, report errors
 */
void spqrMainWindow::saveViewDot(QString file, QString script, QString errmsg) {
    QFile f(file);
    monitorScript.removePath(file);
    if (f.open(f.WriteOnly|f.Text)) {
        f.write(script.toUtf8());
        f.close();
        fileSource = file;
        viewDot();
        setWindowModified(false);
    }
    else
        errbox(tr("error writing %1").arg(file), errmsg);
}

/** make backup and save script file
 */
void spqrMainWindow::saveFile() {
    QString fileBak = fileSource + ".bak";
    QFile::remove(fileBak);
    if (!QFile::copy(fileSource, fileBak))
        errbox(tr("error writing %1").arg(fileBak), tr("cannot make backup copy"));

    saveViewDot(fileSource, source()->toPlainText(), tr("cannot save Dot Script"));
}

/** get name and make a new file of it
 */
void spqrMainWindow::saveFileAs() {
    QFileDialog fd(this, tr("Save Prolog file as"), lastDir, Prolog_Exts());
    fd.setAcceptMode(fd.AcceptSave);
    fd.setDefaultSuffix("loqt");
    if (fd.exec())
        saveViewDot(fd.selectedFiles()[0], source()->toPlainText(), tr("cannot save Prolog source"));
}

/** create a new Prolog script
 */
void spqrMainWindow::newFile() {
    QFileDialog fd(this, tr("New Prolog file"), lastDir, Prolog_Exts());
    fd.setAcceptMode(fd.AcceptSave);
    fd.setDefaultSuffix("lp");
    if (fd.exec()) {
        QString newPath = fd.selectedFiles()[0];
        saveViewDot(newPath,
                    tr("/* Prolog source %1\n"
                       " * created at %2\n"
                       " */\n"
                       ":- module(%3, [%3/0]).\n"
                       /** ":- use_module(gv_uty).\n" */
                       "\n"
                       "% entry point\n"
                       "%3 :- graph_window(%3(G), G, [window_title(hello)]).\n"
                       "\n"
                       "%% %3(+G) is det.\n"
                       "%  build some graph\n"
                       "%3(G) :-\n"
                       "\tmake_node(G, hello, H),\n"
                       "\tmake_node(G, world, W),\n"
                       "\tnew_edge(G, H, W).\n"
                    ).arg(newPath, QDateTime::currentDateTime().toString(), QFileInfo(newPath).baseName().replace(' ', '_')),
                    tr("cannot create new Prolog file"));
    }
}

/** this is default as required by MruHelper
 */
void spqrMainWindow::openFileIndex(int i) {
    fileSource = files[i];
    viewDot();
}

void spqrMainWindow::make_tabs() {
    delete tabs;

    setCentralWidget(tabs = new QStackedWidget_KeybTabs);

    tabs->addWidget(new CodeMirroring);
    tabs->addWidget(con);
    tabs->addWidget(new HelpDocView);

    connect(source(), SIGNAL(helpRequestTopic(QString)), SLOT(helpRequest(QString)));

    connect(helpDoc(), SIGNAL(loadFinished(bool)), SLOT(adjustLocation()));
    connect(helpDoc(), SIGNAL(titleChanged(QString)), SLOT(adjustTitle()));
    connect(helpDoc(), SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(helpDoc(), SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));
}

void spqrMainWindow::engineReady() {
    connect(con->engine(), SIGNAL(query_complete(QString,int)), this, SLOT(queryComplete(QString,int)));
    connect(con->engine(), SIGNAL(query_exception(QString,QString)), this, SLOT(queryException(QString,QString)));

    SwiPrologEngine::in_thread it;
    foreach (auto m, QString("gv_uty").split(',')) {
        bool rc = it.resource_module(m);
        qDebug() << m << rc;
    }

    con->engine()->query_run("use_module(library(pldoc))");
    con->engine()->query_run(QString("doc_server(%1)").arg(DOC_PORT));
}

void spqrMainWindow::queryComplete(QString query, int tot_occurrences) {
    qDebug() << "queryComplete" << query << tot_occurrences;
    if (query.indexOf("doc_server") == 0) {
        helpDoc()->setUrl(QString("http://localhost:%1").arg(DOC_PORT));
        con->engine()->query_run(QString("(%1)").arg(DOC_PORT));
    }
}
void spqrMainWindow::queryException(QString functor, QString exmsg) {
    errbox(tr("Query Exception"), QString("%1 - %2").arg(functor, exmsg));
}

void spqrMainWindow::helpRequest(QString topic)
{
    helpDoc()->setUrl(QString("http://localhost:%1/search?for=%2&in=all&match=summary").arg(DOC_PORT).arg(topic));
    viewHelp();
}

/** reinitialize GUI with required script
 */
void spqrMainWindow::viewDot() {
    if (!source()->loadFile(fileSource)) {
        errbox(tr("Cannot read %1").arg(fileSource), tr("open file failed"));
        removePath(this, fileSource);
    }
}

/** change the rendered
 */
void spqrMainWindow::changeLayout() {
    foreach (QAction *a, menuBar()->actions()[0]->menu()->actions())
        if (a->isCheckable())
            a->setChecked(false);
    qobject_cast<QAction*>(sender())->setChecked(true);
    viewDot();
}

/** keep modified status updated
 */
void spqrMainWindow::textChanged() {
    if (mode == highlighting)
        mode = editing;
    else
        setWindowModified(true);
}

void spqrMainWindow::scriptChanged(QString path) {
    MB req(MB::Warning, tr("Warning"), tr("Script has been modified"), MB::Yes|MB::Ignore, this);
    req.setInformativeText(tr("Do you want to reload '%1'").arg(path));
    if (req.exec() == MB::Yes)
        viewDot();
}

/** factorize common usage
 */
QMessageBox::StandardButton spqrMainWindow::errbox(QString msg, QString info, MB::StandardButtons buttons, MB::Icon ic, QString title) {
    if (title.isEmpty())
        title = tr("Error");
    MB box(ic, title, msg, buttons, this);
    box.setInformativeText(info);
    return MB::StandardButton(box.exec());
}

void spqrMainWindow::adjustLocation() {
    if (locationEdit)
        locationEdit->setText(helpDoc()->url().toString());
}

void spqrMainWindow::changeLocation() {
    QUrl url = QUrl(locationEdit->text());
    helpDoc()->load(url);
    helpDoc()->setFocus();
}

void spqrMainWindow::adjustTitle() {
    if (progress <= 0 || progress >= 100)
        setWindowTitle(helpDoc()->title());
    else
        setWindowTitle(QString("%1 (%2%)").arg(helpDoc()->title()).arg(progress));
}

void spqrMainWindow::setProgress(int p) {
    progress = p;
    adjustTitle();
}

void spqrMainWindow::finishLoading(bool) {
    progress = 100;
    adjustTitle();
}

void spqrMainWindow::viewGraph() {
    auto e = con->engine();
    e->query_run(QString("consult('%1')").arg(fileSource));
    e->query_run(QFileInfo(fileSource).baseName());
}

void spqrMainWindow::viewSource() {
    tabs->setCurrentIndex(t_source);
}

void spqrMainWindow::viewConsole() {
    tabs->setCurrentIndex(t_console);
}

void spqrMainWindow::viewHelp() {
    tabs->setCurrentIndex(t_helpdoc);
}

void spqrMainWindow::log(QString msg) {
    qDebug() << msg;
}

void spqrMainWindow::msg(QString msg) {
    statusBar()->showMessage(msg);
}

void spqrMainWindow::err(QString msg) {
    errbox(tr("Error"), msg);
}