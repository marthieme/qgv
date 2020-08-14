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
#include <QGVEdge.h>
#include <QGVCore.h>
#include <QGVScene.h>
#include <QGVGraphPrivate.h>
#include <QGVEdgePrivate.h>
#include <QDebug>
#include <QPainter>

QGVEdge::QGVEdge(QGVEdgePrivate *edge, QGVScene *scene) :  _scene(scene), _edge(edge)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QGVEdge::~QGVEdge()
{
    _scene->removeItem(this);
    delete _edge;
}

QString QGVEdge::label() const
{
    return getAttribute("xlabel");
}

QRectF QGVEdge::boundingRect() const
{
    return _path.boundingRect() | _head_arrow.boundingRect() | _tail_arrow.boundingRect() | _label_rect;
}

QPainterPath QGVEdge::shape() const
{
    QPainterPathStroker ps;
    ps.setCapStyle(_pen.capStyle());
    ps.setWidth(_pen.widthF() + 10);
    ps.setJoinStyle(_pen.joinStyle());
    ps.setMiterLimit(_pen.miterLimit());
    return ps.createStroke(_path);
}

void QGVEdge::setLabel(const QString &label)
{
    setAttribute("xlabel", label);
}

void QGVEdge::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();

    if(isSelected())
    {
        QPen tpen(_pen);
        tpen.setColor(_pen.color().darker(120));
        tpen.setStyle(Qt::DotLine);
        painter->setPen(tpen);
    }
    else
        painter->setPen(_pen);


    painter->drawPath(_path);

    /*
       QRectF pp = _path.controlPointRect();
       if(pp.width() < pp.height())
       {
        painter->save();
        painter->translate(_label_rect.topLeft());
        painter->rotate(90);
        painter->drawText(QRectF(QPointF(0, -_label_rect.width()), _label_rect.size()), Qt::AlignCenter, _label);
        painter->restore();
       }
       else
     */
    painter->drawText(_label_rect, Qt::AlignCenter, _label);

    painter->setBrush(QBrush(_pen.color(), Qt::SolidPattern));
    painter->drawPolygon(_head_arrow);
    painter->drawPolygon(_tail_arrow);
    painter->restore();
}

void QGVEdge::setAttribute(const QString &name, const QString &value)
{
    char empty[] = "";
    agsafeset(_edge->edge(), name.toLocal8Bit().data(), value.toLocal8Bit().data(), empty);
}

QString QGVEdge::getAttribute(const QString &name) const
{
    char* value = agget(_edge->edge(), name.toLocal8Bit().data());
    if(value)
        return value;
    return QString();
}

QVector<QPointF> QGVEdge::getPos()
{
    QVector<QPointF> pts;
    QStringList poss1 = getAttribute("pos").split(" ", QString::SkipEmptyParts);
    QStringList poss;
    foreach (const QString &pos, poss1) {
        poss = poss << pos.split(";", QString::SkipEmptyParts);
    }
    Q_ASSERT(poss.size() > 0);
    if(poss.size() == 0) return QVector<QPointF>();
    bool ok = true;

    foreach (const QString &pos, poss) {
        QStringList l = pos.split(",");
        l[0] = l[0].replace("!", "");
        l[1] = l[1].replace("!", "");
        bool ok1, ok2;
        QPointF pt(l.at(0).toDouble(&ok1), l.at(1).toDouble(&ok2));
        ok &= ok1 & ok2;
        Q_ASSERT(ok);
        pts.append(pt);
    }
    Q_ASSERT(poss.size() == pts.size());
    return pts;
}

void QGVEdge::setPosition(QVector<QPointF> pts, bool lock)
{
    QString posStr;
    const bool spline = pts.size() > 3;

    int count = 0;
    foreach (QPointF pt, pts) {
        count++;
        posStr += QString::number(pt.x()) + "," + QString::number(pt.y()) + " ";
        if(count % 4 == 0)
        {
            posStr.chop(1);
            posStr += ";";
        }

    }
    posStr.chop(1);
    setAttribute("pos", posStr);

    if(lock)
        setAttribute("pin", "true");
}

void QGVEdge::translate(QPointF diff)
{
    QVector<QPointF> pts = getPos();
    if(pts.size() == 0) return;

    QVector<QPointF> ptsTranslated;

    foreach (QPointF pt, pts) {
        ptsTranslated.append(pt + diff);
    }

    setPosition(ptsTranslated);
}

void QGVEdge::updateLayout()
{
    prepareGeometryChange();

    qreal gheight = QGVCore::graphHeight(_scene->_graph->graph());

    const splines* spl = ED_spl(_edge->edge());
    _path = QGVCore::toPath(spl, gheight);


    //Edge arrows
    if((spl != 0) && (spl->list != 0) && (spl->list->size % 3 == 1))
    {
        if(spl->list->sflag)
        {
            _tail_arrow = toArrow(QLineF(QGVCore::toPoint(spl->list->list[0], gheight), QGVCore::toPoint(spl->list->sp, gheight)));
        }

        if(spl->list->eflag)
        {
            _head_arrow = toArrow(QLineF(QGVCore::toPoint(spl->list->list[spl->list->size - 1], gheight), QGVCore::toPoint(spl->list->ep, gheight)));
        }
    }

    _pen.setWidth(1);
    _pen.setColor(QGVCore::toColor(getAttribute("color")));
    _pen.setStyle(QGVCore::toPenStyle(getAttribute("style")));

    //Edge label
    textlabel_t *xlabel = ED_xlabel(_edge->edge());
    if(xlabel)
    {
        _label = xlabel->text;
        _label_rect.setSize(QSize(xlabel->dimen.x, xlabel->dimen.y));
        _label_rect.moveCenter(QGVCore::toPoint(xlabel->pos, QGVCore::graphHeight(_scene->_graph->graph())));
    }

    setToolTip(getAttribute("tooltip"));
}


bool QGVEdge::lockPosition()
{
    QString pos = getAttribute("pos");
    if(pos.isEmpty()) return false;
    pos.replace(" ", "! ");
    pos.append("!");
    setAttribute("pos", pos);
    setAttribute("pin", "true");
    return true;
}

QPolygonF QGVEdge::toArrow(const QLineF &line) const
{
    QLineF n = line.normalVector();
    QPointF o(n.dx() / 3.0, n.dy() / 3.0);

    //Only support normal arrow type
    QPolygonF polygon;
    polygon.append(line.p1() + o);
    polygon.append(line.p2());
    polygon.append(line.p1() - o);

    return polygon;
}
