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

    gainGroup.addButton( ui->lowButton, ColorSensorAccess::Low );
    gainGroup.addButton( ui->highButton, ColorSensorAccess::High );

    // Initialize Graph widgets
    ui->graphWidget->setLabel( tr( "RAW data" ) );
    ui->graphWidget->wave->setXScale( 10 );
    ui->graphWidget->wave->setXGrid( 10 );
    ui->graphWidget->wave->setLegendFontSize( 12 );
    ui->graphWidget->wave->setDefaultFontSize( 12 );
    ui->graphWidget->wave->setShowCursor( true );
    ui->graphWidget->wave->setXName( "" );
    ui->graphWidget->wave->setForceRequestedRawX( true );
    ui->graphWidget->wave->setUpSize( 4, 10000 );
    ui->graphWidget->wave->setYGridCount( 1 );
    ui->graphWidget->wave->setAutoUpdateYMax( true );
    ui->graphWidget->wave->setAutoUpdateYMin( true );
    ui->graphWidget->wave->setAutoZeroCenter( true );
    ui->graphWidget->wave->setNames( QStringList() << "B" << "G" << "R" << "IR" );
    ui->graphWidget->wave->setColors( QList<QColor>() << Qt::blue << Qt::darkGreen << Qt::red << Qt::darkRed );
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

    // Push data to graph

    // Show last integration time if in manual integration mode
    if ( colorSensor->getManualIntegrationMode() ) {
        ui->intTimeLabel->setText( QString( "Last integration time : %1[ms]" ).arg( colorSensor->getLastElapsedNanosec() / 1000.0 / 1000 ) );
    } else {
        ui->intTimeLabel->setText( "Integration time measuring is not supported" );
    }
}

void Widget::setDataToGraph(ColorSensorAccess::ColorData data)
{
    ui->graphWidget->wave->enqueueData( QVector<double>( { (double)data.blue, (double)data.green, (double)data.red, (double)data.infraRed } ) );
}

void Widget::statusMessage(QString str)
{
    ui->intTimeLabel->setText( str );
}

void Widget::enableSensorButtons(bool enable)
{
    ui->openButton->setEnabled( !enable );
    ui->initializeButton->setEnabled( enable );
    ui->closeSensorButton->setEnabled( enable );
    ui->readSensorButton->setEnabled( enable );
    ui->readSensorContButton->setEnabled( enable );
    ui->stopReadingButton->setEnabled( enable );
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
    if ( ir > max ) {
        max = ir;
    }

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

    enableSensorButtons();

    statusMessage( "Device Opening is completed" );
}

void Widget::on_initializeButton_clicked()
{
    if( !colorSensor->initializeSensor( (ColorSensorAccess::IntegrationTime)intTimeGroup.checkedId(), ui->intTimeManButton->isChecked(), ui->intTimeSpinBox->value(), (ColorSensorAccess::Gain)gainGroup.checkedId() ) )
    {
        QMessageBox::critical( this, "Error", "Failed to initialize color sensor" );
        return;
    }

    statusMessage( "Initialization is completed" );
}

void Widget::on_pushButton_7_clicked()
{
    ui->logEdit->clear();
}

void Widget::on_pushButton_8_clicked()
{
    // Save log file
    QFileDialog saveDialog( this );
    saveDialog.setDefaultSuffix( "csv" );
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

    enableSensorButtons( false );
}
