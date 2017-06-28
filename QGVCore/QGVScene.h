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
#ifndef QGVSCENE_H
#define QGVSCENE_H

#include "qgv.h"
#include <QGraphicsScene>
#include <private/QGVCore.h>

class QGVNode;
class QGVEdge;
class QGVSubGraph;

class QGVGraphPrivate;
class QGVGvcPrivate;

/**
 * @brief GraphViz interactive scene
 *
 */
class QGVCORE_EXPORT QGVScene : public QGraphicsScene
{
    Q_OBJECT
public:

    explicit QGVScene(const QString &name, QObject *parent = 0, int descriptor_idx = 0);
    ~QGVScene();

    void setGraphAttribute(const QString &name, const QString &value);
    void setNodeAttribute(const QString &name, const QString &value);
    void setEdgeAttribute(const QString &name, const QString &value);

    QGVNode* addNode(const QString& label);
    QGVEdge* addEdge(QGVNode* source, QGVNode* target, const QString& label=QString());
    QGVSubGraph* addSubGraph(const QString& name, bool cluster=true);
    void testPack();
    void deleteNode(QGVNode *node);
    void deleteEdge(QGVEdge *edge);
    void deleteSubGraph(QGVSubGraph *subgraph);

    void setRootNode(QGVNode *node);

    void loadLayout(const QString &text);
    void applyLayout(const QString &engine = "dot");
    void layoutGraphWithSubgraphs(const QString &engine1 = "neato",const QString &engine2 = "fdp");
    void clear();

    void nodeHoverEnter(QGVNode *node);
    void nodeHoverLeave(QGVNode *node);

    void renderPng(const QString &path);
    void exportAsDotFormat(const QString &path);
    void freeLayout(bool removeGraphicsItems = true);

    void testPack2();
    void reRouteEdges();
    QString getGraphAttribute(const QString &name, const QString defaultValue) const;
    void renderGraph(const QString &format, FILE *out = NULL);
    void renderData(const QString &format, char **result, unsigned int *length);
    void loadRenderData(char **result, unsigned int *length);
    QString versionInfo();

    QList<QGVNode *> getNodes() const;

    QList<QGVEdge *> getEdges() const;

    QList<QGVSubGraph *> getSubGraphs() const;

signals:

    void nodeContextMenu(QGVNode* node);
    void nodeDoubleClick(QGVNode* node);

    void edgeContextMenu(QGVEdge* edge);
    void edgeDoubleClick(QGVEdge* edge);

    void subGraphContextMenu(QGVSubGraph* graph);
    void subGraphDoubleClick(QGVSubGraph* graph);

    void graphContextMenuEvent();

    void nodeHoverEnterSignal(QGVNode *node);
    void nodeHoverLeaveSignal(QGVNode *node);

protected:

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent * contextMenuEvent);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent);
    virtual void drawBackground(QPainter * painter, const QRectF & rect);

private:

    friend class QGVNode;
    friend class QGVEdge;
    friend class QGVSubGraph;

    Agdesc_t descriptor;
    QGVGvcPrivate *_context;
    QGVGraphPrivate *_graph;
    //QFont _font;

    QList<QGVNode*> _nodes;
    QList<QGVEdge*> _edges;
    QList<QGVSubGraph*> _subGraphs;

    QGraphicsTextItem *label = 0;
};

#endif // QGVSCENE_H
