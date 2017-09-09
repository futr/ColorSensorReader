#ifndef COLORSENSORACCESS_H
#define COLORSENSORACCESS_H

#include <QObject>

#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

class ColorSensorAccess : public QObject
{
    Q_OBJECT

    enum SensorParameter {
        SensorAddress = 0x12,
    };

    enum Sensitivity {
        Low,
        High,
        Manual,
    };

    struct ColorData {
        uint16_t blue;
        uint16_t green;
        uint16_t red;
        uint16_t infraRed;
    };

public:
    explicit ColorSensorAccess(QObject *parent = 0);

    void openSensor();
    void initializeSensor();
    void closeSensor();

    void readColors();
    void readColorsContinuously();

signals:
    void dataRead( ColorData data );

public slots:
    void startReading( bool continuously = false );
    void stopReading();

private:
    uint8_t sensorAddress;
    Sensitivity sensitivity;
    ColorData colorData;

    bool doReading;
};

#endif // COLORSENSORACCESS_H
