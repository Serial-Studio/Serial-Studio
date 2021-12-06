/*!
 * \example citymap.cpp
 * This demo appclication shows more features of the QMapControl.
 * It shows images, which changes its size when changing the zoomlevel.
 * You can display/hide layers and choose different map providers.
 * Also it demonstrates more possibilities for user interaction:<br/>
 * - notes can be added to any coordinate (a QTextEdit is used for editing the note)<br/>
 * - the user can measure distances between two points
 * 
 * \image html sample_citymap.png "screenshot"
 */

#include "citymap.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QApplication>
#include <QPushButton>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QTimer>

Citymap::Citymap(QWidget*)
{
	// create MapControl
	mc = new MapControl(QSize(380,540));
    mc->showScale(true);
	// display the MapControl in the application
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(mc);
    layout->setContentsMargins(0,0,0,0);
	
	QWidget* w = new QWidget();
	w->setLayout(layout);
	setCentralWidget(w);
	
    notepixmap = new QPixmap(QApplication::applicationDirPath() + "/images/note.png");
	
	coord1 = QPointF();
	coord2 = QPointF();
	mapadapter = new OSMMapAdapter();

	// create a layer with the mapadapter and type MapLayer
	l = new MapLayer("Custom Layer", mapadapter);

	mc->addLayer(l);
		
	notes = new GeometryLayer("Notes", mapadapter);

	createTours();
	addSights();
	addPubs();
	addMuseums();
	
	addZoomButtons();
	createActions();
	createMenus();

    connect(mc, SIGNAL(viewChanged(QPointF,int)), this, SLOT(mapControlZoomChanged(QPointF,int)), Qt::QueuedConnection);
	
	mc->addLayer(notes);
	connect(notes, SIGNAL(geometryClicked(Geometry*, QPoint)),
			  this, SLOT(editNote(Geometry*, QPoint)));
	
	mc->setView(QPointF(8.26,50));
	mc->setZoom(13);
	
	ignoreClicks = false;
	addingNote = false;
	noteID = 0;
	
	notetextedit = new QTextEdit(mc);
	notetextedit->setGeometry(0,0,200,100);
	notepoint = new Point(0, 0, notetextedit, ".", Point::TopLeft);
	notepoint->setVisible(false);
	notes->addGeometry(notepoint);

    statusBar = new QStatusBar( this );
    setStatusBar(statusBar);

    loadingProgress = new QLabel("");
    statusBar->addWidget( loadingProgress );
    loadingProgressTimer = new QTimer(this);
    connect(loadingProgressTimer, SIGNAL(timeout()), this, SLOT(updateProgress()), Qt::QueuedConnection );
    loadingProgressTimer->start( 500 ); //update every 500ms

    cacheTiles(true);
}

void Citymap::updateProgress()
{
    QString progressText = QString(" %1 tiles remaining").arg(mc->loadingQueueSize());

    loadingProgress->setText( progressText );
}

void Citymap::cacheTiles(bool qEnabled)
{
    if (qEnabled)
    {
        mc->enablePersistentCache();
    }
    else
    {
        mc->enablePersistentCache(QDir());
    }
}

void Citymap::createTours()
{
	QPen* pen = new QPen(QColor(0,0,255,100));
	pen->setWidth(5);
	
	QList<Point*> points;
	points << new Point(8.2606, 50.0051);
	points << new Point(8.2602, 50.0050);
	points << new Point(8.2598, 50.0044);
	points << new Point(8.2569, 50.0057);
	points << new Point(8.2595, 50.0083);
	points << new Point(8.2587, 50.0086);
	points << new Point(8.2589, 50.0100);
	points << new Point(8.2590, 50.0105);
	pub_tour = new LineString(points, "", pen);
	notes->addGeometry(pub_tour);
	
	points.clear();
	points << new Point(8.25987, 50.0018);
	points << new Point(8.26192, 50.0019);
	points << new Point(8.26301, 50.0031);
	points << new Point(8.26459, 50.0026);
	points << new Point(8.26601, 50.004);
	points << new Point(8.26781, 50.0033);
	points << new Point(8.27052, 50.0054);
	points << new Point(8.2697, 50.0059);
	museum_tour = new LineString(points, "", pen);
	notes->addGeometry(museum_tour);
	
	points.clear();
	points << new Point(8.26015, 50.0015);
	points << new Point(8.2617, 50.0012);
	points << new Point(8.26423, 50.0002);
	points << new Point(8.26698, 50.0024);
	points << new Point(8.27065, 50.0012);
	points << new Point(8.27152, 50.0016);
	points << new Point(8.27225, 50.0004);
	points << new Point(8.27333, 49.9994);
	points << new Point(8.26946, 49.9983);
	points << new Point(8.27105, 49.9973);
	points << new Point(8.27024, 49.9972);
	points << new Point(8.26833, 49.9958);
	sights_tour = new LineString(points, "", pen);
	notes->addGeometry(sights_tour);
}

