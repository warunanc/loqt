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


#ifndef LQXDOTVIEW_H
#define LQXDOTVIEW_H

#include "lqXDot_global.h"
#include "lqXDotScene.h"

#include <QGraphicsView>
#include <QMouseEvent>
#include <QSignalMapper>

/** Graphviz library rendering using Qt Graphics View Framework
 */
class LQXDOTSHARED_EXPORT lqXDotView : public QGraphicsView, public GV_ptr_types {

    Q_OBJECT

    /** graphviz graph properties */
    Q_PROPERTY(bool truecolor READ truecolor WRITE setTruecolor)
    Q_PROPERTY(QString imagepath READ imagepath WRITE setImagepath)

    /** which layout has been run */
    Q_PROPERTY(QString layoutKind READ layoutKind WRITE setLayoutKind)

    /** enable node folding */
    Q_PROPERTY(bool foldNodes READ foldNodes WRITE setFoldNodes)

public:

    lqXDotView(QWidget* parent = 0);
    lqXDotView(const lqXDotView &v);

    virtual ~lqXDotView();

    operator bool() const { return Cp(*cg) && Gp(*cg); }

    bool render_file(QString file, QString algo);
    bool render_script(QString script, QString algo);

    void render_graph();

    /** properties */
    bool truecolor() const { return truecolor_; }
    void setTruecolor(bool value) { truecolor_ = value; }

    QString imagepath() const { return imagepath_; }
    void setImagepath(QString value) { imagepath_ = value; }

    bool foldNodes() const { return foldNodes_; }
    void setFoldNodes(bool value) { foldNodes_ = value; }

    QString layoutKind() const { return layoutKind_; }
    void setLayoutKind(QString value) { layoutKind_ = value; }

    /** make available to scripting */
    Q_INVOKABLE void show_context_graph_layout(GVC_t* c, Agraph_t *g, QString layout);

    lqXDotScene* scene() const { return qobject_cast<lqXDotScene*>(QGraphicsView::scene()); }

    Cp getContext() const;
    Gp getGraph() const;

    void setFoldedScene(lqXDotScene *s, QPointF nodeOffset);

signals:

    void report_error(QString) const;

protected:

    // allocation factories
    virtual lqContextGraph* factory_cg(GVC_t* context, Agraph_t *graph, lqXDotView* view);
    virtual lqContextGraph* factory_cg(lqXDotView* view);
    virtual lqXDotScene* factory_scene(lqContextGraph* cg);

    // context menu handlers
    virtual void addActionsItem(QMenu *menu, QGraphicsItem *item);
    virtual void addActionsScene(QMenu *menu);

    void keyPressEvent(QKeyEvent* event);
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent *event);

    void contextMenuEvent(QContextMenuEvent *event);

    //! preset layout on selected algorithm
    bool render_layout(QString &err);

    QPointer<QSignalMapper> exportFmt;
    QString lastExportDir;

protected slots:

    void toggleFolding();
    void exportAs(QString fmt);
    void reloadLayout(QString newLayout);

protected:

    //! graphviz rendering context and data
    QPointer<lqContextGraph> cg;

    void setup();

    //! report error to user
    void error(QString msg) const;

    //! scale full scene (code from SvgView)
    void scale_view(qreal scaleFactor);

    // when enabled, accept colors with alpha component
    bool truecolor_;

    // enable folding
    bool foldNodes_;

    // required when displaying images in rendered graph
    QString imagepath_;

    // remember which layout has run
    QString layoutKind_;
};

#endif // LQXDOTVIEW_H
