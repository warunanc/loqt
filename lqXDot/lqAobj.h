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

#ifndef LQAOBJ
#define LQAOBJ

#include <QObject>
#include <QGraphicsItemGroup>
#include "lqXDot_global.h"

/** subclass to get animations working
 */
class LQXDOTSHARED_EXPORT lqItem : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT

    //! animate position
    Q_PROPERTY(QPointF pos READ pos WRITE setPos FINAL)

    //! animate visibility
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity FINAL)

    //! each object has unique name
    Q_PROPERTY(QString name READ name WRITE setName)

public:

    typedef QList<QGraphicsItem *> items;

    //! construct with graphics primitives
    lqItem(QGraphicsScene *s, items objs);

    //! name storage
    QString name() const { return name_; }
    void setName(QString name) { name_ = name; }

protected:

    //! serve itemHasChanged() signal
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    QString name_;

signals:

    void itemHasChanged(QGraphicsItem::GraphicsItemChange change, const QVariant &value);

public slots:

};

/** an animatable node
 */
class LQXDOTSHARED_EXPORT lqNode : public lqItem
{
    Q_OBJECT
public:

    //! construct with graphics primitives
    lqNode(QGraphicsScene *s, items l) : lqItem(s, l) {}

    //! fullfill qgraphics_cast requirements
    enum { Type = UserType + 1 };
    int type() const { return Type; }

signals:

public slots:

};

Q_DECLARE_METATYPE(lqNode*)

/** a recognizable edge
 */
class lqEdge : public lqItem {
    Q_OBJECT
public:

    //! construct with graphics primitives
    lqEdge(QGraphicsScene *s, items l) : lqItem(s, l) {}

    //! fullfill qgraphics_cast requirements
    enum { Type = UserType + 2 };
    int type() const { return Type; }

signals:

public slots:

};
Q_DECLARE_METATYPE(lqEdge*)

/** a recognizable subgraph
 */
class lqGraph : public lqItem {
    Q_OBJECT
public:

    //! construct with graphics primitives
    lqGraph(QGraphicsScene *s, items l) : lqItem(s, l) {}

    //! fullfill qgraphics_cast requirements
    enum { Type = UserType + 3 };
    int type() const { return Type; }

signals:

public slots:

};
Q_DECLARE_METATYPE(lqGraph*)

#endif // LQAOBJ
