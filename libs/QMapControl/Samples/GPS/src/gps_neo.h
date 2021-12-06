#ifndef GPS_NEO_H
#define GPS_NEO_H

#include <QObject>
#include <QtGui>
#include <gps_position.h>
//! A parser for the NMEA data format
/*!
 * This class parses gps data from the Neo´s gllin service, which you have to start manually
 * It reads the device file every seconds and emits a signal which contains a GPS_Position.
 * @see http://3rdparty.downloads.openmoko.org
 *	@author Kai Winter <kaiwinter@gmx.de>
*/
using namespace qmapcontrol;
class GPS_Neo: public QObject
{
        Q_OBJECT
        public:
                GPS_Neo(QObject *parent = 0);
                ~GPS_Neo();
                void start();
                void stop();

        private:
                QList<GPS_Position> positions;
                GPS_Position process_line(QByteArray line);
                bool running;

        signals:
                void new_position(float, QPointF);

        public slots:
                void tick();
};

#endif