void Citymap::addSights()
{
	sights = new GeometryLayer("Sehenswürdigkeiten", mapadapter);
	mc->addLayer(sights);
	Point* dom = new ImagePoint(8.274167, 49.998889, QCoreApplication::applicationDirPath() + "/images/180-dom.jpg", "Mainzer Dom");
	dom->setBaselevel(17);
	sights->addGeometry(dom);
	
	Point* stephan = new ImagePoint(8.268611, 49.995556, QCoreApplication::applicationDirPath() + "/images/180-stephan.jpg","St. Stephan");
	stephan->setBaselevel(17);
	sights->addGeometry(stephan);
	
	Point* quitin = new ImagePoint(8.272222, 50.000833, QCoreApplication::applicationDirPath() + "/images/180-quintin.jpg","St. Quintin");
	quitin->setBaselevel(17);
	sights->addGeometry(quitin);	
	connect(sights, SIGNAL(geometryClicked(Geometry*, QPoint)),
			  this, SLOT(geometryClicked(Geometry*, QPoint)));
}
void Citymap::addPubs()
{
	pubs = new GeometryLayer("Kneipe", mapadapter);
	mc->addLayer(pubs);
	
    Point* bagatelle = new Point(8.2606, 50.0052, QPixmap(QCoreApplication::applicationDirPath() + "/images/pub.png"), "Bagatelle");
	pubs->addGeometry(bagatelle);
	
    Point* nirgendwo = new Point(8.2595, 50.0048, QPixmap(QCoreApplication::applicationDirPath() + "/images/pub.png"), "Nirgendwo");
	pubs->addGeometry(nirgendwo);
	
    Point* krokodil = new Point(8.2594,50.0106, QPixmap(QCoreApplication::applicationDirPath() + "/images/pub.png"), "Krokodil");
	pubs->addGeometry(krokodil);
	
	connect(pubs, SIGNAL(geometryClicked(Geometry*, QPoint)),
			  this, SLOT(geometryClickEventKneipe(Geometry*, QPoint)));
}
void Citymap::addMuseums()
{
	museum = new GeometryLayer("Museen", mapadapter);
	mc->addLayer(museum);
	Point* rgzm = new ImagePoint(8.269722, 50.006111, QCoreApplication::applicationDirPath() + "/images/180-rgzm.jpg", "rgzm");
	rgzm->setBaselevel(17);
	museum->addGeometry(rgzm);
	
	Point* lm= new ImagePoint(8.26778, 50.00385, QCoreApplication::applicationDirPath() + "/images/180-lm.jpg", "lm");
	lm ->setBaselevel(17);
	museum->addGeometry(lm);	
	
	connect(museum, SIGNAL(geometryClicked(Geometry*, QPoint)),
			  this, SLOT(geometryClicked(Geometry*, QPoint)));
}

