#ifndef GPS_H
#define GPS_H

#include <QPushButton>
#include <QLabel>
#include <mapcontrol.h>
#include "gps_neo.h"
#include "gps_simulation.h"
#include "circlepoint.h"

using namespace qmapcontrol;
class GPS: public QWidget
{
    Q_OBJECT
    public:
            GPS();
            ~GPS();

    private:
            QPushButton* followgps;
            QPushButton* simulategps;
            QLabel* gpsposition;
            MapControl* mc;
            GPS_Simulation* gpsSim;
            CirclePoint* gpsDot;

    public slots:
            void updatePosition(float time, QPointF coordinate);
            void resizeEvent(QResizeEvent *qEvent);
            void simulategps_checked();
};

#endif
