#include "EnvireGraph2DStructureVisualization.hpp"
#include <QGraphicsScene>
#include "QZoomableGraphicsView.hpp"
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <gvc.h>
#include <envire_core/graph/GraphViz.hpp>
#include <sstream>

using namespace envire::core;

EnvireGraph2DStructureVisualization::EnvireGraph2DStructureVisualization(QWidget *parent)
    : QWidget(parent)
{
    resize(300,120);
    
    scene = new QGraphicsScene();
    QZoomableGraphicsView* view = new QZoomableGraphicsView(scene);    
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(view);
    setLayout(vbox);
    
    QFile f("/home/dfki.uni-bremen.de/aboeckmann/git/rock-entern/envire/envire_core/build/test/envireGraph_complex_graphviz_test.dot");
    if (!f.open(QFile::ReadOnly | QFile::Text)) return;
    QTextStream in(&f);
    displayGraph(in.readAll());    
    show();
}

void EnvireGraph2DStructureVisualization::displayGraph(const QString& dotGraph)
{
    //see: http://www.graphviz.org/doc/libguide/libguide.pdf
    const QByteArray data = dotGraph.toAscii();
    Agraph_t* graph = agmemread(data.constData());
    GVC_t* gvc = gvContext();
    gvLayout(gvc, graph, "dot");
    char* svg;
    unsigned int svgSize = 0;
    gvRenderData (gvc, graph, "svg", &svg, &svgSize);
    
    QByteArray qSvg(svg, svgSize);
    
    gvFreeRenderData(svg);
    gvFreeLayout(gvc, graph);
    agclose(graph);
    gvFreeContext(gvc);
    
    QSvgRenderer* renderer = new QSvgRenderer(qSvg);
    QGraphicsSvgItem *item = new QGraphicsSvgItem();
    item->setSharedRenderer(renderer);
    scene->clear();
    scene->addItem(item);
}

void EnvireGraph2DStructureVisualization::displayGraph(const envire::core::EnvireGraph& graph)
{
    std::stringstream stream;
    GraphViz::write(graph, stream);
    displayGraph(QString::fromStdString(stream.str()));
}

EnvireGraph2DStructureVisualization::~EnvireGraph2DStructureVisualization()
{
}