#include "colorsensoraccess.h"

ColorSensorAccess::ColorSensorAccess(QObject *parent) : QObject(parent),
    sensorAddress( SensorAddress )
{

}

void ColorSensorAccess::startReading(bool continuously)
{
    doReading = continuously;
}

void ColorSensorAccess::stopReading()
{
    doReading = false;
}