void Citymap::geometryClicked(Geometry* geometry, QPoint)
{
	if (ignoreClicks || addingNote)
		return;
	
	InfoDialog* infodialog = new InfoDialog(this);
	infodialog->setWindowTitle(geometry->name());
	
	if (geometry->name() == "Mainzer Dom")
	{
		infodialog->setInfotext("<h1>Mainzer Dom</h1><p><img src=\"images/180-dom.jpg\" align=\"left\"/>Der Hohe Dom zu Mainz ist die Bischofskirche der Diözese Mainz und steht unter dem Patrozinium des heiligen Martin von Tours. Der Ostchor ist dem Hl. Stephan geweiht. Der zu den Kaiserdomen zählende Bau ist in seiner heutigen Form eine dreischiffige romanische Säulenbasilika, die in ihren Anbauten sowohl gotische als auch barocke Elemente aufweist.</p>");
		
	} else if (geometry->name() == "St. Stephan")
	{
		infodialog->setInfotext("<h1>St. Stephan</h1><p><img src=\"images/180-stephan.jpg\" align=\"left\"/>Die katholische Pfarrkirche Sankt Stephan in Mainz wurde 990 von Erzbischof Willigis auf der höchsten Erhebung der Stadt gegründet. Auftraggeberin war höchstwahrscheinlich die Kaiserwitwe Theophanu. Willigis wollte mit ihr die Gebetsstätte des Reiches schaffen. In der Kirche war ursprünglich ein Stift untergebracht. Der Propst des Stiftes verwaltete eines der Archidiakonate (mittelalterliche Organisationseinheit, ähnlich den heutigen Dekanaten) des Erzbistums.</p>");
	} else if (geometry->name() == "St. Quintin")
	{
		infodialog->setInfotext("<h1>St. Quintin</h1><p><img src=\"images/180-quintin.jpg\" align=\"left\"/>Die Kirche St. Quintin in Mainz ist die Pfarrkirche der ältesten nachgewiesenen Pfarrei der Stadt.");
	} else if (geometry->name() == "rgzm")
	{
		infodialog->setInfotext("<h1>Römisch-Germanisches Zentralmuseum</h1><p><img src=\"images/180-rgzm.jpg\" align=\"left\"/>Das Römisch-Germanische Zentralmuseum (RGZM) in Mainz ist ein Forschungsinstitut für Vor- und Frühgeschichte, das von Bund und Ländern getragen wird und zur Leibniz-Gemeinschaft deutscher Forschungseinrichtungen gehört. Gegliedert in mehrere Abteilungen, arbeitet das Institut im Bereich der Alten Welt sowie seiner Kontaktzonen von der Altsteinzeit bis ins Mittelalter.");
	} else if (geometry->name() == "lm")
	{
		infodialog->setInfotext("<h1>Landesmuseum Mainz</h1><p><img src=\"images/180-lm.jpg\" align=\"left\"/>Das Landesmuseum Mainz ist eines der ältesten Museen in Deutschland. Eine seiner Vorgängerinstitutionen, die Städtische Gemäldesammlung, wurde bereits 1803 von Jean-Antoine Chaptal auf Veranlassung Napoléon Bonapartes durch eine Schenkung von 36 Gemälden gegründet. Das Museum, welches sich heute im ehemaligen kurfürstlichen Marstall befindet, gehört zusammen mit dem Römisch-Germanischen Zentralmuseum und dem Gutenbergmuseum zu den bedeutenden Museen in Mainz. Seine kunst- und kulturgeschichtliche Sammlung erstreckt sich von der Vorgeschichte über die römische Zeit, dem Mittelalter und Barock bis hin zur Jugendstilzeit und der Kunst des 20. Jahrhunderts.");
	}
	if (geometry->name() != "")
		infodialog->showMaximized();
}

void Citymap::geometryClickEventKneipe(Geometry* geometry, QPoint)
{
	if (ignoreClicks || addingNote)
		return;
	InfoDialog* infodialog = new InfoDialog(this);
	infodialog->setWindowTitle(geometry->name());
	infodialog->setInfotext("<h1>" + geometry->name() + "</h1>");
	infodialog->showNormal();
}

void Citymap::addZoomButtons()
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

