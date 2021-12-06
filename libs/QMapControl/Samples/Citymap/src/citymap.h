#ifndef CITYMAP_H
#define CITYMAP_H

#include <mapcontrol.h>
#include <osmmapadapter.h>
#include <googlemapadapter.h>
#include <maplayer.h>
#include <geometrylayer.h>
#include <linestring.h>
#include <imagepoint.h>
#include "dialogs.h"
#include <QMainWindow>

class QLabel;

using namespace qmapcontrol;
class Citymap: public QMainWindow
{
        Q_OBJECT
        public:
                Citymap(QWidget* parent = 0);

                ~Citymap();

        private:
                MapControl* mc;
                MapAdapter* mapadapter;

                QPixmap* notepixmap;

                Layer* sights;
                Layer* museum;
                Layer* pubs;
                Layer* notes;

                void addZoomButtons();

                void createTours();
                void createActions();
                void createMenus();

                QMenu* layerMenu;
                QMenu* tourMenu;
                QMenu* toolsMenu;
                QMenu* mapMenu;
                QMenu* zoomMenu;

                QAction* toggleSights;
                QAction* togglePub;
                QAction* toggleMuseum;

                QAction* togglePubTour;
                QAction* toggleMuseumTour;
                QAction* toggleSightTour;

                QAction* addNoteAction;
                QAction* toolsDistance;
                QAction* toolsLocalDiskCache;

                QAction* osmAction;
                QAction* googleActionMap;
                QAction* googleActionSatellite;
                QAction* googleActionTerrain;
                QAction* googleActionHybrid;

                QList<QAction*> zoomActions;

                QStatusBar* statusBar;

                bool ignoreClicks;
                bool addingNote;

                void addSights();
                void addPubs();
                void addMuseums();

                QPointF coord1;
                QPointF coord2;

                Layer* l;

                LineString* pub_tour;
                LineString* museum_tour;
                LineString* sights_tour;

                QTextEdit* notetextedit;
                Point* notepoint;
                int noteID;
                int currentnoteID;
                QHash<int, QString> notestext;
                QLabel* loadingProgress;
                QTimer* loadingProgressTimer;

        public slots:
                void hideNote(const QMouseEvent* evnt, const QPointF coordinate);
                void geometryClicked(Geometry* geometry, QPoint point);
                void geometryClickEventKneipe(Geometry* geometry, QPoint point);
                void addNote();
                void writeNote(const QMouseEvent*, const QPointF);
                void calcDistance();
                void calcDistanceClick(const QMouseEvent*, const QPointF);
                void mapControlZoomChanged(const QPointF &coordinate, int zoom ) const;

                void mapproviderSelected(QAction*);
                void mapZoomSelected(QAction*);
                void editNote(Geometry* geom, QPoint point);
                void resizeEvent(QResizeEvent *qEvent);
                void updateProgress();
                void cacheTiles(bool qEnabled);
};

#endif
