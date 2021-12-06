#include "multidemo.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

/*!
 * \example multidemo.cpp
 * This is a bit complexer application, which lets you play around.
 * There are the following buttons configured:
 *  - Add Point: adds a Point to the coordinate you click (this point will be clickable)
 *  - Drag Rect: lets to drag a rectangular into which will be zoomed in
 *  - Move To Click: moves the view middle to the clicked coordinate
 *  - GPS: starts a "pseudo" GPS receiver which emits new positions, these are connected to the ImagePoint
 *  - Follow Geom: Follows the ImagePoint, when it moves because of new GPS positions
 *
 * A overview map lefts you see where you are. You can even click on it to change your position.
 *
 * You can find this example here: MapAPI/Samples/Multimap
 * \image html sample_multidemo.png "screenshot"
 */
Multidemo::Multidemo(QWidget *parent)
	: QWidget(parent)
{
	setupMaps();
	createLayout();

	gm = new GPS_Modul();
	connect(gm, SIGNAL(new_position(QPointF)),
			  ip, SLOT(setCoordinate(QPointF)));

}

void Multidemo::setupMaps()
{
	QSize size = QSize(480,640);

	// main map control
	mc = new MapControl(size);
	MapAdapter* mapadapter = new WMSMapAdapter("www2.demis.nl", "/wms/wms.asp?wms=WorldMap&LAYERS=Countries,Borders,Cities,Rivers,Settlements,Hillshading,Waterbodies,Railroads,Highways,Roads&FORMAT=image/png&VERSION=1.1.1&SERVICE=WMS&REQUEST=GetMap&STYLES=&EXCEPTIONS=application/vnd.ogc.se_inimage&SRS=EPSG:4326&TRANSPARENT=FALSE", 256);

	// maplayer
	Layer* l = new MapLayer("Custom Layer", mapadapter);
	mc->addLayer(l);
	// Geometry layer
	Layer* l2 = new GeometryLayer("Geom Layer", mapadapter);
	mc->addLayer(l2);


	// "minimap" control
	mc2 = new MapControl(QSize(150,150), MapControl::None);
	MapAdapter* mapadapter_mini = new OSMMapAdapter();
	Layer* layer_mini = new MapLayer("Custom Layer", mapadapter_mini);
	mc2->addLayer(layer_mini);

	// create points
	QPen* pen = new QPen(QColor(255, 0, 0, 100));
	pen->setWidth(5);
	QList<Point*> points;
	points.append(new CirclePoint(8.259959, 50.001781,	"Mainz, Hauptbahnhof", Point::Middle, pen));
	points.append(new CirclePoint(8.263758, 49.998917,	"Mainz, Münsterplatz", Point::Middle, pen));
	points.append(new CirclePoint(8.265812, 50.001952,	"Mainz, Neubrunnenplatz", Point::Middle, pen));
	points.append(new CirclePoint(8.2688, 50.004015,	"Mainz, Bauhofstraße LRP", Point::Middle, pen));
	points.append(new CirclePoint(8.272845, 50.00495,	"Mainz, Landtag", Point::Middle, pen));
	points.append(new CirclePoint(8.272845, 50.00495,	"Mainz, Brückenplatz", Point::Middle, pen));
	points.append(new CirclePoint(8.280349, 50.008173,	"Mainz, Brückenkopf", Point::Middle, pen));
	points.append(new CirclePoint(8.273573, 50.016315,	"Wiesbaden-Mainz-Kastel, Eleonorenstraße", Point::Middle, pen));
	points.append(new CirclePoint(8.275145, 50.016992,	"Wiesbaden-Mainz-Kastel, Johannes-Goßner-Straße", Point::Middle, pen));
	points.append(new CirclePoint(8.270476, 50.021426,	"Wiesbaden-Mainz-Kastel, Ruthof", Point::Middle, pen));
	points.append(new CirclePoint(8.266445, 50.025913,	"Wiesbaden-Mainz-Kastel, Mudra Kaserne", Point::Middle, pen));
	points.append(new CirclePoint(8.260378, 50.030345,	"Wiesbaden-Mainz-Amoneburg, Dyckerhoffstraße", Point::Middle, pen));

	// add points to linestring
	pen = new QPen(QColor(0, 0, 255, 100));
	pen->setWidth(5);
	LineString* ls = new LineString(points, "Busline 54", pen);
	// the linestring is added to the MapLayer l, since it doenst change its points
	l->addGeometry(ls);

	// this point receives position changes from the "gps modul"
	ip = new ImagePoint(0,0, QCoreApplication::applicationDirPath() + "/images/marker1.png", "image point", Point::TopRight);

	// so if have to be added to the GeometryLayer l2
	l2->addGeometry(ip);
	QPushButton* pb = new QPushButton("test button", mc);

	// widget example
	Point* wpoint = new  Point(-20,-20, pb, ".", Point::TopLeft);
	wpoint->setBaselevel(3);
 	l->addGeometry(wpoint);
	pb->setGeometry(0,0,100,50);

	connect(l, SIGNAL(geometryClicked(Geometry*, QPoint)),
			  this, SLOT(geometryClickEvent(Geometry*, QPoint)));
	connect(l2, SIGNAL(geometryClicked(Geometry*, QPoint)),
			  this, SLOT(geometryClickEvent(Geometry*, QPoint)));
	connect(mc, SIGNAL(boxDragged(const QRectF)),
			  this, SLOT(draggedRect(QRectF)));
	connect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
			  this, SLOT(mouseEventCoordinate(const QMouseEvent*, const QPointF)));
	connect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
			  this, SLOT(coordinateClicked(const QMouseEvent*, const QPointF)));
	connect(mc2, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
			  this, SLOT(coordinateClicked_mc2(const QMouseEvent*, const QPointF)));
}

