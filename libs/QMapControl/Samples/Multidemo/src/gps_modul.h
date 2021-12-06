#ifndef GPS_MODUL_H
#define GPS_MODUL_H

#include <QObject>
#include <QtGui>
#include <gps_position.h>

/**
        @author Kai Winter <kaiwinter@gmx.de>
*/
using namespace qmapcontrol;
class GPS_Modul : public QObject
{
        Q_OBJECT
        public:
                GPS_Modul(QObject *parent = 0);
                ~GPS_Modul();
                void start();
                void stop();

        private:
                QList<GPS_Position> positions;
                void loadFile();
                void process_line(QByteArray line);
                bool running;

        signals:
                void new_position(QPointF);
                void changed();

        public slots:
                void tick();

};

#endif
