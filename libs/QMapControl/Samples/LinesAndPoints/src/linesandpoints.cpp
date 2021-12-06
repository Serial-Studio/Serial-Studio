#include "linesandpoints.h"
/*!
 * \example linesandpoints.cpp
 * This application demonstrates how to use Geometry objects and how to add them to a
 * layer.
 *
 * Here are used three different point types:
 *  - One which displays a image
 *  - One which draws a plain circle
 *  - One which uses a QPen to draw a circle
 *  - One which has no markers
 * Then these Points were added to a LineString
 *
 * Also there is a keylistener.
 *
 * You can find this example here: MapAPI/Samples/LinesAndPoints
 * \image html sample_linesandpoints.png "screenshot"
 */

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

LinesAndPoints::LinesAndPoints(QWidget *parent)
    : QWidget(parent)
{
    // the size which the QMapControl should fill
    QSize size = QSize(480, 640);

    mc = new MapControl(size);
    // create layout
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(mc);
    setLayout(layout);

    // create layer
    MapAdapter *mapadapter = new OSMMapAdapter();
    Layer *l = new MapLayer("Custom Layer", mapadapter);

    mc->addLayer(l);

    // create a LineString
    QList<Point *> points;
    // Points with image
    points.append(new ImagePoint(8.259959, 50.001781, "images/bus_stop.png",
                                 "Mainz, Hauptbahnhof", Point::BottomLeft));
    points.append(new ImagePoint(8.263758, 49.998917, "images/bus_stop.png",
                                 "Mainz, Münsterplatz", Point::BottomLeft));
    points.append(new ImagePoint(8.265812, 50.001952, "images/bus_stop.png",
                                 "Mainz, Neubrunnenplatz", Point::BottomLeft));
    // Points with a circle
    points.append(
        new CirclePoint(8.2688, 50.004015, "Mainz, Bauhofstraße LRP", Point::Middle));
    points.append(new CirclePoint(8.272845, 50.00495, "Mainz, Landtag", Point::Middle));
    points.append(
        new CirclePoint(8.280349, 50.008173, "Mainz, Brückenkopf", Point::Middle));
    // A QPen can be used to customize the
    QPen *pointpen = new QPen(QColor(0, 255, 0));
    pointpen->setWidth(3);
    points.append(new CirclePoint(8.273573, 50.016315, 15,
                                  "Wiesbaden-Mainz-Kastel, Eleonorenstraße",
                                  Point::Middle, pointpen));
    points.append(new CirclePoint(8.275145, 50.016992, 15,
                                  "Wiesbaden-Mainz-Kastel, Johannes-Goßner-Straße",
                                  Point::Middle, pointpen));
    points.append(new CirclePoint(8.270476, 50.021426, 15,
                                  "Wiesbaden-Mainz-Kastel, Ruthof", Point::Middle,
                                  pointpen));
    // "Blind" Points
    points.append(
        new Point(8.266445, 50.025913, "Wiesbaden-Mainz-Kastel, Mudra Kaserne"));
    points.append(
        new Point(8.260378, 50.030345, "Wiesbaden-Mainz-Amoneburg, Dyckerhoffstraße"));

    // A QPen also can use transparency
    QPen *linepen = new QPen(QColor(0, 0, 255, 100));
    linepen->setWidth(5);
    // Add the Points and the QPen to a LineString
    LineString *ls = new LineString(points, "Busline 54", linepen);

    // Add the LineString to the layer
    l->addGeometry(ls);

    // Connect click events of the layer to this object
    connect(l, SIGNAL(geometryClicked(Geometry *, QPoint)), this,
            SLOT(geometryClicked(Geometry *, QPoint)));

    // Sets the view to the interesting area
    mc->setView(QPointF(8.259959, 50.001781));
    mc->setZoom(11);

    addZoomButtons();
}

void LinesAndPoints::addZoomButtons()
{
    // create buttons as controls for zoom
    QPushButton *zoomin = new QPushButton("+");
    QPushButton *zoomout = new QPushButton("-");
    zoomin->setMaximumWidth(50);
    zoomout->setMaximumWidth(50);

    connect(zoomin, SIGNAL(clicked(bool)), mc, SLOT(zoomIn()));
    connect(zoomout, SIGNAL(clicked(bool)), mc, SLOT(zoomOut()));

    // add zoom buttons to the layout of the MapControl
    QVBoxLayout *innerlayout = new QVBoxLayout;
    innerlayout->addWidget(zoomin);
    innerlayout->addWidget(zoomout);
    mc->setLayout(innerlayout);
}

void LinesAndPoints::geometryClicked(Geometry *geom, QPoint)
{
    qDebug() << "parent: " << geom->parentGeometry();
    qDebug() << "Element clicked: " << geom->name();
    if (geom->hasClickedPoints())
    {
        QList<Geometry *> pp = geom->clickedPoints();
        qDebug() << "number of child elements: " << pp.size();
        for (int i = 0; i < pp.size(); ++i)
        {
            QMessageBox::information(this, geom->name(), pp.at(i)->name());
        }
    }
    else if (geom->GeometryType == "Point")
    {
        QMessageBox::information(this, geom->name(), "just a point");
    }
}

LinesAndPoints::~LinesAndPoints() { }

void LinesAndPoints::resizeEvent(QResizeEvent *qEvent)
{
    Q_UNUSED(qEvent);
    if (mc)
    {
        mc->resize(size());
    }
}

void LinesAndPoints::keyPressEvent(QKeyEvent *evnt)
{
    if (evnt->key() == 49 || evnt->key() == 17825792) // tastatur '1'
    {
        mc->zoomIn();
    }
    else if (evnt->key() == 50)
    {
        mc->moveTo(QPointF(8.25, 60));
    }
    else if (evnt->key() == 51 || evnt->key() == 16777313) // tastatur '3'
    {
        mc->zoomOut();
    }
    else if (evnt->key() == 54) // 6
    {
        mc->setView(QPointF(8, 50));
    }
    else if (evnt->key() == 16777234) // left
    {
        mc->scrollLeft();
    }
    else if (evnt->key() == 16777236) // right
    {
        mc->scrollRight();
    }
    else if (evnt->key() == 16777235) // up
    {
        mc->scrollUp();
    }
    else if (evnt->key() == 16777237) // down
    {
        mc->scrollDown();
    }
    else if (evnt->key() == 48 || evnt->key() == 17825797) // 0
    {
        emit(close());
    }
    else
    {
        qDebug() << evnt->key() << endl;
    }
}
