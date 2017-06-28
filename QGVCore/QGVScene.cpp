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
#include "QGVScene.h"
// The following include allows the automoc to detect, that it must moc this class
#include "moc_QGVScene.cpp"
#include <QDebug>

#include <QGVNode.h>
#include <QGVEdge.h>
#include <QGVSubGraph.h>
#include <qcoreapplication.h>


#include <QGVGraphPrivate.h>
#include <QGVGvcPrivate.h>
#include <QGVEdgePrivate.h>
#include <QGVNodePrivate.h>
#include <iostream>

//#include "QThread"

QGVScene::QGVScene(const QString &name, QObject *parent, int descriptor_idx) : QGraphicsScene(parent)
{
    _context = new QGVGvcPrivate(gvContext());


    switch (descriptor_idx) {
    case 1:  descriptor = Agdirected;         break;
    case 2:  descriptor = Agstrictdirected;   break;
    case 3:  descriptor = Agundirected;       break;
    case 4:  descriptor = Agstrictundirected; break;
    default: descriptor = Agdirected;         break;
    }

    _graph = new QGVGraphPrivate(agopen(name.toLocal8Bit().data(), descriptor, NULL));
    //setGraphAttribute("fontname", QFont().family());
}

QGVScene::~QGVScene()
{
    gvFreeLayout(_context->context(), _graph->graph());
    agclose(_graph->graph());
    gvFreeContext(_context->context());
    delete _graph;
    delete _context;
}

void QGVScene::setGraphAttribute(const QString &name, const QString &value)
{
    agattr(_graph->graph(), AGRAPH, name.toLocal8Bit().data(), value.toLocal8Bit().data());
}

void QGVScene::setNodeAttribute(const QString &name, const QString &value)
{
    agattr(_graph->graph(), AGNODE, name.toLocal8Bit().data(), value.toLocal8Bit().data());
}

void QGVScene::setEdgeAttribute(const QString &name, const QString &value)
{
    agattr(_graph->graph(), AGEDGE, name.toLocal8Bit().data(), value.toLocal8Bit().data());
}

QGVNode *QGVScene::addNode(const QString &label)
{
    Agnode_t *node = agnode(_graph->graph(), NULL, TRUE);
    if(node == NULL)
    {
        qWarning() << "Invalid node :" << label;
        return 0;
    }
    QGVNode *item = new QGVNode(new QGVNodePrivate(node, _graph->graph()), this);
    item->setLabel(label);
    addItem(item);
    _nodes.append(item);
    return item;
}

QGVEdge *QGVScene::addEdge(QGVNode *source, QGVNode *target, const QString &label)
{
    Agedge_t* edge = agedge(_graph->graph(), source->_node->node(), target->_node->node(), NULL, TRUE);
    if(edge == NULL)
    {
        qWarning() << "Invalid egde :" << label;
        return 0;
    }

    QGVEdge *item = new QGVEdge(new QGVEdgePrivate(edge), this);
    item->setLabel(label);
    addItem(item);
    _edges.append(item);
    source->addEdge(item);
    if(!descriptor.directed) //if undirected, add edge to target
        target->addEdge(item);
    return item;
}

QGVSubGraph *QGVScene::addSubGraph(const QString &name, bool cluster)
{
    Agraph_t* sgraph;
    if(cluster)
        sgraph = agsubg(_graph->graph(), ("cluster_" + name).toLocal8Bit().data(), TRUE);
    else
        sgraph = agsubg(_graph->graph(), name.toLocal8Bit().data(), TRUE);

    if(sgraph == NULL)
    {
        qWarning() << "Invalid subGraph :" << name;
        return 0;
    }

    QGVSubGraph *item = new QGVSubGraph(new QGVGraphPrivate(sgraph), this);
    addItem(item);
    _subGraphs.append(item);
    return item;
}

#include "pack.h"

