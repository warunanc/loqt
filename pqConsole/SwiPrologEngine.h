/*
    pqConsole    : interfacing SWI-Prolog and Qt

    Author       : Carlo Capelli
    E-mail       : cc.carlo.cap@gmail.com
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

#ifndef SWIPROLOGENGINE_H
#define SWIPROLOGENGINE_H

#include "swi.h"

#include <QMap>
#include <QMutex>
#include <QThread>
#include <QVariant>
#include <QStringList>
#include <QWaitCondition>
#include <functional>

/** 1. attempt to run generic code inter threads */
typedef std::function<void()> pfunc;

#include "FlushOutputEvents.h"
#include "pqConsole_global.h"

/** interface IO running SWI Prolog engine in background
 */
class PQCONSOLESHARED_EXPORT SwiPrologEngine : public QThread, public FlushOutputEvents {
    Q_OBJECT
public:

    explicit SwiPrologEngine(ConsoleEdit *target, QObject *parent = 0);
    ~SwiPrologEngine();

    /** main console startup point */
    void start(int argc, char **argv);

    /** run query on background thread */
    void query_run(QString text);
    void query_run(QString module, QString text);

    /** run script on background thread */
    void script_run(QString name, QString text);

    /** start/stop a Prolog engine in thread - use for syncronized GUI */
    struct PQCONSOLESHARED_EXPORT in_thread {
        in_thread();
        ~in_thread();

        /** run named script in current thread */
        bool named_load(QString name, QString script, bool silent = true);

        /** if module not yet loaded, load code (i.e. assumes it starts with :-module(module)) */
        bool inline_module(QString Module, QString code, bool silent = true);

        /** if not yet loaded, parse module code from resource */
        bool resource_module(QString module, QString location = ":/prolog", bool silent = true);

    private:

        /** allocate resources for current calls */
        PlFrame *frame;

        /** create only in not already available */
        int thid;
    };

    /** handle application quit request in thread that started PL_toplevel */
    static bool quit_request();

    /** utility: make public */
    static void msleep(unsigned long n) { QThread::msleep(n); }

    /** query engine about expected interface */
    static bool is_tty(const FlushOutputEvents *target = 0);

    /** loading in foreign thread */
    static bool named_load(QString n, QString t, bool silent_yn);

signals:

    /** issued to queue a string to user output */
    void user_output(QString output);

    /** issued to peek input - til to CR - from user */
    void user_prompt(int threadId, bool tty);

    /** signal a query result */
    void query_result(QString query, int occurrence);

    /** signal query completed */
    void query_complete(QString query, int tot_occurrences);

    /** signal exception */
    void query_exception(QString query, QString message);

public slots:

    /** store string in buffer */
    void user_input(QString input);

protected:

    // run a polling loop on buffer and queries
    virtual void run();

    int argc;
    char **argv;

    /** queries to be dispatched to engine thread */
    struct query {
        bool is_script; // change entry type
        QString name;   // arbitrary symbol
        QString text;   // if is_script is path name, else query text
        query(bool is_script, const QString & name, const QString & text) :
           is_script(is_script), name(name), text(text) {}
    };

    QMutex sync;
    QByteArray buffer;      // syncronized !
    QList<query> queries;   // syncronized !

    void serve_query(query q);

    static ssize_t _read_(void *handle, char *buf, size_t bufsize);
    static ssize_t _write_(void *handle, char *buf, size_t bufsize);
    static int     _control_(void *handle, int cmd, void *closure);
    ssize_t _read_(char *buf, size_t bufsize);
    ssize_t _read_f(char *buf, size_t bufsize);

    static int halt_engine(int status, void*data);

private slots:

    void awake();

private:

    /** main console singleton (thread constructed differently) */
    static SwiPrologEngine* spe;
    friend struct in_thread;
};

#endif // SWIPROLOGENGINE_H
