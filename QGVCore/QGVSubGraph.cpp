/***************************************************************
   QGVCore
   Copyright (c) 2014, Bergont Nicolas, All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3.0 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.
***************************************************************/
#include "QGVEdge.h"
#include "QGVSubGraph.h"
#include <QGVCore.h>
#include <QGVScene.h>
#include <QGVGraphPrivate.h>
#include <QGVNodePrivate.h>
#include <QGVNode.h>
#include <QDebug>
#include <QPainter>

QGVSubGraph::QGVSubGraph(QGVGraphPrivate *subGraph, QGVScene *scene) :  _scene(scene), _sgraph(subGraph)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QGVSubGraph::~QGVSubGraph()
{
    _scene->removeItem(this);
    delete _sgraph;
}

QString QGVSubGraph::name() const
{
    return QString::fromLocal8Bit(GD_label(_sgraph->graph())->text);
}

QGVNode *QGVSubGraph::addNode(const QString &label)
{
    Agnode_t *node = agnode(_sgraph->graph(), NULL, TRUE);
    if(node == NULL)
    {
        qWarning() << "Invalid sub node :" << label;
        return 0;
    }
    agsubnode(_sgraph->graph(), node, TRUE);

    QGVNode *item = new QGVNode(new QGVNodePrivate(node, _sgraph->graph()), _scene);
    item->setLabel(label);
    _scene->addItem(item);
    _scene->_nodes.append(item);
    _nodes.append(item);
    return item;
}

QGVSubGraph *QGVSubGraph::addSubGraph(const QString &name, bool cluster)
{
    Agraph_t* sgraph;
    if(cluster)
        sgraph = agsubg(_sgraph->graph(), ("cluster_" + name).toLocal8Bit().data(), TRUE);
    else
        sgraph = agsubg(_sgraph->graph(), name.toLocal8Bit().data(), TRUE);

    if(sgraph == NULL)
    {
        qWarning() << "Invalid subGraph :" << name;
        return 0;
    }

    QGVSubGraph *item = new QGVSubGraph(new QGVGraphPrivate(sgraph), _scene);
    _scene->_subGraphs.append(item);
    _scene->addItem(item);
    return item;
}

QRectF QGVSubGraph::boundingRect() const
{
    return QRectF(0, 0, _width, _height);
}

void QGVSubGraph::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();
    painter->setPen(_pen);
    painter->setBrush(_brush);

    painter->drawRect(boundingRect());
    painter->drawText(_label_rect, Qt::AlignCenter, _label);
    painter->restore();
}

void QGVSubGraph::setAttribute(const QString &name, const QString &value)
{
    char empty[] = "";
    agsafeset(_sgraph->graph(), name.toLocal8Bit().data(), value.toLocal8Bit().data(), empty);
}

QString QGVSubGraph::getAttribute(const QString &name, const QString &defaultValue) const
{
    char* value = agget(_sgraph->graph(), name.toLocal8Bit().data());
    if(value)
        return value;
    return defaultValue;
}

void QGVSubGraph::getPen(const QString &style){
    if(style == "solid")
        _pen.setStyle(Qt::SolidLine);
    if(style == "dashed")
        _pen.setStyle(Qt::DashLine);
    if(style == "dotted")
    {
        _pen.setStyle(Qt::DotLine);
        _pen.setJoinStyle(Qt::RoundJoin);
        _pen.setCapStyle(Qt::RoundCap);
    }
    if(style == "bold") {
        _pen.setWidth(4);
    } else {
        _pen.setWidth(1);
    }
}