void Citymap::createActions()
{
	toggleSights = new QAction(tr("Show Sights"), this);
	toggleSights->setCheckable(true);
	toggleSights->setChecked(true);
	connect(toggleSights, SIGNAL(triggered(bool)),
			  sights, SLOT(setVisible(bool)));
	
	togglePub = new QAction(tr("Show Pubs"), this);
	togglePub->setCheckable(true);
	togglePub->setChecked(true);
	connect(togglePub, SIGNAL(triggered(bool)),
			  pubs, SLOT(setVisible(bool)));
	
	toggleMuseum = new QAction(tr("Show Museums"), this);
	toggleMuseum->setCheckable(true);
	toggleMuseum->setChecked(true);
	connect(toggleMuseum, SIGNAL(triggered(bool)),
			  museum, SLOT(setVisible(bool)));
	
	
	toggleSightTour = new QAction(tr("Show Sight Tour"), this);
	toggleSightTour->setCheckable(true);
	toggleSightTour->setChecked(true);
	connect(toggleSightTour, SIGNAL(triggered(bool)),
			  sights_tour, SLOT(setVisible(bool)));
	
	togglePubTour = new QAction(tr("Show Pub Tour"), this);
	togglePubTour->setCheckable(true);
	togglePubTour->setChecked(true);
	connect(togglePubTour, SIGNAL(triggered(bool)),
			  pub_tour, SLOT(setVisible(bool)));
	
	toggleMuseumTour = new QAction(tr("Show Museum Tour"), this);
	toggleMuseumTour->setCheckable(true);
	toggleMuseumTour->setChecked(true);
	connect(toggleMuseumTour, SIGNAL(triggered(bool)),
			  museum_tour, SLOT(setVisible(bool)));
	
	addNoteAction = new QAction(tr("Add Note"), this);
	connect(addNoteAction, SIGNAL(triggered(bool)),
			  this, SLOT(addNote()));
	
	toolsDistance = new QAction(tr("Calculate Distance"), this);
	connect(toolsDistance, SIGNAL(triggered(bool)),
              this, SLOT(calcDistance()));

    toolsLocalDiskCache = new QAction(tr("Cache Tiles Locally"), this);
    toolsLocalDiskCache->setCheckable(true);
    toolsLocalDiskCache->setChecked(true);
    connect(toolsLocalDiskCache, SIGNAL(triggered(bool)),
              this, SLOT(cacheTiles(bool)));
	
	QActionGroup* mapproviderGroup = new QActionGroup(this);
    osmAction = new QAction(tr("OpenStreetMap"), mapproviderGroup);
    googleActionMap = new QAction(tr("Google: Roadmap (default)"), mapproviderGroup);
    googleActionSatellite = new QAction(tr("Google: Satellite"), mapproviderGroup);

    googleActionSatellite = new QAction(tr("Google: Satellite"), mapproviderGroup);
    googleActionTerrain = new QAction(tr("Google: Terrain"), mapproviderGroup);
    googleActionHybrid = new QAction(tr("Google: Hybrid"), mapproviderGroup);

    osmAction->setCheckable(true);
	googleActionMap->setCheckable(true);
    googleActionSatellite->setCheckable(true);
    googleActionTerrain->setCheckable(true);
    googleActionHybrid->setCheckable(true);
    osmAction->setChecked(true);
	connect(mapproviderGroup, SIGNAL(triggered(QAction*)),
			  this, SLOT(mapproviderSelected(QAction*)));
		
    QActionGroup* mapZoomGroup = new QActionGroup(this);

    for( int i=0; i <= 17; ++i )
    {
        QString title = QString("Zoom %1").arg(i);
        QAction* action = new QAction(title, mapZoomGroup);
        action->setCheckable(true);
        zoomActions << action;
    }
    connect(mapZoomGroup, SIGNAL(triggered(QAction*)),
              this, SLOT(mapZoomSelected(QAction*)));
}