void QGVScene::testPack()
{
    /*
       const int subgraphCount = _subGraphs.size();
       Agraph_t **graphs = new  Agraph_t *[subgraphCount];
       int *nodeCount = new int[subgraphCount];
       boolean **fixed = new boolean*[subgraphCount];

       pack_info *infos  = new pack_info[subgraphCount];
       for (int i = 0; i < subgraphCount; ++i) {
        qDebug() << isConnected(_subGraphs.at(i)->_sgraph->graph());
        graphs[i] = _subGraphs.at(i)->_sgraph->graph();
        qDebug() << "Addresses:" <<  _subGraphs.at(i)->_sgraph->graph();
        nodeCount[i] = _subGraphs.at(i)->_nodes.size();
        fixed[i] = new boolean[nodeCount[i]];
        unsigned int *vals = new unsigned int[nodeCount[i]];
        for (int j = 0; j < nodeCount[i]; ++j) {
            fixed[i][j] = 0;
            vals[j] = j;
        }
        pack_info packInfo;
        packInfo.aspect = 1.f;
        packInfo.doSplines = 1;
        packInfo.fixed = NULL;
        packInfo.flags = PK_COL_MAJOR;
        packInfo.margin = 1;
        packInfo.mode = l_clust;
        packInfo.sz = 4;
        packInfo.vals = vals;
        infos[i] = packInfo;
       }
       int ret = packSubgraphs(subgraphCount, graphs, this->_graph->graph(), infos);
       qDebug() << ret;*/

    Agraph_t **cc;
    Agraph_t *sg;
    int ncc = 0;
    qDebug() << "Graph (TOP) Adresse:" <<  _graph->graph();
    qDebug() << "Context (TOP) Adresse:" <<  _context->context();
    cc = ccomps(_graph->graph(), &ncc, QString("test").toLocal8Bit().data());
    qDebug() << ncc << "FROM CCOMPS!!!!!!";
    for (int i = 0; i < ncc; i++) {
        sg = cc[i];
        qDebug() << "Edges induced: " << nodeInduce (sg);
//        qDebug() << "Subgraph graph address:" << _subGraphs.at(i)->_sgraph->graph();
        qDebug() << "sg address:" << sg;
//        qDebug() << "Subgraph context address:" << _subGraphs.at(i)->_scene->_context->context();
        qDebug() << "Scene context address:" << this->_context->context();
        qDebug() << "layout: " << gvLayout(_context->context(), sg, "neato");
//        qDebug() << _subGraphs.at(i)->_sgraph->graph();
    }
    qDebug() << "Graph Adresse:" <<  _graph->graph();

    //Update items layout
    foreach(QGVNode * node, _nodes)
    node->updateLayout();

    foreach(QGVEdge * edge, _edges)
    edge->updateLayout();

    foreach(QGVSubGraph * sgraph, _subGraphs)
    sgraph->updateLayout();

    //Graph label
    textlabel_t *xlabel = GD_label(_graph->graph());
    if(xlabel)
    {
        QGraphicsTextItem *item = addText(xlabel->text);
        item->setPos( QGVCore::centerToOrigin( QGVCore::toPoint(xlabel->pos, QGVCore::graphHeight(_graph->graph()) ), xlabel->dimen.x, -4) );
    }

//        gvFreeLayout(_context->context(), _graph->graph());

    update();


//    pack_info info = pack_info();
//    getPackInfo(_graph->graph(), l_clust, 20, &info);
//    info.doSplines = 1;
//    info.fixed = 0;
//    int ret = packSubgraphs(ncc, cc, _graph->graph(), &info);
//    qDebug() << "Graph Adresse:" <<  _graph->graph();
//    qDebug() << ret << "FROM packSubgraphs!!!!!!";
//    packGraphs(ncc, cc, _graph->graph(), 0);

    //    for (int i = 0; i < ncc; i++) {
//        sg = cc[i];
//        gvFreeLayout(_context->context(), sg);
//        agdelete(_graph->graph(), sg);
//    }

}

