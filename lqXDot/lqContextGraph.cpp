/*
    lqXDot       : interfacing Qt and Graphviz library

    Author       : Carlo Capelli
    E-mail       : cc.carlo.cap@gmail.com
    Copyright (C): 2013, Carlo Capelli

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

#include "lqContextGraph.h"
#include <QStack>
#include <QDebug>

/** shortcuts */
inline void OK(int rc) { Q_ASSERT(rc == 0); }

/** allocate empty
 */
lqContextGraph::lqContextGraph(QObject *parent) :
    QObject(parent), context(0), graph(0), buffer(0)
{
}

/** keep pointers allocated elsewhere (Prolog, in first use case)
 */
lqContextGraph::lqContextGraph(GVC_t* context, Agraph_t *graph, QObject *parent) :
    QObject(parent), context(context), graph(graph), buffer(0)
{
}

/** release resources
 */
lqContextGraph::~lqContextGraph()
{
    clear();
}

/** hopefully remove cleanup all memory used
 */
void lqContextGraph::clear() {
    if (context) {
        if (graph) {
            gvFreeLayout(context, graph);
            agclose(graph);
            graph = 0;
        }
        if (buffer) {
            // no layout performed on buffer
            agclose(buffer);
            buffer = 0;
        }
        gvFreeContext(context);
        context = 0;
    }
}

/** keep a static list to collect errors from global handler
 */
QStringList lqContextGraph::errors;
const int max_errors = 10;

int lqContextGraph::store_errors(char *msg) {
    if (errors.count() < max_errors)
        errors.append(QString::fromUtf8(msg));
    return 0;
}

/** perform task
 *  display a box with errors (upto max_errors), if any
 *  return true if no error happened
 */
bool lqContextGraph::run_with_error_report(std::function<QString()> worker) {

    QString err;
    try {
        errors.clear();
        agusererrf savef = agseterrf(store_errors);
        err = worker();
        agseterrf(savef);
    }
    catch(...) {
        err = tr("Exception!");
    }
    if (!err.isEmpty() || !errors.isEmpty()) {
        critical(err + "\n" + errors.join("\n"));
        return false;
    }
    return true;
}

/** ensure context available for subsequent operations
 */
bool lqContextGraph::in_context() {
    if (!context && (context = gvContext()) == 0) {
        critical(tr("gvContext() failed"));
        return false;
    }
    return true;
}

/** perform required layout with basic error handling
 */
bool lqContextGraph::layout(QString algo) {
    if (!in_context())
        return false;

    // workaround multiple boxes on root, suggested by Erg
    agset(graph, ccstr("_draw_"), ccstr(""));

    if (gvLayout(context, graph, algo.toUtf8().data()) == 0) {
        last_layout = algo;
        return true;
    }
    critical(tr("gvLayout(%1) failed").arg(algo));
    return false;
}

/** perform rendering with basic error handling
 */
bool lqContextGraph::render(QString algo) {
    if (!in_context())
        return false;
    if (gvRender(context, graph, algo.toUtf8().data(), 0) == 0) {
        last_render = algo;
        return true;
    }
    critical(tr("gvRender(%1) failed on %2").arg(algo, last_layout));
    return false;
}

/** repeat layout and render operations with basic error handling
 */
bool lqContextGraph::repeatOperations() {
    if (freeLayout()) {
        if (layout(last_layout))
            if (render(last_render))
                return true;
    }
    critical(tr("repeatOperations failed on %1,%2").arg(last_layout, last_render));
    return false;
}

/** release layout memory with basic error handling
 */
bool lqContextGraph::freeLayout() {
    if (gvFreeLayout(context, graph)) {
        critical(tr("gvFreeLayout failed"));
        return false;
    }
    return true;
}

/** read from file
 */
bool lqContextGraph::parse(FILE *fp) {
    return (graph = agread(fp, 0)) ? true : false;
}

/** read from string
 */
bool lqContextGraph::parse(QString script) {
    return (graph = agmemread(script.toUtf8())) ? true : false;
}

/** allocate a spare graph, to store elements
 *  on structure change (folding/unfolding)
 */
lqContextGraph::Gp lqContextGraph::buff(bool decl_attrs) {
    if (!buffer) {
        bool strict = agisstrict(graph);
        Agdesc_t desc = agisdirected(graph) ? (strict ? Agstrictdirected : Agdirected) : (strict ? Agstrictundirected : Agundirected);
        buffer = agopen(ccstr("#buffer#"), desc, 0);
    }
    if (decl_attrs) {
        for (Agsym_t* sym = 0; (sym = agnxtattr(graph, AGNODE, sym)); )
            agattr(buffer, AGNODE, sym->name, sym->defval);
        for (Agsym_t* sym = 0; (sym = agnxtattr(graph, AGEDGE, sym)); )
            agattr(buffer, AGEDGE, sym->name, sym->defval);
    }
    Q_ASSERT(buffer);
    return buffer;
}

/** a simple depth first visit starting on <root>
 */
void lqContextGraph::depth_first(Np root, Nf nv, Gp g) {
    depth_first(root, nv, [](Ep){}, g);
}

