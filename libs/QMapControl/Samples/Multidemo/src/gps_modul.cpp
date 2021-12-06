#include "gps_modul.h"

double x = 1.0;
double y = 1.0;
GPS_Modul::GPS_Modul(QObject *parent)
 : QObject(parent)
{
// 	qDebug() << "GPS_Modul()";
	loadFile();
	
	running = false;	
}


GPS_Modul::~GPS_Modul()
{
}

void GPS_Modul::start()
{
	
	if (!running)
	{
		running = true;
		QTimer::singleShot(1000/25, this, SLOT(tick()));
	}
}
void GPS_Modul::stop()
{
	running = false;
}

void GPS_Modul::tick()
{
// 	qDebug() << "GPS_Modul::tick()";
	
// 	GPS_Position pos = positions.takeFirst();
// 	x = pos.longitude;
// 	y = pos.latitude;
	
// 	qDebug() << pos.latitude  << ", " << pos.longitude;
	x += .1;
	y += .1;
	emit(new_position(QPointF(x,y)));
// 	emit(changed());
	
// 	if (running && !positions.isEmpty())
	if (running)
		QTimer::singleShot(1000/25, this, SLOT(tick()));
}

void GPS_Modul::loadFile()
{
	QFile file("/home/kai/kwint001/trunk/code/MapAPI/src/mainz_gps.nme");
// 	qDebug() << file.exists();
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << file.error();
		return;
	}
	while (!file.atEnd())
	{
		QByteArray line = file.readLine();
		process_line(line);
	}
}

void GPS_Modul::process_line(QByteArray line)
{
	if (!line.contains("GPRMC"))
		return;
		 
	line.chop(1);
// 	qDebug() << line;
		
	// get time
	QList<QByteArray> elems = line.split(',');

	float time = QString(elems.at(1)).toFloat();
	float latitude = elems.at(3).toFloat()/100;
	QString latitude_dir = elems.at(4);
	float longitude = elems.at(5).toFloat()/100;
	QString longitude_dir = elems.at(6);
	
	positions.append(GPS_Position(time, longitude, longitude_dir, latitude, latitude_dir));
	
	
// 	qDebug() << elems.at(6) << " | " << latitude;
}