void QGVScene::testPack2(){
//    qDebug() << getGraphAttribute("bb", "");
////    freeLayout();

//    Agraph_t *sg;
//    for (int i = 0; i < _subGraphs.size(); i++) {
//        sg = this->_subGraphs.at(i)->_sgraph->graph();
////        qDebug() << "Edges induced: " << nodeInduce (sg);
//        qDebug() << "sg address:" << sg;
//        qDebug() << "layout: " << gvLayout(_context->context(), sg, "neato");
//    }

//    qDebug() << getGraphAttribute("bb", "");

//    qDebug() << "Graph Adresse:" <<  _graph->graph();

//    //Update items layout
//    foreach(QGVNode * node, _nodes)
//    node->updateLayout();

//    foreach(QGVEdge * edge, _edges)
//    edge->updateLayout();

//    foreach(QGVSubGraph * sgraph, _subGraphs)
//    sgraph->updateLayout();

//    //Graph label
//    textlabel_t *xlabel = GD_label(_graph->graph());
//    if(xlabel)
//    {
//        QGraphicsTextItem *item = addText(xlabel->text);
//        item->setPos( QGVCore::centerToOrigin( QGVCore::toPoint(xlabel->pos, QGVCore::graphHeight(_graph->graph()) ), xlabel->dimen.x, -4) );
//    }

////        gvFreeLayout(_context->context(), _graph->graph());

//    update();


    Agraph_t **cc;
    Agraph_t *root = _graph->graph();
    int ncc = 0;

    cc = ccomps(root, &ncc, QString("test").toLocal8Bit().data());
    qDebug() << "clusters:" << ncc;
    pack_info info;
    pack_mode m = getPackInfo(root, l_clust, 20, &info);
//    info.doSplines = 1;
//    info.fixed = 0;
    int ret = packSubgraphs(ncc, cc, root, &info);
    qDebug() <<  ret;


}

void QGVScene::reRouteEdges()
{
    ///only a test!

    //get all the edges, that need to be redone --> all?
    QList<QGVEdge *> edges;
    edges.append(_edges.at(9));
    edges.append(_edges.at(10));

    //get the nodes, that belong to them
    QList<QGVNode *> nodes;
    nodes.append(_nodes.at(1));
    nodes.append(_nodes.at(2));
    nodes.append(_nodes.at(6));
    nodes.append(_nodes.at(9));


    QStringList nPosIn;
    QPointF p1 = _nodes.at(1)->getPos();
    QPointF p2 = _nodes.at(2)->getPos();
    QPointF p6 = _nodes.at(6)->getPos();
    QPointF p9 = _nodes.at(9)->getPos();
    nPosIn.append(QString::number(p1.x()) + "," + QString::number(p1.y()) + "");
    nPosIn.append(QString::number(p2.x()) + "," + QString::number(p2.y()) + "");
    nPosIn.append(QString::number(p6.x()) + "," + QString::number(p6.y()) + "");
    nPosIn.append(QString::number(p9.x()) + "," + QString::number(p9.y()) + "");

    //create tmpGraph
//    agclose(_graph->graph());
    Agraph_t *g = agopen(QString("tmp").toLocal8Bit().data(), descriptor, NULL);
    Agnode_t *n1 = agnode(g, QString("1").toLocal8Bit().data(), TRUE);
    Agnode_t *n2 = agnode(g, QString("2").toLocal8Bit().data(), TRUE);
    Agnode_t *n6 = agnode(g, QString("6").toLocal8Bit().data(), TRUE);
    Agnode_t *n9 = agnode(g, QString("9").toLocal8Bit().data(), TRUE);

    QList<Agnode_t *> agnodes;
    agnodes.append(n1);
    agnodes.append(n2);
    agnodes.append(n6);
    agnodes.append(n9);

    agsafeset(n1, QString("pos").toLocal8Bit().data(), QString("0.0,0.0").toLocal8Bit().data(), QString("0.0,0.0").toLocal8Bit().data());
    agset(n1, QString("pos").toLocal8Bit().data(), nPosIn.at(0).toLocal8Bit().data());
    agset(n2, QString("pos").toLocal8Bit().data(), nPosIn.at(1).toLocal8Bit().data());
    agset(n6, QString("pos").toLocal8Bit().data(), nPosIn.at(2).toLocal8Bit().data());
    agset(n9, QString("pos").toLocal8Bit().data(), nPosIn.at(3).toLocal8Bit().data());

    qDebug() << QString::fromLocal8Bit(agget(n1, QString("pos").toLocal8Bit().data()));

    agsafeset(n1, QString("pin").toLocal8Bit().data(), QString("true").toLocal8Bit().data(), QString("true").toLocal8Bit().data());
    foreach (Agnode_t * n, agnodes) {
        agset(n, QString("pin").toLocal8Bit().data(), QString("true").toLocal8Bit().data());
    }

    Agedge_t *e12 = agedge(g, n1, n2, QString("1--2").toLocal8Bit().data(), TRUE);
    Agedge_t *e69 = agedge(g, n6, n9, QString("6--9").toLocal8Bit().data(), TRUE);
    agsafeset(e12, QString("splines").toLocal8Bit().data(), QString("spline").toLocal8Bit().data(), QString("spline").toLocal8Bit().data());

    int ret = gvLayout(_context->context(), g, "nop");
    qDebug() << ret;

    int ret2 = gvRender(_context->context(), g, QString("dot").toLocal8Bit().data(), stdout);
    qDebug() << ret2;
    QString pos12 = QString::fromLocal8Bit(agget(e12, QString("pos").toLocal8Bit().data()));
    qDebug() << pos12;

    gvFreeLayout(_context->context(), g);
    agclose(g);

    _edges.at(9)->setAttribute("pos", pos12);
    setGraphAttribute("overlap", "false");
    qDebug() << QString::fromLocal8Bit(agget(_graph->graph(), QString("overlap").toLocal8Bit().data()));

    renderGraph("dot", stdout);
    applyLayout("nop");
    //    agopen(QString("DEMO").toLocal8Bit().data(), descriptor, NULL);

}