void Multidemo::createLayout()
{
	btn1 = new QPushButton("Add Point");
	btn1->setCheckable(true);
	btn1->setMaximumWidth(80);
	btn1->setMaximumHeight(20);
	btn1->setFont(QFont("Verdana", 5));

	btn2 = new QPushButton("Drag Rect");
	btn2->setCheckable(true);
	btn2->setMaximumHeight(20);
	btn2->setFont(QFont("Verdana", 5));
	btn2->setMaximumWidth(80);

	btn3 = new QPushButton("Move to Click");
	btn3->setCheckable(true);
	btn3->setMaximumHeight(20);
	btn3->setFont(QFont("Verdana", 5));
	btn3->setMaximumWidth(80);

	btn4 = new QPushButton("Follow Geom");
	btn4->setCheckable(true);
	btn4->setMaximumHeight(20);
	btn4->setFont(QFont("Verdana", 5));
	btn4->setMaximumWidth(80);

	btn5 = new QPushButton("GPS");
	btn5->setCheckable(true);
	btn5->setMaximumHeight(20);
	btn5->setFont(QFont("Verdana", 5));
	btn5->setMaximumWidth(80);
	btn1->setFocusPolicy(Qt::NoFocus);
	btn2->setFocusPolicy(Qt::NoFocus);
	btn3->setFocusPolicy(Qt::NoFocus);
	btn4->setFocusPolicy(Qt::NoFocus);
	btn5->setFocusPolicy(Qt::NoFocus);

	QHBoxLayout* layout = new QHBoxLayout;
	QVBoxLayout* layoutinner = new QVBoxLayout;

	layoutinner->addWidget(mc2);
	layoutinner->addWidget(btn1);
	layoutinner->addWidget(btn2);
	layoutinner->addWidget(btn3);
	layoutinner->addWidget(btn4);
	layoutinner->addWidget(btn5);
	layoutinner->addSpacing(70);
	layout->addLayout(layoutinner);

	QHBoxLayout* mclayout = new QHBoxLayout;
	mclayout->addWidget(mc);
	mclayout->setMargin(0);
	setLayout(mclayout);

	mc->setLayout(layoutinner);

	connect(btn2, SIGNAL(toggled( bool )),
			  this, SLOT(buttonToggled(bool)));

	connect(btn4, SIGNAL(toggled( bool )),
			  this, SLOT(toggleFollow(bool)));

	connect(btn5, SIGNAL(toggled( bool )),
			  this, SLOT(toggleGPS(bool)));
}

void Multidemo::coordinateClicked(const QMouseEvent* evnt, const QPointF coord)
{
	if (btn1->isChecked() && evnt->type()==QEvent::MouseButtonPress)
	{
		mc->layer("Geom Layer")->addGeometry(new CirclePoint(coord.x(), coord.y(), 10, "added point"));
		mc->updateRequestNew();
	}
}

void Multidemo::geometryClickEvent(Geometry* geom, QPoint)
{
	if (geom->hasClickedPoints())
	{
        QList<Geometry*> pp = geom->clickedPoints();
        for (int i=0; i<pp.size(); ++i)
		{
			QMessageBox::information(this, geom->name(), pp.at(i)->name());
		}
	}
	else if (geom->GeometryType == "Point")
	{
		QMessageBox::information(this, geom->name(), QString("Position: ").append(QString().setNum(((Point*)geom)->longitude())).append(QString("/")).append(QString().setNum(((Point*)geom)->latitude())));
	}

}

Multidemo::~Multidemo()
{
}

void Multidemo::keyPressEvent(QKeyEvent* evnt)
{
	if (evnt->key() == 49 || evnt->key() == 17825792)  // keyboard '1'
	{
		mc->zoomIn();
	}
	else if (evnt->key() == 50)
	{
		mc->moveTo(QPointF(8.25, 60));
	}
	else if (evnt->key() == 51 || evnt->key() == 16777313)     // keyboard '3'
	{
		mc->zoomOut();
	}
	else if (evnt->key() == 52)	//4
	{
		mc->updateRequestNew();
	}
	else if (evnt->key() == 16777234) // left
	{
		mc->scrollLeft();
	}
	else if (evnt->key() == 16777236) // right
	{
		mc->scrollRight();
	}
	else if (evnt->key() == 16777235 ) // up
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
}

void Multidemo::resizeEvent(QResizeEvent *event)
{
    mc->resize(event->size());
}

void Multidemo::buttonToggled(bool active)
{
	if (active)
		mc->setMouseMode(MapControl::Dragging);
	else
		mc->setMouseMode(MapControl::Panning);
}
void Multidemo::toggleFollow(bool follow)
{
	if (follow)
		mc->followGeometry(ip);
	else
		mc->stopFollowing(ip);
}
void Multidemo::toggleGPS(bool gps)
{
	if (gps)
		gm->start();
	else
		gm->stop();

}

void Multidemo::draggedRect(QRectF rect)
{
	QList<QPointF> coords;
	coords.append(rect.topLeft());
	coords.append(rect.bottomRight());
	mc->setViewAndZoomIn(coords);
}

void Multidemo::mouseEventCoordinate(const QMouseEvent* evnt, const QPointF coordinate)
{
	if (evnt->type() == QEvent::MouseButtonPress && btn3->isChecked())
	{
		mc->moveTo(coordinate);
	}
	//update mini-window
	else if(evnt->type() == QEvent::MouseButtonRelease)
	{
		mc2->setView(mc->currentCoordinate());
	}
}
void Multidemo::coordinateClicked_mc2(const QMouseEvent* evnt, const QPointF coordinate)
{
	if (evnt->type() == QEvent::MouseButtonPress)
	{
		mc2->moveTo(coordinate);
		mc->setView(coordinate);
	}
}
