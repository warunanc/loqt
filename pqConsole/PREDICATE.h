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

#ifndef PREDICATE_H
#define PREDICATE_H

#include "swi.h"

#ifndef NO_SHORTEN_INTERFACE

/** shorten interface */

typedef const char* CCP;
typedef const wchar_t* WCP;
typedef const void* CVP;

typedef char* CP;
typedef void* VP;

#define CT QThread::currentThread()

#include <QString>
#include <QVector>

inline CCP S(const PlTerm &T) { return T; }

inline PlAtom W(const QString &s) {
    return PlAtom(s.toStdWString().data());
}

inline PlAtom A(QString s) {
    return W(s);
}

inline QString t2w(PlTerm t) {
    return QString::fromWCharArray(WCP(t));
}

/** fast interface to get a string out of a ground term.
  * thanks Jan !
  */
inline QString serialize(PlTerm t) {
    wchar_t *s;

    if ( PL_get_wchars(t, NULL, &s, CVT_WRITEQ|BUF_RING) )
      return QString::fromWCharArray(s);

    throw PlTypeError("text", t);
    PL_THROWN(NULL);
}

typedef PlTerm T;
typedef PlTermv V;
typedef PlCompound C;
typedef PlTail L;

inline PlTerm empty_list() {
    PlTerm el;
    PlTail EL(el);
    EL.close();
    return el;
}

inline PlTerm build_list(PlTerm host, QVector<PlTerm> ts) {
    PlTail l(host);
    for (auto x: ts)
        l.append(x);
    l.close();
    return host;
}

/** get back an object passed by pointer to Prolog */
template<typename Obj> Obj* pq_cast(T ptr) { return static_cast<Obj*>(static_cast<void*>(ptr)); }

/** structureN(name): named compound term construction.
    For instance 'structure2(point)' enables
      point(X,Y)
    instead of
      PlCompound("point", PlTermv(X,Y))
 */
#define structure1(X) inline PlCompound X(T A) { return PlCompound(#X, V(A)); }
#define structure2(X) inline PlCompound X(T A, T B) { return PlCompound(#X, V(A, B)); }
#define structure3(X) inline PlCompound X(T A, T B, T C) { return PlCompound(#X, V(A, B, C)); }
#define structure4(X) inline PlCompound X(T A, T B, T C, T D) { return PlCompound(#X, V(A, B, C, D)); }
#define structure5(X) inline PlCompound X(T A, T B, T C, T D, T E) { return PlCompound(#X, V(A, B, C, D, E)); }

/** predicateN(name) : access Prolog predicate by name.
    For instance predicate2(member) enables
      if (member(X, Y))...
    instead of
      if (PlCall("member", PlTermv(X, Y)))...
 */
#define predicate0(P) inline int P() { return PlCall(#P); }
#define predicate1(P) inline int P(T A) { return PlCall(#P, V(A)); }
#define predicate2(P) inline int P(T A, T B) { return PlCall(#P, V(A, B)); }
#define predicate3(P) inline int P(T A, T B, T C) { return PlCall(#P, V(A, B, C)); }
#define predicate4(P) inline int P(T A, T B, T C, T D) { return PlCall(#P, V(A, B, C, D)); }
#define predicate5(P) inline int P(T A, T B, T C, T D, T E) { return PlCall(#P, V(A, B, C, D, E)); }

/** mod_predicateN(module,name) : access Prolog predicate by name and context module.
    For instance mod_predicate2(lists,member) enables
      if (member(X, Y))...
    instead of
      if (PlCall("lists", "member", PlTermv(X, Y)))...
 */
#define mod_predicate0(M,P) inline int P() { return PlCall(#M, #P); }
#define mod_predicate1(M,P) inline int P(T A) { return PlCall(#M, #P, V(A)); }
#define mod_predicate2(M,P) inline int P(T A, T B) { return PlCall(#M, #P, V(A, B)); }
#define mod_predicate3(M,P) inline int P(T A, T B, T C) { return PlCall(#M, #P, V(A, B, C)); }
#define mod_predicate4(M,P) inline int P(T A, T B, T C, T D) { return PlCall(#M, #P, V(A, B, C, D)); }
#define mod_predicate5(M,P) inline int P(T A, T B, T C, T D, T E) { return PlCall(#M, #P, V(A, B, C, D, E)); }

/** queryN(name) : multiple solution by name.
    For instance 'query3(select)' enables
      for (select s(X, Xs, Rs); s; ) {}
    instead of
      PlQuery s("select", PlTermv(X, X, Rs));
      while (s.next_solution()) {}
 */
#define LOOP__ { } operator bool() { return next_solution(); }
#define query0(P) struct P : PlQuery { P() : PlQuery(#P, V()) LOOP__ };
#define query1(P) struct P : PlQuery { P(T A) : PlQuery(#P, V(A)) LOOP__ };
#define query2(P) struct P : PlQuery { P(T A, T B) : PlQuery(#P, V(A, B)) LOOP__ };
#define query3(P) struct P : PlQuery { P(T A, T B, T C) : PlQuery(#P, V(A, B, C)) LOOP__ };
#define query4(P) struct P : PlQuery { P(T A, T B, T C, T D) : PlQuery(#P, V(A, B, C, D)) LOOP__ };
#define query5(P) struct P : PlQuery { P(T A, T B, T C, T D, T E) : PlQuery(#P, V(A, B, C, D, E)) LOOP__ };

#endif

#endif // PREDICATE_H