QString QGVScene::getGraphAttribute(const QString &name, const QString defaultValue) const
{
    char* value = agget(_graph->graph(), name.toLocal8Bit().data());
    if(value)
        return value;
    return defaultValue;
}

void QGVScene::deleteNode(QGVNode* node)
{
    QList<QGVNode *>::iterator it = std::find(_nodes.begin(), _nodes.end(), node);
    if(it == _nodes.end())
    {
        std::cout << "Error, node not part of Scene" << std::endl;
        return;
    }
    std::cout << "delNode ret " << agdelnode(node->_node->graph(), node->_node->node()) << std::endl;;
    _nodes.erase(it);
    delete node;
}

void QGVScene::deleteEdge(QGVEdge* edge)
{
    std::cout << "delEdge ret " << agdeledge(_graph->graph(), edge->_edge->edge()) << std::endl;
    QList<QGVEdge *>::iterator it = std::find(_edges.begin(), _edges.end(), edge);
    if(it == _edges.end())
    {
        std::cout << "Error, QGVEdge not part of Scene" << std::endl;
        return;
    }
    _edges.erase(it);
    delete edge;
}

void QGVScene::deleteSubGraph(QGVSubGraph *subgraph)
{
    std::cout << "Removing sug " << subgraph->_sgraph->graph() << std::endl;
    std::cout << "delSubg ret " << agclose(subgraph->_sgraph->graph()) << std::endl;
    QList<QGVSubGraph *>::iterator it = std::find(_subGraphs.begin(), _subGraphs.end(), subgraph);
    if(it == _subGraphs.end())
    {
        std::cout << "Error, QGVSubGraph not part of Scene" << std::endl;
        return;
    }
    _subGraphs.erase(it);

    delete subgraph;
}

void QGVScene::setRootNode(QGVNode *node)
{
    Q_ASSERT(_nodes.contains(node));
    char root[] = "root";
    agset(_graph->graph(), root, node->label().toLocal8Bit().data());
}

void QGVScene::loadLayout(const QString &text)
{
    _graph->setGraph(QGVCore::agmemread2(text.toLocal8Bit().constData()));

    if(gvLayout(_context->context(), _graph->graph(), "dot") != 0)
    {
        qCritical() << "Layout render error" << agerrors() << QString::fromLocal8Bit(aglasterr());
        return;
    }
    //Debug output
    //gvRenderFilename(_context->context(), _graph->graph(), "png", "debug.png");

    //Read nodes and edges
    for (Agnode_t* node = agfstnode(_graph->graph()); node != NULL; node = agnxtnode(_graph->graph(), node))
    {
        QGVNode *inode = new QGVNode(new QGVNodePrivate(node, _graph->graph()), this);
        inode->updateLayout();
        addItem(inode);
        for (Agedge_t* edge = agfstout(_graph->graph(), node); edge != NULL; edge = agnxtout(_graph->graph(), edge))
        {
            QGVEdge *iedge = new QGVEdge(new QGVEdgePrivate(edge), this);
            iedge->updateLayout();
            addItem(iedge);
        }

    }
    update();
}

