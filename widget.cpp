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
    qRegisterMetaType<ColorSensorAccess::ColorData>("ColorSensorAccess::ColorData");

    connect( this, SIGNAL(doReading(bool)), colorSensor, SLOT(startReading(bool)) );
    connect( this, SIGNAL(stopReading()), colorSensor, SLOT(stopReading()) );
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
}

void Widget::setColorLabel(ColorSensorAccess::ColorData data)
{
    QPixmap map( ui->colorPreviewLabel->pixmap()->size() );
    QPainter painter( &map );
    QColor color = data.getColor();

    // Fill by latest color
    painter.fillRect( map.rect(), color );

    // Calculate text size
    QString str = QString( "B:%1 G:%2 R:%3 IR:%4" ).arg( data.blue ).arg( data.green ).arg( data.red ).arg( data.infraRed );
    QRect strRect = QFontMetrics( painter.font() ).boundingRect( str );

    painter.drawText( QRect( map.width() / 2 - strRect.width() / 2, map.height() / 2 - strRect.height() / 2, strRect.width(), strRect.height() ), str );
}

void Widget::on_openButton_clicked()
{
    if ( !colorSensor->openSensor( ui->sensorPathEdit->text() ) )
    {
        QMessageBox::critical( this, "Error", "Failed to open color sensor" );
    }
}

void Widget::on_initializeButton_clicked()
{
    if( !colorSensor->initializeSensor( (ColorSensorAccess::IntegrationTime)intTimeGroup.checkedId(), (ColorSensorAccess::Gain)gainGroup.checkedId() ) )
    {
        QMessageBox::critical( this, "Error", "Failed to initialize color sensor" );
    }
}

void Widget::on_saveLogButton_clicked()
{
    colorSensor->closeSensor();
}

void Widget::on_pushButton_7_clicked()
{
    ui->logEdit->clear();
}

void Widget::on_pushButton_8_clicked()
{
    // Save log file
    QString ret = QFileDialog::getSaveFileName( this, "Save log file", "", "*.csv" );

    if ( ret == "" ) {
        return;
    }
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
