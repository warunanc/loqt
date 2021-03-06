/*
    lqXDot       : interfacing Qt and Graphviz library

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

#ifndef LQXDOTSCENE_H
#define LQXDOTSCENE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPointer>

#include "lqContextGraph.h"
#include "lqAobj.h"
class lqXDotView;

#include <graphviz/xdot.h>

/** translate a Graphviz graph to a QGraphicScene display graph
  * each graph object get its concrete representation as QGraphicsItemGroup
  */
class LQXDOTSHARED_EXPORT lqXDotScene : public QGraphicsScene, public GV_ptr_types
{
    Q_OBJECT

    Q_PROPERTY(bool truecolor READ truecolor WRITE setTruecolor)
    Q_PROPERTY(QString imagepath READ imagepath WRITE setImagepath)

public:

    lqXDotScene(lqContextGraph *cg);

    bool truecolor() const { return truecolor_; }
    void setTruecolor(bool value) { truecolor_ = value; }

    QString imagepath() const { return imagepath_; }
    void setImagepath(QString value) { imagepath_ = value; }

    static QColor parse_color(QString color, bool truecolor);

    Np to_node(lqNode* n) const {
        return agnode(*cg, qcstr(n->name()), 0);
    }
    Np it_node(QGraphicsItem* i) const {
        lqNode* n = ancestor<lqNode>(i);
        return n ? to_node(n) : 0;
    }
    lqNode *find_node(Np obj) const;

    typedef QList<QGraphicsItem*> l_items;

    //! change the content to get node folded/unfolded
    lqXDotScene* fold(lqNode *node, lqXDotView* v);

    //! dump to debugger output
    void dump(QString m) const;

    //! perform translation from layout to graphics primitives
    void build();

    //! parse the bounding rect attribute on <graph> to QRectF
    QRectF graph_bb(Gp graph);

    //! after a partial layout, just redo changed objects
    void redo_objects(QList<void*> objects);

signals:

    void setup_completed();
    void reload_layout(QString newLayout);

public slots:

    void msg(QString);
    void itemHasChanged(QGraphicsItem::GraphicsItemChange,QVariant);

protected:

    //! context/graph for root object
    QPointer<lqContextGraph> cg;

    //! when enabled, accept colors with alpha component
    bool truecolor_;

    //! required when displaying images in rendered graph
    QString imagepath_;

    //! constructing visual objects
    virtual QGraphicsItem* add_node(Np n);
    virtual QGraphicsItem* add_edge(Ep e);
    void subgraphs(Gp graph, qreal off_z);

    //! factories
    virtual lqNode* build_node(Np n, l_items items);
    virtual lqEdge* build_edge(Ep e, l_items items);
    virtual lqGraph* build_subgraph(Gp g, l_items items);

public:
    enum x_attrs {
        _draw_ = 1 << 0,
        _ldraw_ = 1 << 1,
        _hdraw_ = 1 << 2,
        _tdraw_ = 1 << 3,
        _hldraw_ = 1 << 4,
        _htdraw_ = 1 << 5,

        x_attrs_node = _draw_|_ldraw_,
        x_attrs_edge = x_attrs_node|_hdraw_|_tdraw_|_hldraw_|_htdraw_,
        x_attrs_graph = _draw_|_ldraw_
    };
    static void clear_XDotAttrs(void *obj, int ops);

    //! change using values from lqXDot_configure.h
    static int configure_behaviour;

protected:

    l_items build_graphic(void *obj, int ops);

    l_items build_graphic(Np obj) { return build_graphic(obj, x_attrs_node); }
    l_items build_graphic(Ep obj) { return build_graphic(obj, x_attrs_edge); }
    l_items build_graphic(Gp obj) { return build_graphic(obj, x_attrs_graph); }

    //! presumed bounding box - not actually built graphics yet
    QRectF bb_rect(void *obj, int ops);
    QRectF bb_rect(Np node) { return bb_rect(node, x_attrs_node); }

    typedef QVector<QPointF> t_poly;
    t_poly poly_spec(const xdot_polyline& l) const {
        t_poly pts;
        for (int c = 0; c < l.cnt; ++c)
            pts.append(QPointF(l.pts[c].x, cy(l.pts[c].y)));
        return pts;
    }

    //! parse font spec specifier
    static QString font_spec(cstr fontname);

    QRectF rect_spec(const xdot_rect& r) const {
        return QRectF(QPointF(r.x - r.w, cy(r.y + r.h)), QSize(r.w * 2, r.h * 2));
    }

    void perform_attrs(void* obj, int attrs, std::function<void(const xdot_op& op)> worker) const;

    //! map graphviz coordinates to scene
    QRectF bbscene;
    qreal cy(qreal y) const { return bbscene.height() - y; }

    typedef QHash<QString, lqNode*> name2node;
    name2node names2nodes;
    name2node nodeNames() {
        name2node v;
        cg->for_nodes([&](Np n) { v[gvname(n)] = find_node(n); });
        return v;
    }

    //! handle layout rebuild
    virtual void moveEdges(lqNode *moving, QPointF nodeOldpos);

    QPointer<lqNode> nodeMoving;
    QPointF nodeOldPos, nodeNewPos;

    // QGraphicsScene interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // LQXDOTSCENE_H