void QGVScene::applyLayout(const QString &engine)
{
    if(label != 0)
        label->deleteLater();
    if(gvLayout(_context->context(), _graph->graph(), engine.toLocal8Bit().data()) != 0)
    {
        /*
         * Si plantage ici :
         *  - Verifier que les dll sont dans le repertoire d'execution
         *  - Verifie que le fichier "configN" est dans le repertoire d'execution !
         */
        qCritical() << "Layout render error" << agerrors() << QString::fromLocal8Bit(aglasterr());
        return;
    }

    //Debug output
    //gvRenderFilename(_context->context(), _graph->graph(), "canon", "debug.dot");
    //gvRenderFilename(_context->context(), _graph->graph(), "png", "debug.png");

    //Update items layout
    foreach(QGVNode * node, _nodes)
    node->updateLayout();

    foreach(QGVEdge * edge, _edges)
    edge->updateLayout();

    foreach(QGVSubGraph * sgraph, _subGraphs)
    sgraph->updateLayout();

    //Graph label
    textlabel_t *xlabel = GD_label(_graph->graph());
    if(xlabel)
    {
        label = addText(xlabel->text);
        label->setPos( QGVCore::centerToOrigin( QGVCore::toPoint(xlabel->pos, QGVCore::graphHeight(_graph->graph()) ), xlabel->dimen.x, -4) );
    }

//    gvFreeLayout(_context->context(), _graph->graph());

    update();
}

void QGVScene::layoutGraphWithSubgraphs(const QString &engine1, const QString &engine2)
{

//    Agraph_t *newGraph = agopen(QString("newgraph").toLocal8Bit().data(), descriptor, NULL);
//    foreach (QGVEdge * e, _edges) {
//        e->setAttribute("splines", "ortho");
//    }
    applyLayout(engine1);
//    QPointF pN1Orig0 = _nodes.first()->getPos();
//    renderGraph("dot", stdout);
    foreach (QGVNode * n, _nodes) {
        n->lockPosition();
    }
    bool ok = true;
    foreach (QGVEdge * e, _edges) {
//        ok &= e->lockPosition();
        e->setAttribute("splines", "true");
    }
    if(!ok) qDebug() << "ERROR LOCKING EDGES!";
//    QPointF pN1Orig1 = _nodes.first()->getPos();

    Agraph_t * *cc;
    Agraph_t *sg;
    int ncc = 0;
    cc = ccomps(_graph->graph(), &ncc, QString("test").toLocal8Bit().data());
    pack_info info;
    info.mode = l_graph;
    getPackModeInfo(_graph->graph(), l_clust, &info);
    int pack = packGraphs(ncc, cc, _graph->graph(), &info);

//    renderGraph("dot", stdout);
    qDebug() << pack;

    freeLayout(false);
    setGraphAttribute("overlap", "9:prism");
    applyLayout(engine2);
    qDebug() << "var";

//    QPointF pN1New0 = _nodes.first()->getPos();
//    QPointF diff0 = pN1Orig0 - pN1New0;
//    qDebug() << "diff0:" << diff0;
    renderGraph("dot", stderr);


//    QPointF pN1New1 = _nodes.first()->getPos();
//    freeLayout(false);
//    qDebug() << pN1Orig1 - pN1New1;
//    qDebug() << "diff1:" << diff1;


    return;

    cc = ccomps(_graph->graph(), &ncc, QString("test").toLocal8Bit().data());
    qDebug() << ncc << "FROM CCOMPS!!!!!!";
    for (int i = 0; i < ncc; i++) {
        sg = cc[i];
        if(sg->parent != 0)
        {
            qDebug() << sg->parent;
            qDebug() << "Edges induced: " << nodeInduce (sg);
            qDebug() << "layout: " << gvLayout(_context->context(), sg, engine1.toLocal8Bit().data());
            gvRender(_context->context(), sg, "dot", stdout);
            gvFreeLayout(_context->context(), sg);
        }
    }
    qDebug() << "out";
//    packGraphs(ncc, cc, _graph->graph(), 0);
//    gvRender(_context->context(), _graph->graph(), "dot", stdout);

}

void QGVScene::freeLayout(bool removeGraphicsItems){

    gvFreeLayout(_context->context(), _graph->graph());
    if(removeGraphicsItems)
    {
        foreach (QGraphicsItem * item, items()) {
            removeItem(item);
        }
    }
}

