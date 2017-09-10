#ifndef COLORSENSORACCESS_H
#define COLORSENSORACCESS_H

#include <QObject>
#include <QMutex>
#include <QColor>
#include <QDebug>
#include <QThread>
#include <QtEndian>

#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

class ColorSensorAccess : public QObject
{
    Q_OBJECT

public:
    enum SensorParameter {
        SensorAddress = 0x2A,
    };

    enum IntegrationTime {
        T00 = 0,
        T01,
        T10,
        T11,
        Manual,
    };

    enum Gain {
        Low = 0,
        High,
    };

    struct ColorData {
        uint16_t blue;
        uint16_t green;
        uint16_t red;
        uint16_t infraRed;

    public:
        QColor getColor() {
            return QColor( (double)red / UINT16_MAX * UINT8_MAX, (double)green / UINT16_MAX * UINT8_MAX, (double)blue / UINT16_MAX * UINT8_MAX );
        }
    };

public:
    explicit ColorSensorAccess(QObject *parent = 0);

    bool openSensor(QString filePath = "/dev/i2c-0");
    bool initializeSensor( IntegrationTime intTime, Gain gain );
    void closeSensor();

    void readColors( bool waitForIntegration );

    void waitIntegrationTime();

signals:
    void dataRead( ColorSensorAccess::ColorData data );

public slots:
    void startReading( bool continuously = false );
    void stopReading();

private:
    QMutex mutex;

    uint8_t sensorAddress;
    QString sensorPath;
    Gain gain;
    IntegrationTime intTime;
    ColorData colorData;

    bool doReading;

    int file;
};

Q_DECLARE_METATYPE(ColorSensorAccess::ColorData)

#endif // COLORSENSORACCESS_H
