#include "colorsensoraccess.h"

ColorSensorAccess::ColorSensorAccess(QObject *parent) : QObject(parent),
    sensorAddress( SensorAddress ),
    sensorPath( "/dev/i2c-1" ),
    file( -1 )
{

}

bool ColorSensorAccess::openSensor( QString filePath )
{
    // Open color sensor device file
    sensorPath = filePath;

    file = open( filePath.toLocal8Bit().constData(), O_RDWR );

    if ( file < 0 ) {
        return false;
    }

    if ( ioctl( file, I2C_SLAVE, sensorAddress ) < 0 ) {
        closeSensor();

        return false;
    }

    return true;
}

bool ColorSensorAccess::initializeSensor(ColorSensorAccess::IntegrationTime intTime, ColorSensorAccess::Gain gain)
{
    uint8_t bytes[4];
    uint8_t intTimeByte = 0;

    if ( file < 0 ) {
        return false;
    }

    // Copy args
    this->intTime = intTime;
    this->gain = gain;

    // Register address
    bytes[0] = 0x00;
    bytes[2] = 0x00;

    if ( intTime == Manual ) {
        // This mode is not yet implemented
        intTimeByte = ( gain << 3 ) | 0;
    } else {
        intTimeByte = ( gain << 3 ) | intTime;
    }

    // Using simple R/W APIs
    /*
    // reset ADC, disable sleeping
    bytes[1] = 0x80 | intTimeByte;
    write( file, bytes, 2 );

    // start ADC
    bytes[1] = 0x00 | intTimeByte;
    write( file, bytes, 2 );
    */

    // Using IOCTL
    i2c_rdwr_ioctl_data i2cData;
    i2c_msg i2cMsg[2];
    int ret;

    // reset ADC, disable sleeping
    bytes[1] = 0x80 | intTimeByte;

    // start ADC
    bytes[3] = 0x00 | intTimeByte;

    i2cMsg[0].addr  = sensorAddress;
    i2cMsg[0].buf   = bytes;
    i2cMsg[0].flags = 0;
    i2cMsg[0].len   = 2;

    i2cMsg[1].addr  = sensorAddress;
    i2cMsg[1].buf   = bytes + 2;
    i2cMsg[1].flags = 0;
    i2cMsg[1].len   = 2;

    i2cData.msgs  = i2cMsg;
    i2cData.nmsgs = 2;

    ret = ioctl( file, I2C_RDWR, &i2cData );

    //qDebug() << ret;

    return true;
}

void ColorSensorAccess::closeSensor()
{
    if ( file < 0 ) {
        return;
    }

    close( file );

    file = -1;
}

void ColorSensorAccess::readColors(bool waitForIntegration)
{
    uint8_t bytes[8];

    if ( file < 0 ) {
        return ;
    }

    // Wait until doing integration
    if ( waitForIntegration ) {
        waitIntegrationTime();
    }

    // Using simple R/W IO APIs
    /*
    // Write register address to read
    bytes[0] = 0x03;
    write( file, bytes, 1 );

    // Read color data
    read( file, bytes, 8 );
    */

    // Using IOCTL
    i2c_rdwr_ioctl_data i2cData;
    i2c_msg i2cMsg[2];
    uint8_t reg = 0x03;
    int ret;

    i2cMsg[0].addr  = sensorAddress;
    i2cMsg[0].buf   = &reg;
    i2cMsg[0].flags = 0;
    i2cMsg[0].len   = 1;

    i2cMsg[1].addr  = sensorAddress;
    i2cMsg[1].buf   = bytes;
    i2cMsg[1].flags = I2C_M_RD;
    i2cMsg[1].len   = 8;

    i2cData.msgs  = i2cMsg;
    i2cData.nmsgs = 2;

    ret = ioctl( file, I2C_RDWR, &i2cData );

    //qDebug() << ret;

    // Store into structure
    mutex.lock();
    colorData.red      = qFromBigEndian<uint16_t>( (uint16_t *)( bytes + 0 ) );
    colorData.green    = qFromBigEndian<uint16_t>( (uint16_t *)( bytes + 2 ) );
    colorData.blue     = qFromBigEndian<uint16_t>( (uint16_t *)( bytes + 4 ) );
    colorData.infraRed = qFromBigEndian<uint16_t>( (uint16_t *)( bytes + 6 ) );
    /*
    colorData.red      = *(uint16_t *)( bytes + 0 );
    colorData.green    = *(uint16_t *)( bytes + 2 );
    colorData.blue     = *(uint16_t *)( bytes + 4 );
    colorData.infraRed = *(uint16_t *)( bytes + 6 );
    */
    mutex.unlock();

    emit dataRead( colorData );
}

void ColorSensorAccess::waitIntegrationTime()
{
    // Wait for integration time
    switch ( intTime ) {
    case T00:
        QThread::msleep( 1 );
        break;
    case T01:
        QThread::msleep( 5 );
        break;
    case T10:
        QThread::msleep( 30 );
        break;
    case T11:
        QThread::msleep( 200 );
        break;
    default:
        // Need to calculate integral time
        QThread::msleep( 500 );
        break;
    }
}

void ColorSensorAccess::startReading(bool continuously)
{
    doReading = continuously;

    do {
        readColors( true );
    } while( doReading );
}

void ColorSensorAccess::stopReading()
{
    doReading = false;
}