void QGVScene::renderData(const QString &format, char **result, unsigned int *length){
    gvRenderData(this->_context->context(), this->_graph->graph(), format.toLocal8Bit().data(), result, length);
}

void QGVScene::loadRenderData(char **result, unsigned int *length){
    qDebug() << result;
}

QString QGVScene::versionInfo()
{
    QString ret;
    //info contains 3 char*
    char **info = gvcInfo(_context->context());
    const QString appName = QString::fromLocal8Bit(info[0]);
    const QString versionStr = QString::fromLocal8Bit(info[1]);
    const QString buildDate = QString::fromLocal8Bit(info[2]);

    QString version = versionStr.left(versionStr.indexOf(".", 3));
    QString year = buildDate.left(4);
    QString month = buildDate.mid(4, 2);
    QString day = buildDate.mid(6, 2);

    ret = appName + " v" + version + " built on " + day + "." + month + "." + year;
    return ret;
}

void QGVScene::renderGraph(const QString &format, FILE* out){
    gvRender(this->_context->context(), this->_graph->graph(), format.toLocal8Bit().data(), out);
}

void QGVScene::renderPng(const QString &path){
    /*qDebug() << "render:" << */ gvRenderFilename(_context->context(), _graph->graph(), "png", path.toLocal8Bit().data());
}
void QGVScene::exportAsDotFormat(const QString &path){
    gvRenderFilename(_context->context(), _graph->graph(), "canon", path.toLocal8Bit().data());
}

void QGVScene::clear()
{
    gvFreeLayout(_context->context(), _graph->graph());
    _nodes.clear();
    _edges.clear();
    _subGraphs.clear();
//    QGraphicsScene::clear();
}

void QGVScene::nodeHoverEnter(QGVNode *node)
{
    emit nodeHoverEnterSignal(node);
}

void QGVScene::nodeHoverLeave(QGVNode *node)
{
    emit nodeHoverLeaveSignal(node);
}

#include <QGraphicsSceneContextMenuEvent>
void QGVScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
    QGraphicsItem *item = itemAt(contextMenuEvent->scenePos(), QTransform());
    if(item)
    {
        item->setSelected(true);
        if(item->type() == QGVNode::Type)
            emit nodeContextMenu(qgraphicsitem_cast<QGVNode*>(item));
        else if(item->type() == QGVEdge::Type)
            emit edgeContextMenu(qgraphicsitem_cast<QGVEdge*>(item));
        else if(item->type() == QGVSubGraph::Type)
            emit subGraphContextMenu(qgraphicsitem_cast<QGVSubGraph*>(item));
        else
            emit graphContextMenuEvent();
    }
    QGraphicsScene::contextMenuEvent(contextMenuEvent);
}

void QGVScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
    if(item)
    {
        if(item->type() == QGVNode::Type)
            emit nodeDoubleClick(qgraphicsitem_cast<QGVNode*>(item));
        else if(item->type() == QGVEdge::Type)
            emit edgeDoubleClick(qgraphicsitem_cast<QGVEdge*>(item));
        else if(item->type() == QGVSubGraph::Type)
            emit subGraphDoubleClick(qgraphicsitem_cast<QGVSubGraph*>(item));
    }
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
}

#include <QVarLengthArray>
#include <QPainter>
//#include <qthread.h>
void QGVScene::drawBackground(QPainter * painter, const QRectF & rect)
{
    const int gridSize = 25;

    const qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    const qreal top = int(rect.top()) - (int(rect.top()) % gridSize);

    QVarLengthArray<QLineF, 100> lines;

    for (qreal x = left; x < rect.right(); x += gridSize)
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += gridSize)
        lines.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setRenderHint(QPainter::Antialiasing, false);

    painter->setPen(QColor(Qt::lightGray).lighter(110));
    painter->drawLines(lines.data(), lines.size());
    painter->setPen(Qt::black);
    //painter->drawRect(sceneRect());
}

QList<QGVSubGraph *> QGVScene::getSubGraphs() const
{
    return _subGraphs;
}

QList<QGVEdge *> QGVScene::getEdges() const
{
    return _edges;
}

QList<QGVNode *> QGVScene::getNodes() const
{
    return _nodes;
}