void Citymap::createMenus()
{
	layerMenu = menuBar()->addMenu(tr("&Layer"));
	layerMenu->addAction(toggleSights);
	layerMenu->addAction(togglePub);
	layerMenu->addAction(toggleMuseum);

	tourMenu = menuBar()->addMenu(tr("T&ours"));
	tourMenu->addAction(toggleSightTour);
	tourMenu->addAction(togglePubTour);
	tourMenu->addAction(toggleMuseumTour);
	
	toolsMenu = menuBar()->addMenu(tr("&Tools"));
	toolsMenu->addAction(addNoteAction);
	toolsMenu->addAction(toolsDistance);
    toolsMenu->addAction(toolsLocalDiskCache);
	
	mapMenu = menuBar()->addMenu(tr("&Map Provider"));
	mapMenu->addAction(osmAction);
	mapMenu->addAction(googleActionMap);
    mapMenu->addAction(googleActionSatellite);
    mapMenu->addAction(googleActionTerrain);
    mapMenu->addAction(googleActionHybrid);
	
    zoomMenu = menuBar()->addMenu(tr("&Zoom Level"));
    foreach( QAction* action, zoomActions )
    {
        zoomMenu->addAction(action);
    }

}

void Citymap::addNote()
{
	addingNote = true;
	connect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
			  this, SLOT(writeNote(const QMouseEvent*, const QPointF)));
}

void Citymap::writeNote(const QMouseEvent*, const QPointF coord)
{
    Point* p = new Point(coord.x(), coord.y(), *notepixmap, QString::number(++noteID), Point::BottomLeft);
	currentnoteID = noteID;
	p->setBaselevel(16);
	p->setMinsize(QSize(12, 10));
	p->setMaxsize(QSize(47, 40));
	notes->addGeometry(p);
	
	notetextedit->clear();
	
	notepoint->setCoordinate(coord);
	notepoint->setVisible(true);
	
	mc->updateRequestNew();
	
	disconnect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
			  this, SLOT(writeNote(const QMouseEvent*, const QPointF)));
	
	connect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
			  this, SLOT(hideNote(const QMouseEvent*, const QPointF)));
}

void Citymap::hideNote(const QMouseEvent* evnt, const QPointF)
{
	if (addingNote && evnt->type() == QEvent::MouseButtonDblClick)
	{
		addingNote = false;
		notepoint->setVisible(false);

		mc->updateRequestNew();
		
		// save text
		notestext[currentnoteID] = notetextedit->toPlainText();
		
		disconnect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
				  this, SLOT(hideNote(const QMouseEvent*, const QPointF)));
	}
}

void Citymap::editNote(Geometry* geom, QPoint)
{
	addingNote = true;
	currentnoteID = QVariant(geom->name()).toInt();
	notetextedit->setPlainText(notestext[currentnoteID]);
	notepoint->setCoordinate(geom->points().at(0)->coordinate());
	notepoint->setVisible(true);
	
	mc->updateRequestNew();
	connect(mc, SIGNAL(mouseEventCoordinate(const QMouseEvent*, const QPointF)),
            this, SLOT(hideNote(const QMouseEvent*, const QPointF)));
}

void Citymap::resizeEvent(QResizeEvent *qEvent)
{
    Q_UNUSED( qEvent );
    if (mc)
    {
        mc->resize(size());
    }
}

void Citymap::calcDistance()
{
	ignoreClicks = true;
	connect(mc, SIGNAL(mouseEventCoordinate( const QMouseEvent*, const QPointF )),
			  this, SLOT(calcDistanceClick(const QMouseEvent*, const QPointF)));
}
void Citymap::calcDistanceClick(const QMouseEvent* evnt, const QPointF coord)
{
	if (coord1 == QPointF() && evnt->type() == QEvent::MouseButtonPress)
	{
		coord1 = coord;
		l->addGeometry(new ImagePoint(coord1.x(), coord1.y(), QCoreApplication::applicationDirPath() + "/images/flag.png", "", Point::BottomRight));
		mc->updateRequestNew();
	}
	else if (coord2 == QPointF() && evnt->type() == QEvent::MouseButtonPress)
	{
		coord2 = coord;
		double PI = acos(-1.0);
		double a1 = coord1.x()* (PI/180.0);;
		double b1 = coord1.y()* (PI/180.0);;
		double a2 = coord2.x()* (PI/180.0);;
		double b2 = coord2.y()* (PI/180.0);;
		double r = 6378;
		
		double km = acos(cos(a1)*cos(b1)*cos(a2)*cos(b2) + cos(a1)*sin(b1)*cos(a2)*sin(b2) + sin(a1)*sin(a2)) * r;
		
		QList<Point*> points;
		points.append(new Point(coord1.x(), coord1.y()));
		QPixmap* pixm = new QPixmap(100,20);
		pixm->fill(Qt::transparent);
		QPainter pain(pixm);
		pain.setFont(QFont("Helvetiva", 6));
		pain.drawText(pixm->rect(), QString().setNum(km, 'f', 3) + " km");
		pain.end();
        points.append(new Point(coord2.x(), coord2.y(), *pixm, "", Point::BottomLeft));
		l->addGeometry(new LineString(points));
		mc->updateRequestNew();
		coord1 = QPointF();
		coord2 = QPointF();
		ignoreClicks = false;
		disconnect(mc, SIGNAL(mouseEventCoordinate( const QMouseEvent*, const QPointF)),
					  this, SLOT(calcDistanceClick(const QMouseEvent*, const QPointF)));
		
    }
}

