#include "mapviewer.h"
Mapviewer::Mapviewer(QWidget *parent)
    : QMainWindow(parent)
{
    // create MapControl
    mc = new MapControl(QSize(380, 540));
    mc->showScale(true);

    // create mapadapter, for mainlayer and overlay
    mapadapter = new EmptyMapAdapter(512);

    // create a layer with the mapadapter and type MapLayer
    mainlayer = new MapLayer("", mapadapter);

    // add Layer to the MapControl
    mc->addLayer(mainlayer);

    addZoomButtons();

    // show mapcontrol in mainwindow
    setCentralWidget(mc);

    FixedImageOverlay* fip = new FixedImageOverlay(-36, 66, 37, 23, QCoreApplication::applicationDirPath() + "/sample.png");

    mc->setView(QPointF(10,50));
    mc->zoomIn();

    mainlayer->addGeometry(fip);

    connect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
			  this, SLOT(coordinateClicked(const QMouseEvent*, const QPointF)));

}
void Mapviewer::addZoomButtons()
{
    // create buttons as controls for zoom
    QPushButton* zoomin = new QPushButton("+");
    QPushButton* zoomout = new QPushButton("-");
    zoomin->setMaximumWidth(50);
    zoomout->setMaximumWidth(50);

    connect(zoomin, SIGNAL(clicked(bool)),
            mc, SLOT(zoomIn()));
    connect(zoomout, SIGNAL(clicked(bool)),
            mc, SLOT(zoomOut()));

    // add zoom buttons to the layout of the MapControl
    QVBoxLayout* innerlayout = new QVBoxLayout;
    innerlayout->addWidget(zoomin);
    innerlayout->addWidget(zoomout);
    mc->setLayout(innerlayout);
}


Mapviewer::~Mapviewer()
{
}

// resize the widget
void Mapviewer::resizeEvent ( QResizeEvent * event )
{
    mc->resize(event->size());
}

void Mapviewer::coordinateClicked(const QMouseEvent * evnt, const QPointF coordinate)
{
    if (evnt->type()==QEvent::MouseButtonPress)
    {
	qDebug() << coordinate << ": " << evnt->x() << " / " << evnt->y();
    }
}