QVariant QGVSubGraph::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemPositionHasChanged && !changeActive)
    {
        changeActive = true;

        boxf box = GD_bb(_sgraph->graph());
        if(((box.UR.x - box.LL.x) == _width) && ((box.UR.y - box.LL.y) == _height)) //only works one time
        {
//        //SubGraph box
//        boxf box = GD_bb(_sgraph->graph());
//        pointf p1 = box.UR;
//        pointf p2 = box.LL;
//        _width = p1.x - p2.x;
//        _height = p1.y - p2.y;

//        qreal gheight = QGVCore::graphHeight(_scene->_graph->graph());
//        setPos(p2.x, gheight - p1.y);

            QPointF newPos = value.toPointF();

            QPointF gvLL;
            QPointF gvUR;
            QPointF diff = lastPos - newPos;

            //alte version:
//        QPointF gvDiff;
//        gvLL.setX(newPos.x());
//        gvLL.setY(box.LL.y - diff.y());
//        gvUR.setX(newPos.x() + _width);
//        gvUR.setY(box.UR.y - diff.y());
//        gvDiff.setX(box.LL.x - gvLL.x());
//        gvDiff.setY(box.LL.y - gvLL.y());

            //neue version
            QPointF gvDiff;
            gvLL.setX(newPos.x());
            gvLL.setY(box.LL.y + diff.y() - (_height / 2.) );
            gvUR.setX(newPos.x() + _width);
            gvUR.setY(box.UR.y + diff.y() + (_height / 2.) );
            gvDiff.setX(box.LL.x - gvLL.x());
            gvDiff.setY(box.LL.y - gvLL.y());

            //set bounding box for graphviz
            setAttribute("bb", QString::number(gvLL.x(), 'G', 8) + "," + QString::number(gvLL.y(), 'G', 8) + "," + QString::number(gvUR.x(), 'G', 8) + "," + QString::number(gvUR.y(), 'f', 8));

            //change label position
            textlabel_t *label = GD_label(_sgraph->graph());
            QString lp = getAttribute("lp", "");
            if(!lp.isEmpty())
            {
                QStringList lpStrL = lp.split(",");
                bool ok = true;
                bool ok1;
                double lpx1 = lpStrL.at(0).toDouble(&ok1);
                ok &= ok1;
                double lpy1 = lpStrL.at(1).toDouble(&ok1);
                ok &= ok1;
                if(ok) {
                    double lpx = lpx1 + gvDiff.x();
                    double lpy = lpy1 + gvDiff.y();


                    lp = QString::number(lpx, 'G', 8) + "!," + QString::number(lpy, 'G', 8);
                    agset(_sgraph->graph(), QString("lp").toLocal8Bit().data(),  lp.toLocal8Bit().data());
                }
                label->pos;
            }

            //        QSet<QGVEdge *> edgesWithinCluster;
            foreach (QGVNode * n, _nodes) {
                QPointF nPos = n->getPos();
                n->setPosition(nPos, true); //test
//            QPointF newnPos =  nPos + gvDiff;
//            n->setPosition(newnPos, true);

//            foreach (QGVEdge * e, n->edges) {
//                if(edgesWithinCluster.contains(e))
//                {
//                    e->translate(gvDiff);
//                } else {
//                    edgesWithinCluster.insert(e); //edges that are inserted but not locked, are edges that connect to clusters
//                }
//            }
            }
            lastPos = newPos;
            _scene->applyLayout("nop");

//        _scene->reRouteEdges();
            _scene->freeLayout(false);
        }
        changeActive = false;
    }
    return value;
}

void QGVSubGraph::updateLayout()
{
    const bool sendChanges = flags().testFlag(QGraphicsItem::ItemSendsGeometryChanges);

    if(sendChanges)
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    prepareGeometryChange();

    //SubGraph box
    boxf box = GD_bb(_sgraph->graph());
    pointf p1 = box.UR;
    pointf p2 = box.LL;
    _width = p1.x - p2.x;
    _height = p1.y - p2.y;

    qreal gheight = QGVCore::graphHeight(_scene->_graph->graph());
    setPos(p2.x, gheight - p1.y);
    lastPos.setX(p2.x);
    lastPos.setY(gheight - p1.y);
//    _pen.setWidth(1);
//    _brush.setStyle(QGVCore::toBrushStyle(getAttribute("style", "filled")));
//    _brush.setColor(QGVCore::toColor(getAttribute("fillcolor", "gray")));
//    _pen.setColor(QGVCore::toColor(getAttribute("color", "black")));
    getPen(getAttribute("style", "solid"));
    //SubGraph label
    textlabel_t *xlabel = GD_label(_sgraph->graph());
    if(xlabel)
    {
        _label = xlabel->text;
        _label_rect.setSize(QSize(xlabel->dimen.x, xlabel->dimen.y));
        _label_rect.moveCenter(QGVCore::toPoint(xlabel->pos, QGVCore::graphHeight(_scene->_graph->graph())) - pos());
    }

    if(sendChanges)
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}
