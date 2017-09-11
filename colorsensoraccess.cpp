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

bool ColorSensorAccess::initializeSensor(ColorSensorAccess::IntegrationTime intTime, bool manualIntegrationMode, uint16_t manualTime, ColorSensorAccess::Gain gain)
{
    uint8_t bytes[4];
    uint8_t intTimeByte = 0;

    if ( file < 0 ) {
        return false;
    }

    // Copy args
    this->intTime = intTime;
    this->gain = gain;
    this->manualIntegrationMode = manualIntegrationMode;
    this->manualTime = manualTime;

    // Register address
    bytes[0] = 0x00;
    bytes[2] = 0x00;

    if ( manualIntegrationMode ) {
        // This mode is not yet implemented
        intTimeByte = ( gain << 3 ) | intTime | Manual;
    } else {
        intTimeByte = ( gain << 3 ) | intTime;
    }

    controlByte = intTimeByte;

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

    // Write manual timing if mode is set to manual integration
    // host processor is assumed as little endian in this block
    if ( manualIntegrationMode ) {
        bytes[0] = 0x01;
        qToBigEndian<uint16_t>( manualTime, bytes + 1 );

        i2cMsg[0].addr  = sensorAddress;
        i2cMsg[0].buf   = bytes;
        i2cMsg[0].flags = 0;
        i2cMsg[0].len   = 3;

        i2cData.msgs  = i2cMsg;
        i2cData.nmsgs = 1;

        ret = ioctl( file, I2C_RDWR, &i2cData );
        qDebug() << ret;
    }

    // reset ADC, disable sleeping
    bytes[0] = 0x00;
    bytes[1] = 0x80 | intTimeByte;

    // start ADC
    bytes[2] = 0x00;
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
    qDebug() << ret;

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
    uint8_t reg;
    int ret;

    // Disable sleep mode if integration mode is manual
    if ( manualIntegrationMode ) {
        bytes[0] = 0x00;
        bytes[1] = controlByte & 0x0F;

        i2cMsg[0].addr  = sensorAddress;
        i2cMsg[0].buf   = bytes;
        i2cMsg[0].flags = 0;
        i2cMsg[0].len   = 2;

        i2cData.msgs  = i2cMsg;
        i2cData.nmsgs = 1;

        ret = ioctl( file, I2C_RDWR, &i2cData );
        qDebug() << ret;
    }

    // Wait until doing integration
    if ( waitForIntegration ) {
        waitIntegrationTime();
    }

    // Read data
    reg = 0x03;

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
    qDebug() << ret;

    // Store data into structure
    // host processor is assumed as little endian in this block
    mutex.lock();
    colorData.red      = qFromBigEndian<uint16_t>( (uint16_t *)( bytes + 0 ) );
    colorData.green    = qFromBigEndian<uint16_t>( (uint16_t *)( bytes + 2 ) );
    colorData.blue     = qFromBigEndian<uint16_t>( (uint16_t *)( bytes + 4 ) );
    colorData.infraRed = qFromBigEndian<uint16_t>( (uint16_t *)( bytes + 6 ) );
    mutex.unlock();

    emit dataRead( colorData );
}

void ColorSensorAccess::waitIntegrationTime()
{
    // Wait for integration time
    unsigned int ms;

    if ( !manualIntegrationMode ) {
        // Static time integration mode
        switch ( intTime ) {
        case T00:
            ms = 1;
            break;
        case T01:
            ms = 5;
            break;
        case T10:
            ms = 30;
            break;
        case T11:
            ms = 200;
            break;
        default:
            // NOT ENTER HERE
            ms = 500;
            break;
        }
    } else {
        // Manual integration mode
        switch ( intTime ) {
        case T00:
            ms = 1 * manualTime;
            break;
        case T01:
            ms = 5 * manualTime;
            break;
        case T10:
            ms = 60 * manualTime;
            break;
        case T11:
            ms = 400 * manualTime;
            break;
        default:
            // NOT ENTER HERE
            ms = 500 * manualTime;
            break;
        }
    }

    QThread::msleep( ms );
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