/** a simple depth first visit starting on <root>
 */
void lqContextGraph::depth_first(Np root, Nf nv, Ef ev, Gp g) {
    QStack<Np> s; s.push(root);
    nodes visited;
    while (!s.isEmpty()) {
        Np n = s.pop();
        if (!visited.contains(n)) {
            visited << n;
            nv(n);
            for_edges_out(n, [&](Ep e){ ev(e); s.push(e->node); }, g);
        }
    }
}

/** a simple depth first visit
 */
void lqContextGraph::depth_first(Gf gv, Gp root) {
    QStack<Gp> s; s.push(root ? root : graph);
    while (!s.isEmpty()) {
        Gp n = s.pop();
        gv(n);
        for_subgraphs(n, [&](Gp g){ s.push(g); });
    }
}

/** this test must be performed only on 'user available' nodes
 */
bool lqContextGraph::is_folded(Np n) const {
    Q_ASSERT(agraphof(n) == graph);
    return buffer != 0 && agnode(buffer, agnameof(n), 0) != 0;
}

/** make structural changes required to fold node <n>
 *  all nodes reachable from n are merged to n (removed and buffered in context),
 *  and edges are routed to arguably preserve the structure
 */
void lqContextGraph::fold(Np n) {
    Q_ASSERT(!is_folded(n));

    edges edel;
    nodes ndel;

    Gp B = buff(true);

    Nf N = [&](Np v) {
        Np t = agnode(B, agnameof(v), 1);
        OK(agcopyattr(v, t));
        if (v != n && !ndel.contains(v))
            ndel << v;
    };
    Ef E = [&](Ep e) {
        Ep t = agedge(B, copy(agtail(e)), copy(aghead(e)), agnameof(e), 1);
        OK(agcopyattr(e, t));
        if (!edel.contains(e))
            edel << e;
    };
    depth_first(n, N, E);

    foreach(auto x, edel)
        agdeledge(graph, x);
    foreach(auto x, ndel)
        agdelnode(graph, x);

    OK(agsafeset(n, ccstr("shape"), ccstr("folder"), ccstr("ellipse")));
}
    /*
void lqContextGraph::fold(Np n) {
    Q_ASSERT(!is_folded(n));

    edges edel;
    nodes ndel;

    Np folded = agnode(buff(true), agnameof(n), 1);
    OK(agcopyattr(n, folded));

    for_edges_out(n, [&](Ep e)
    {
        // move edges from hidden to source
        edel << e;

        Np h = agnode(buff(), agnameof(e->node), 1);
        OK(agcopyattr(e->node, h));

        Ep E = agedge(buff(), folded, h, agnameof(e), 1);
        OK(agcopyattr(e, E));

        if (!ndel.contains(e->node))    // multiple edges ?
            // remove node
            ndel << e->node;
    });

    foreach(auto x, edel)
        agdeledge(graph, x);
    foreach(auto x, ndel)
        agdelnode(graph, x);

    OK(agsafeset(n, ccstr("shape"), ccstr("folder"), ccstr("ellipse")));
}
*/

/** make structural changes required to unfold node <n>
 */
    /*
void lqContextGraph::unfold(Np n) {
    Q_ASSERT(is_folded(n));

    edges edel;
    nodes ndel;

    Np t = agnode(buff(true), agnameof(n), 0);
    OK(agcopyattr(t, n));

    ndel << t;

    for_edges_out(t, [&](Ep e)
    {
        edel << e;

        Np h = agnode(graph, agnameof(e->node), 1);
        OK(agcopyattr(e->node, h));

        Ep E = agedge(graph, n, h, agnameof(e), 1);
        OK(agcopyattr(e, E));

        if (!ndel.contains(e->node))    // multiple edges ?
            ndel << e->node;
    }, buff());

    foreach(auto x, edel)
        agdeledge(buff(), x);
    foreach(auto x, ndel)
        agdelnode(buff(), x);
}
*/
void lqContextGraph::unfold(Np n) {
    Q_ASSERT(is_folded(n));

    Gp B = buff(true);

    edges edel;
    nodes ndel;

    Np t = agnode(B, agnameof(n), 0);
    OK(agcopyattr(t, n));

    Nf N = [&](Np v) {
        Np t = agnode(graph, agnameof(v), 1);
        OK(agcopyattr(v, t));
        if (!ndel.contains(v))
            ndel << v;
    };
    Ef E = [&](Ep e) {
        Ep t = agedge(graph, copy(agtail(e)), copy(aghead(e)), agnameof(e), 1);
        OK(agcopyattr(e, t));
        if (!edel.contains(e))
            edel << e;
    };
    depth_first(t, N, E, B);

    foreach(auto x, edel)
        agdeledge(B, x);
    foreach(auto x, ndel)
        agdelnode(B, x);
}
lqContextGraph::Np lqContextGraph::copy(Np n) {
    Gp g = agraphof(n) == graph ? buff() : graph;
    Np t = agnode(g, agnameof(n), 1);
    OK(agcopyattr(n, t));
    return t;
}
