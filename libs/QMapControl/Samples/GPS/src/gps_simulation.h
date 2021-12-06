#ifndef GPS_SIMULATION_H
#define GPS_SIMULATION_H

#include <QObject>
#include <QTimer>
#include <QPointF>

class GPS_Simulation : public QObject
{
    Q_OBJECT
public:
    explicit GPS_Simulation(QObject *parent = 0);
    ~GPS_Simulation();

    void start();
    void stop();

signals:
    void newPosition(float time, QPointF coordinate);

public slots:
    void tick();

private:
    QTimer *timer;
    float mLat;
    float mLong;
};

#endif // GPS_SIMULATION_H
