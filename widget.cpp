#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // Construct and move to worker thread a sensor accessor class
    colorSensor = new ColorSensorAccess;
    colorSensor->moveToThread( &sensorThread );
    sensorThread.start();

    // Connect signals
    qRegisterMetaType<ColorSensorAccess::ColorData>();

    connect( this, SIGNAL(doReading(bool)), colorSensor, SLOT(startReading(bool)) );
    connect( this, SIGNAL(stopReading()), colorSensor, SLOT(stopReading()), Qt::DirectConnection );
    connect( colorSensor, SIGNAL(dataRead(ColorSensorAccess::ColorData)), this, SLOT(setData(ColorSensorAccess::ColorData)) );

    // Setup button groups
    intTimeGroup.addButton( ui->intTime0Button, ColorSensorAccess::T00 );
    intTimeGroup.addButton( ui->intTime1Button, ColorSensorAccess::T01 );
    intTimeGroup.addButton( ui->intTime2Button, ColorSensorAccess::T10 );
    intTimeGroup.addButton( ui->intTime3Button, ColorSensorAccess::T11 );
    intTimeGroup.addButton( ui->intTimeManButton, ColorSensorAccess::Manual );

    gainGroup.addButton( ui->lowButton, ColorSensorAccess::Low );
    gainGroup.addButton( ui->highButton, ColorSensorAccess::High );

}

Widget::~Widget()
{
    // Stop a sensor thread
    colorSensor->stopReading();
    colorSensor->closeSensor();
    sensorThread.quit();
    sensorThread.wait( 3000 );

    delete ui;
}

void Widget::setData(ColorSensorAccess::ColorData data)
{
    // Fill label
    setColorLabel(data);

    // Add comma separated value into edit
    QString str = QString( "%1,%2,%3,%4" ).arg( data.blue ).arg( data.green ).arg( data.red ).arg( data.infraRed );

    ui->logEdit->appendPlainText( str );
    ui->logEdit->ensureCursorVisible();
}

void Widget::setColorLabel(ColorSensorAccess::ColorData data)
{
    QPixmap map( ui->colorPreviewLabel->size() );
    QPainter painter( &map );
    QColor color = data.getColor();

    // Set V maximum
    //color = QColor::fromHsv( color.hsvHue(), color.hsvSaturation(), 255 );

    // Calculate display equivalent color
    double max = 0;
    double b, g, r, ir;

    b  = data.blue  * 1.82;
    g  = data.green * 1.0;
    r  = data.red   * 1.29;
    ir = data.infraRed;

    if ( b > max ) {
        max = b;
    }
    if ( g > max ) {
        max = g;
    }
    if ( r > max ) {
        max = r;
    }
    /*
    if ( data.infraRed > max ) {
        max = data.infraRed;
    }
    */

    // Normalize
    b = b / max;
    g = g / max;
    r = r / max;

    color = QColor::fromRgbF( r, g, b );

    // Fill by latest color
    painter.fillRect( map.rect(), color );

    // Calculate text size
    QString str = QString( "B:%1 G:%2 R:%3 IR:%4 " ).arg( data.blue ).arg( data.green ).arg( data.red ).arg( data.infraRed );

    painter.drawText( map.rect(), Qt::AlignCenter, str );

    ui->colorPreviewLabel->setPixmap( map );
}

void Widget::on_openButton_clicked()
{
    if ( !colorSensor->openSensor( ui->sensorPathEdit->text() ) )
    {
        QMessageBox::critical( this, "Error", "Failed to open color sensor" );
        return;
    }

    ui->openButton->setEnabled( false );
    ui->initializeButton->setEnabled( true );
    ui->closeSensorButton->setEnabled( true );
}

void Widget::on_initializeButton_clicked()
{
    if( !colorSensor->initializeSensor( (ColorSensorAccess::IntegrationTime)intTimeGroup.checkedId(), (ColorSensorAccess::Gain)gainGroup.checkedId() ) )
    {
        QMessageBox::critical( this, "Error", "Failed to initialize color sensor" );
    }
}

void Widget::on_pushButton_7_clicked()
{
    ui->logEdit->clear();
}

void Widget::on_pushButton_8_clicked()
{
    // Save log file
    QFileDialog saveDialog( this );
    saveDialog.setDefaultSuffix( "*.csv" );
    QString ret = saveDialog.getSaveFileName( this, "Save log file", "log.csv", "*.csv" );

    if ( ret == "" ) {
        return;
    }

    QFile file( ret );
    file.open( QIODevice::WriteOnly | QIODevice::Text );

    QTextStream stream( &file );
    stream << ui->logEdit->toPlainText();
}

void Widget::on_readSensorButton_clicked()
{
    emit doReading( false );
}

void Widget::on_readSensorContButton_clicked()
{
    emit doReading( true );
}

void Widget::on_stopReadingButton_clicked()
{
    emit stopReading();
}

void Widget::on_closeSensorButton_clicked()
{
    colorSensor->closeSensor();

    ui->openButton->setEnabled( true );
    ui->initializeButton->setEnabled( false );
    ui->closeSensorButton->setEnabled( false );
}