void Citymap::mapControlZoomChanged(const QPointF &coordinate, int zoom) const
{
    Q_UNUSED(coordinate);
    if ( zoomActions.at(zoom) )
    {
        zoomActions.at(zoom)->setChecked( true );
    }
}

void Citymap::mapZoomSelected(QAction* action)
{
    mc->setZoom( zoomActions.indexOf(action) );
}

void Citymap::mapproviderSelected(QAction* action)
{
	if (action == osmAction)
	{
		int zoom = mapadapter->adaptedZoom();
		mc->setZoom(0);
		
		mapadapter = new OSMMapAdapter();
		l->setMapAdapter(mapadapter);
		sights->setMapAdapter(mapadapter);
		museum->setMapAdapter(mapadapter);
		pubs->setMapAdapter(mapadapter);
		notes->setMapAdapter(mapadapter);
		mc->updateRequestNew();
		mc->setZoom(zoom);
    }
    else if (action == googleActionMap)
	{
        int zoom = mapadapter->adaptedZoom();
        mc->setZoom(0);
		mapadapter = new GoogleMapAdapter();
		l->setMapAdapter(mapadapter);
		sights->setMapAdapter(mapadapter);
		museum->setMapAdapter(mapadapter);
        pubs->setMapAdapter(mapadapter);
		notes->setMapAdapter(mapadapter);
		mc->updateRequestNew();
		mc->setZoom(zoom);
	}
    else if (action == googleActionSatellite)
    {
        int zoom = mapadapter->adaptedZoom();
        mc->setZoom(0);
        mapadapter = new GoogleMapAdapter(GoogleMapAdapter::satellite);
        l->setMapAdapter(mapadapter);
        sights->setMapAdapter(mapadapter);
        museum->setMapAdapter(mapadapter);
        pubs->setMapAdapter(mapadapter);
        notes->setMapAdapter(mapadapter);
        mc->updateRequestNew();
        mc->setZoom(zoom);
    }
    else if (action == googleActionTerrain)
    {
        int zoom = mapadapter->adaptedZoom();
        mc->setZoom(0);
        mapadapter = new GoogleMapAdapter(GoogleMapAdapter::terrain);
        l->setMapAdapter(mapadapter);
        sights->setMapAdapter(mapadapter);
        museum->setMapAdapter(mapadapter);
        pubs->setMapAdapter(mapadapter);
        notes->setMapAdapter(mapadapter);
        mc->updateRequestNew();
        mc->setZoom(zoom);
    }
    else if (action == googleActionHybrid)
    {
        int zoom = mapadapter->adaptedZoom();
        mc->setZoom(0);
        mapadapter = new GoogleMapAdapter(GoogleMapAdapter::hybrid);
        l->setMapAdapter(mapadapter);
        sights->setMapAdapter(mapadapter);
        museum->setMapAdapter(mapadapter);
        pubs->setMapAdapter(mapadapter);
        notes->setMapAdapter(mapadapter);
        mc->updateRequestNew();
        mc->setZoom(zoom);
    }
}

Citymap::~Citymap()
{
	delete mc;
	delete mapadapter;
	delete notepixmap;
	delete sights;
	delete notes;
	delete pubs;
	delete museum;
}

