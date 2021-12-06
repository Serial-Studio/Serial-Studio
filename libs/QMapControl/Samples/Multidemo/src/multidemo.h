#ifndef MULTIDEMO_H
#define MULTIDEMO_H

#include <QPushButton>
#include <mapcontrol.h>
#include <imagepoint.h>
#include <maplayer.h>
#include <geometrylayer.h>
#include <osmmapadapter.h>
#include <circlepoint.h>
#include <linestring.h>
#include "gps_modul.h"
using namespace qmapcontrol;
class Multidemo : public QWidget
{
        Q_OBJECT
        public:
                Multidemo(QWidget *parent = 0);

                ~Multidemo();

        private:
                MapControl* mc;
                MapControl* mc2;
                QPushButton* btn1;
                QPushButton* btn2;
                QPushButton* btn3;
                QPushButton* btn4;
                QPushButton* btn5;
                ImagePoint* ip;
                GPS_Modul* gm;

                void setupMaps();
                void createLayout();
                Layer* l;

        public slots:
                void geometryClickEvent(Geometry* geom, QPoint coord_px);
                void coordinateClicked(const QMouseEvent*, const QPointF);
                void coordinateClicked_mc2(const QMouseEvent*, const QPointF);
                void buttonToggled(bool);
                void toggleFollow(bool);
                void toggleGPS(bool);

                void draggedRect(QRectF);
                void mouseEventCoordinate(const QMouseEvent*, const QPointF);

        protected:
                void keyPressEvent(QKeyEvent* evnt);
                virtual void resizeEvent ( QResizeEvent * event );

        signals:
                void setX(int);
                void setY(int);
                void zoomIn();
                void zoomOut();

};

#endif
