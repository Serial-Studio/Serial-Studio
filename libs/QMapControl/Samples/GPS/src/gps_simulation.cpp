#include "gps_simulation.h"
#include <QDateTime>
#include <QtGlobal>
#include <QPointF>

GPS_Simulation::GPS_Simulation(QObject *parent)
    : QObject(parent)
    , timer(new QTimer(this))
    , mLat(40.748817f)
    , // new york lat
    mLong(-73.985428f) // new york lat
{
    bool connected
        = connect(timer, SIGNAL(timeout()), this, SLOT(tick()), Qt::QueuedConnection);
    Q_ASSERT(connected);
}

GPS_Simulation::~GPS_Simulation() { }

void GPS_Simulation::start()
{
    timer->start(1000); // 1 sec updates
}

void GPS_Simulation::stop()
{
    timer->stop();
}

void GPS_Simulation::tick()
{
    static float faketime = qrand() % 5000;

    float lTempLat = qrand() % 2 - 1; // gives a number between -1 and 1
    float lTempLong = qrand() % 2 - 1; // gives a number between -1 and 1

    mLat = qBound(float(-90), (mLat + (lTempLat / 10)), float(90));
    mLong = qBound(float(-180), (mLat + (lTempLong / 10)), float(180));

    emit newPosition(++faketime, QPointF(mLong, mLat));
}
