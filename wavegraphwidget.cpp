#include "wavegraphwidget.h"

WaveGraphWidget::WaveGraphWidget(QWidget *parent) : QWidget(parent)
{
    // Set default colors
    setBgColor( Qt::white );
    setGridColor( Qt::gray );
    setZeroGridColor( Qt::darkMagenta );
    setStrColor( Qt::black );
    setCursorColor( Qt::black );
    setRightCursorColor( Qt::black );
    setFrameColor( Qt::black );
    setYValueColor( Qt::black );

    // Setup default graph status
    setAutoUpdateYMax( false );
    setAutoUpdateYMin( false );

    setYMax( 1 );
    setYMin( -1 );
    setYZero( 0 );
    setXScale( 1 );
    setXGrid( 50 );
    setXName( "x" );

    setUpSize( 1, 10000 );

    resetIterator();

    headIndex = 0;

    validCursor = false;
    validRightCursor = false;

    autoUpdateYMax = false;
    autoUpdateYMin = false;
    autoZeroCenter = false;

    legendOffsetX = 5;
    legendOffsetY = 5;
    legendLineWidth = 2;
    legendLineLength = 10;
    legendFontSize = 20;

    defaultFontSize = 15;

    setCursorWidth( 2 );

    setShowCursor( true );
    setShowRightCursor( false );

    showCursorValue = true;
    showHeadValue   = true;
    showYGridValue  = true;
    showXGridValue  = true;

    defaultHeadUpdate = true;

    rightCursorForceBig = false;

    forceRequestedRawX = false;

    requestRawX = 0;

    yGridCount = 0;
}

QColor WaveGraphWidget::getBgColor() const
{
    return bgColor;
}

void WaveGraphWidget::setBgColor(const QColor &value)
{
    bgColor = value;
}

QColor WaveGraphWidget::getGridColor() const
{
    return gridColor;
}

void WaveGraphWidget::setGridColor(const QColor &value)
{
    gridColor = value;
}

QColor WaveGraphWidget::getZeroGridColor() const
{
    return zeroGridColor;
}

void WaveGraphWidget::setZeroGridColor(const QColor &value)
{
    zeroGridColor = value;
}

QColor WaveGraphWidget::getStrColor() const
{
    return strColor;
}

void WaveGraphWidget::setStrColor(const QColor &value)
{
    strColor = value;
}

void WaveGraphWidget::setUpSize( int columnCount, int queueSize )
{
    // Setup
    this->columnCount = columnCount;
    this->queueSize = queueSize;

    for ( int i = 0; i < columnCount; i++ ) {
        colors << Qt::blue;
        names << "noname";
    }
}

void WaveGraphWidget::enqueueData( const QVector<double> &data )
{
    enqueueData( data, defaultHeadUpdate );
}

void WaveGraphWidget::enqueueData( const QVector<double> &data, bool updateHead )
{
    // Add data to queue
    bool headmove = false;

    dataQueue.append( data );

    // update iterator
    if ( updateHead ) {
        setHead( dataQueue.end() - 1, dataQueue.size() - 1 );

        headmove = true;
    } else if ( dataQueue.size() == 1 ) {
        // head is always valid iterator if dataQueue size is larger than 0
        setHead( dataQueue.begin(), 0 );

        headmove = true;
    }

    // dequeue
    if ( dataQueue.size() > queueSize ) {
        // reset iterator
        if ( cursor == dataQueue.begin() )  cursor++;
        if ( rightCursor == dataQueue.begin() )  rightCursor++;

        // dequeue
        dataQueue.removeFirst();

        // move head
        setHead( head, headIndex - 1 );

        if ( headIndex < 0 ) {
            setHead( dataQueue.begin(), 0 );
        }

        headmove = true;
    }

    emit rangeChanged( 0, dataQueue.size() - 1 );

    if ( headmove ) {
        emitHeadChanged( true );
    }

    update();
}

double WaveGraphWidget::getXScale() const
{
    return xScale;
}

void WaveGraphWidget::setXScale(double value)
{
    xScale = value;

    update();
}

void WaveGraphWidget::setXScale(int value)
{
    setXScale((double)value);
}

void WaveGraphWidget::moveHeadToHead(bool emitSignal, bool indexOnly)
{
    // ヘッドイテレーターを最新のデータに移動する
    if ( dataQueue.size() == 0 ) {
        return;
    }

    setHead( dataQueue.end() - 1, dataQueue.size() - 1 );

    if ( emitSignal ) {
        emit headChanged( headIndex );

        if ( !indexOnly ) {
            emit headChanged( (*head)[0] );
        }
    }
}

bool WaveGraphWidget::getAutoUpdateYMax() const
{
    return autoUpdateYMax;
}

void WaveGraphWidget::setAutoUpdateYMax(bool value)
{
    autoUpdateYMax = value;
}

bool WaveGraphWidget::getAutoUpdateYMin() const
{
    return autoUpdateYMin;
}

void WaveGraphWidget::setAutoUpdateYMin(bool value)
{
    autoUpdateYMin = value;
}

double WaveGraphWidget::getXGrid() const
{
    return xGrid;
}

void WaveGraphWidget::setXGrid(double value)
{
    xGrid = value;
}

double WaveGraphWidget::getYMax() const
{
    return yMax;
}

void WaveGraphWidget::setYMax(double value)
{
    yMax = value;
}

double WaveGraphWidget::getYMin() const
{
    return yMin;
}

void WaveGraphWidget::setYMin(double value)
{
    yMin = value;
}

double WaveGraphWidget::getYZero() const
{
    return yZero;
}

void WaveGraphWidget::setYZero(double value)
{
    yZero = value;
}

bool WaveGraphWidget::getAutoZeroCenter() const
{
    return autoZeroCenter;
}

void WaveGraphWidget::setAutoZeroCenter(bool value)
{
    autoZeroCenter = value;
}

QList<QColor> WaveGraphWidget::getColors() const
{
    return colors;
}

void WaveGraphWidget::setColors(const QList<QColor> &value)
{
    colors = value;
}

QList<QString> WaveGraphWidget::getNames() const
{
    return names;
}

void WaveGraphWidget::setNames(const QList<QString> &value)
{
    names = value;
}

void WaveGraphWidget::addColorFilter(double left, double right, QColor color)
{
    // Add color filter
    ColorFilter filter( QVector<double>() << left << right, color );

    colorFilterList.append( filter );
}

void WaveGraphWidget::clearColorFilter()
{
    colorFilterList.clear();
}

WaveGraphWidget::DataQueue::iterator WaveGraphWidget::pixPosToIterator(int x)
{
    // Get iterator from widget pix pos X
    double ix;
    auto ret = dataQueue.end();

    // check available
    if ( dataQueue.size() == 0 ) {
        return ret;
    }

    // 強制指定されたxを基準とするためのオフセット作成
    double offX = 0;

    if ( forceRequestedRawX && requestRawX != (*head)[0] ) {
        offX = ( (*head)[0] - requestRawX ) * xScale;
    }

    ix = -offX;

    for ( auto it = head; true; it-- ) {
        int dataX = width() - ix;

        if ( dataX < 0 ) {
            break;
        } else if ( dataX <= x ) {
            ret = it;
            break;
        }

        if ( it == dataQueue.begin() ) {
            // Exit
            ret = it;
            break;
        } else {
            // calc next pix x
            ix += ( (*it)[0] - (*( it - 1 ))[0] ) * xScale;
        }
    }

    return ret;
}

void WaveGraphWidget::resetIterator()
{
    head = dataQueue.end();
    cursor = dataQueue.end();
    rightCursor = dataQueue.end();

    validCursor = false;
    validRightCursor = false;
}

void WaveGraphWidget::updateCursor(int x)
{
    // Update cursor iterator
    cursor = pixPosToIterator( x );

    if ( cursor != dataQueue.end() ) {
        // 追い越せない
        if ( rightCursorForceBig && validRightCursor ) {
            if ( (*rightCursor)[0] < (*cursor)[0] ) {
                cursor = rightCursor;
            }
        }

        validCursor = true;

        emit moveCursor( QPair<int, QVector<double> >( getCurrentPixXFromRawX( (*cursor)[0] ), *cursor ) );
    } else {
        validCursor = false;
    }
}

void WaveGraphWidget::updateRightCursor(int x)
{
    // Update right cursor iterator
    rightCursor = pixPosToIterator( x );

    if ( rightCursor != dataQueue.end() ) {
        // 追い越せない
        if ( rightCursorForceBig && validCursor ) {
            if ( (*rightCursor)[0] < (*cursor)[0] ) {
                rightCursor = cursor;
            }
        }

        validRightCursor = true;

        emit moveRightCursor( QPair<int, QVector<double> >( getCurrentPixXFromRawX( (*rightCursor)[0] ), *rightCursor ) );
    } else {
        validRightCursor = false;
    }
}

int WaveGraphWidget::getCurrentPixXFromRawX(double x)
{
    // Calculate display pix x from raw x
    if ( dataQueue.size() == 0 ) return 0;

    double headX = (*head)[0];

    return width() - ( headX - x ) * xScale;
}

void WaveGraphWidget::emitHeadChanged(bool indexOnly)
{
    // indexOnly option is needed to prevent signal/slot loop

    if ( dataQueue.size() > 0 ) {
        emit headChanged( headIndex );

        if ( !indexOnly ) {
            emit headChanged( (*head)[0] );
        }
    } else {
        // emit headChanged( 0 );
        // emit headChanged( 0 );
    }
}

void WaveGraphWidget::setHead(DataQueue::iterator head, int headIndex, bool overwriteRequest)
{
    this->head = head;
    this->headIndex = headIndex;

    if ( overwriteRequest && dataQueue.size() > 0 && head != dataQueue.end() && head->size() > 0 ) {
        requestRawX = (*head)[0];
    }
}

void WaveGraphWidget::setHead(int headIndex)
{
    this->headIndex = headIndex;
}

int WaveGraphWidget::getYGridCount() const
{
    return yGridCount;
}

void WaveGraphWidget::setYGridCount(int value)
{
    yGridCount = value;
}
bool WaveGraphWidget::getShowXGridValue() const
{
    return showXGridValue;
}

void WaveGraphWidget::setShowXGridValue(bool value)
{
    showXGridValue = value;
}


QColor WaveGraphWidget::getYValueColor() const
{
    return yValueColor;
}

void WaveGraphWidget::setYValueColor(const QColor &value)
{
    yValueColor = value;
}

QColor WaveGraphWidget::getFrameColor() const
{
    return frameColor;
}

void WaveGraphWidget::setFrameColor(const QColor &value)
{
    frameColor = value;
}

void WaveGraphWidget::setRequestRawX(double value)
{
    requestRawX = value;
}

double WaveGraphWidget::getRequestRawX() const
{
    return requestRawX;
}

bool WaveGraphWidget::getForceRequestedRawX() const
{
    return forceRequestedRawX;
}

void WaveGraphWidget::setForceRequestedRawX(bool value)
{
    forceRequestedRawX = value;
}

bool WaveGraphWidget::getRightCursorForceBig() const
{
    return rightCursorForceBig;
}

void WaveGraphWidget::setRightCursorForceBig(bool value)
{
    rightCursorForceBig = value;
}

void WaveGraphWidget::clear()
{
    clearQueue();
    clearColorFilter();
}

double WaveGraphWidget::getStartRawX()
{
    if ( !dataQueue.size() ) return -1;
    if ( !dataQueue.begin()->size() ) return -1;

    return (*dataQueue.begin())[0];
}

double WaveGraphWidget::getEndRawX()
{
    if ( !dataQueue.size() ) return -1;
    if ( !( dataQueue.end() - 1 )->size() ) return -1;

    return ( *( dataQueue.end() - 1 ) )[0];
}

bool WaveGraphWidget::getValidRightCursor() const
{
    return validRightCursor;
}

bool WaveGraphWidget::getValidCursor() const
{
    return validCursor;
}
bool WaveGraphWidget::getDefaultHeadUpdate() const
{
    return defaultHeadUpdate;
}

void WaveGraphWidget::setQueueDataFromList(QList<QVector<double> > &data, double xUnit)
{
    // キューのデータをクリアしてリストをセット
    if ( data.size() == 0 ) {
        return;
    }

    if ( data[0].size() < 2 ) {
        return;
    }

    clearQueue();

    // Read all data
    setUpSize( data[0].size() - 1, data.size() );

    for ( auto elem : data ) {
        elem[0] = elem[0] * xUnit;

        enqueueData( elem, false );
    }

    moveHeadToHead( true, true );
}

QPair<int, QVector<double> > WaveGraphWidget::getShiftedCursorValue(int shift, bool forceShift)
{
    // 現在のカーソル位置から、指定数ずらした位置の値を取得
    if ( !validCursor ) {
        return getCursorValue();
    }

    if ( shift == 0 ) {
        return getCursorValue();
    }

    auto c = cursor;
    bool over = false;

    if ( shift > 0 ) {
        for ( int i = 0; i < shift; i++ ) {
            ++c;

            if ( c == dataQueue.end() ) {
                over = true;
                --c;
                break;
            }
        }
    } else {
        for ( int i = 0; i < qAbs( shift ); i++ ) {
            if ( c == dataQueue.begin() ) {
                over = true;
                break;
            }

            --c;
        }
    }

    if ( over && !forceShift ) {
        return QPair<int, QVector<double> >( -1, QVector< double >() );
    }

    return QPair<int, QVector<double> >( getCurrentPixXFromRawX( (*c)[0] ), *c );
}

void WaveGraphWidget::setDefaultHeadUpdate(bool value)
{
    defaultHeadUpdate = value;
}

int WaveGraphWidget::getDefaultFontSize() const
{
    return defaultFontSize;
}

void WaveGraphWidget::setDefaultFontSize(int value)
{
    defaultFontSize = value;
}


void WaveGraphWidget::clearQueue()
{
    // Clear queue data
    resetIterator();

    dataQueue.clear();

    update();
}
int WaveGraphWidget::getLegendFontSize() const
{
    return legendFontSize;
}

void WaveGraphWidget::setLegendFontSize(int value)
{
    legendFontSize = value;
}

bool WaveGraphWidget::getShowYGridValue() const
{
    return showYGridValue;
}

void WaveGraphWidget::setShowYGridValue(bool value)
{
    showYGridValue = value;
}

QPair<int, QVector<double> > WaveGraphWidget::getCursorValue()
{
    // Return cursor value
    if ( validCursor ) {
        return QPair<int, QVector<double> >( getCurrentPixXFromRawX( (*cursor)[0] ), *cursor );
    } else {
        return QPair<int, QVector<double> >( -1, QVector< double >() );
    }
}

QPair<int, QVector<double> > WaveGraphWidget::getRightCursorValue()
{
    // Return cursor value
    if ( validRightCursor ) {
        return QPair<int, QVector<double> >( getCurrentPixXFromRawX( (*rightCursor)[0] ), *rightCursor );
    } else {
        return QPair<int, QVector<double> >( -1, QVector< double >() );
    }
}

QPair<int, QVector<double> > WaveGraphWidget::getHeadValue()
{
    // ヘッドイテレータの値取得
    if ( dataQueue.count() > 0 ) {
        return QPair<int, QVector<double> >( getCurrentPixXFromRawX( (*head)[0] ), *head );
    } else {
        return QPair<int, QVector<double> >( -1, QVector< double >() );
    }
}

QString WaveGraphWidget::getXName() const
{
    return xName;
}

void WaveGraphWidget::setXName(const QString &value)
{
    xName = value;
}


bool WaveGraphWidget::getShowHeadValue() const
{
    return showHeadValue;
}

void WaveGraphWidget::setShowHeadValue(bool value)
{
    showHeadValue = value;
}

bool WaveGraphWidget::getShowCursorValue() const
{
    return showCursorValue;
}

void WaveGraphWidget::setShowCursorValue(bool value)
{
    showCursorValue = value;
}

int WaveGraphWidget::getHeadIndex() const
{
    return headIndex;
}

void WaveGraphWidget::setHeadIndex(int value)
{
    // Set head index and iterator
    if ( dataQueue.size() == 0 ) {
        setHead( dataQueue.end(), 0 );

        return;
    }

    if ( value == headIndex ) {
        return;
    }

    if ( value < 0 ) {
        value = 0;
    } else if ( value >= dataQueue.size() ) {
        value = dataQueue.size() - 1;
    }

    int diff = value - headIndex;

    // move iterator
    setHead( head + diff, value );

    emitHeadChanged();

    update();
}

void WaveGraphWidget::setHeadFromRawX(double x, const MoveMode mode, bool emitChanged, bool owReq)
{
    // Move head to real x nearest iterator
    if ( dataQueue.size() == 0 ) return;

    double headX = (*head)[0];

    // 必ず再描画は行う
    update();

    // 要求するxを保存
    if ( !owReq ) {
        setRequestRawX( x );
    }

    if ( headX == x ) {
        return;
    }

    bool updated = false;

    // Search
    auto newHead = head;
    double beforeX = headX;
    auto beforeHead = head;
    int diff = 0;

    if ( headX < x ) {
        for ( newHead = head; newHead != dataQueue.end(); newHead++, diff++ ) {
            double newHeadX = (*newHead)[0];

            // Nearet values are found
            if ( newHeadX == x ) {
                // Update head
                setHead( newHead, headIndex + diff, owReq );

                updated = true;

                break;
            } else if ( newHeadX > x ) {
                // Check nearest and update head
                if ( mode == Nearest ) {
                    if ( qAbs( newHeadX - x ) <= qAbs( beforeX - x ) ) {
                        setHead( newHead, headIndex + diff, owReq );
                    } else {
                        setHead( beforeHead, headIndex + ( diff - 1 ), owReq );
                    }
                } else if ( mode == SmallNearest ) {
                    setHead( beforeHead, headIndex + ( diff - 1 ), owReq );
                } else if ( mode == LargeNearest ) {
                    setHead( newHead, headIndex + diff, owReq );
                }

                updated = true;

                break;
            }

            beforeX = newHeadX;
            beforeHead = newHead;
        }

        // reach the end of loop
        if ( newHead == dataQueue.end() ) {
            setHead( dataQueue.end() - 1, dataQueue.count() - 1, owReq );

            updated = true;
        }
    } else {
        for ( newHead = head; true; newHead--, diff++ ) {
            double newHeadX = (*newHead)[0];

            // Nearet values are found
            if ( newHeadX == x ) {
                // Update head
                setHead( newHead, headIndex - diff, owReq );

                updated = true;

                break;
            } else if ( newHeadX < x ) {
                // Check nearest and update head
                if ( mode == Nearest ) {
                    if ( qAbs( newHeadX - x ) <= qAbs( beforeX - x ) ) {
                        setHead( newHead, headIndex - diff, owReq );
                    } else {
                        setHead( beforeHead, headIndex - ( diff - 1 ), owReq );
                    }
                } else if ( mode == LargeNearest ) {
                    setHead( beforeHead, headIndex - ( diff - 1 ), owReq );
                } else if ( mode == SmallNearest ) {
                    setHead( newHead, headIndex - diff, owReq );
                }

                updated = true;

                break;
            }

            // end check
            if ( newHead == dataQueue.begin() ) {
                break;
            }

            beforeX = newHeadX;
            beforeHead = newHead;
        }

        // reach the end of loop
        if ( newHead == dataQueue.begin() && updated == false ) {
            setHead( dataQueue.begin(), 0, owReq );

            updated = true;
        }
    }

    if ( updated && emitChanged ) {
        emitHeadChanged( true );
    }
}

void WaveGraphWidget::setHeadFromRawXSmall(double x)
{
    setHeadFromRawX( x, SmallNearest, true );
}

void WaveGraphWidget::setHeadFromRawXSmallForce(double x)
{
    setHeadFromRawX( x, SmallNearest, true, false );
}

int WaveGraphWidget::getQueueSize()
{
    return dataQueue.size();
}

int WaveGraphWidget::getCursorWidth() const
{
    return cursorWidth;
}

void WaveGraphWidget::setCursorWidth(int value)
{
    cursorWidth = value;
}

void WaveGraphWidget::clearCursor()
{
    validCursor = false;
    cursor = dataQueue.end();
}

void WaveGraphWidget::clearRightCursor()
{
    validRightCursor = false;
    rightCursor = dataQueue.end();
}

QColor WaveGraphWidget::getRightCursorColor() const
{
    return rightCursorColor;
}

void WaveGraphWidget::setRightCursorColor(const QColor &value)
{
    rightCursorColor = value;
}

QColor WaveGraphWidget::getCursorColor() const
{
    return cursorColor;
}

void WaveGraphWidget::setCursorColor(const QColor &value)
{
    cursorColor = value;
}

bool WaveGraphWidget::getShowRightCursor() const
{
    return showRightCursor;
}

void WaveGraphWidget::setShowRightCursor(bool value)
{
    showRightCursor = value;
}

bool WaveGraphWidget::getShowCursor() const
{
    return showCursor;
}

void WaveGraphWidget::setShowCursor(bool value)
{
    showCursor = value;
}

void WaveGraphWidget::paintEvent(QPaintEvent *)
{
    // draw
    QPainter p( this );
    QPen pen;
    QFont font( p.font() );

    // Clear bg
    p.fillRect( rect(), bgColor );

    // 外枠を描く
    p.setPen( frameColor );
    p.drawRect( 0, 0, width() - 1, height() - 1 );

    // Draw x grid
    p.setPen( gridColor );

    for ( double x = xScale * xGrid; x < width(); x += xScale * xGrid ) {
        double devX = width() - x;
        double xVal;

        if ( forceRequestedRawX ) {
            xVal = requestRawX - x / xScale;
        } else {
            xVal = (*head)[0] - x / xScale;
        }

        p.drawLine( devX, 0, devX, height() );

        // Draw x grid value
        if ( showXGridValue ) {
            font.setPixelSize( defaultFontSize );
            p.setFont( font );

            QString str = QString::number( xVal );
            QRect fr = p.fontMetrics().boundingRect( str );

            pen.setColor( gridColor );
            p.setPen( pen );

            p.drawText( QRectF( devX - fr.width() - 2, height() - fr.height(), fr.width() + 5, fr.height() + 5 ), str );

        }
    }

    // Check available
    if ( dataQueue.size() < 2 ) {
        return;
    }

    // Create draw queue
    QQueue<QPair<int, QVector<double> > > drawQueue;
    QList<double> minRawY, maxRawY;
    double localMinY, localMaxY;
    double x = 0;

    // 強制指定されたxを基準とするためのオフセット作成
    double offX = 0;

    if ( forceRequestedRawX && requestRawX != (*head)[0] ) {
        offX = ( (*head)[0] - requestRawX ) * xScale;
    }

    x -= offX;

    for ( int i = 0; i < (*head).size(); i++ ) {
        minRawY << (*head)[i];
        maxRawY << (*head)[i];
    }

    for ( auto it = head; true; it-- ) {
        // enqueue
        drawQueue << QPair<int, QVector<double> >( x, *it );

        // check pix x
        if ( x >= width() ) {
            break;
        }

        // min, max
        for ( int i = 0; i < (*it).size(); i++ ) {
            if ( minRawY[i] > (*it)[i] ) {
                minRawY[i] = (*it)[i];
            }

            if ( maxRawY[i] < (*it)[i] ) {
                maxRawY[i] = (*it)[i];
            }
        }

        if ( it == dataQueue.begin() ) {
            // Exit
            break;
        } else {
            // calc next pix x
            x += ( (*it)[0] - (*( it - 1 ))[0] ) * xScale;
        }
    }

    // deceide local min max
    localMinY = minRawY[1];
    localMaxY = maxRawY[1];

    for ( int i = 1; i < minRawY.size(); i++ ) {
        if ( localMinY > minRawY[i] ) {
            localMinY = minRawY[i];
        }

        if ( localMaxY < maxRawY[i] ) {
            localMaxY = maxRawY[i];
        }
    }

    if ( autoZeroCenter ) {
        double localMax = qMax( qAbs( localMinY ), qAbs( localMaxY ) );

        localMinY = -localMax;
        localMaxY = localMax;
    }

    // Draw zero line
    if ( localMinY <= 0 && localMaxY >= 0 ) {
        double zeroY = height() - ( 0 - localMinY ) / ( localMaxY - localMinY ) * height();

        p.setPen( zeroGridColor );
        p.drawLine( 0, zeroY, width(), zeroY );

        // Draw zero value
        if ( showYGridValue ) {
            font.setPixelSize( defaultFontSize );
            p.setFont( font );

            QString str = QString::number( 0 );
            QRect fr = p.fontMetrics().boundingRect( str );

            pen.setColor( zeroGridColor );
            p.setPen( pen );
            p.drawText( QRectF( width() - fr.width() - 2, zeroY - fr.height(), fr.width() + 5, fr.height() + 5 ), str );
        }
    }

    // Draw Y Grid
    if ( yGridCount > 0 ) {
        double startY;
        double yWidth;
        int count;

        if ( autoZeroCenter ) {
            startY = localMinY + qAbs( localMaxY - localMinY ) / ( ( yGridCount + 1 ) * 2 );
            yWidth = qAbs( localMaxY - localMinY ) / ( ( yGridCount + 1 ) * 2 );
            count = yGridCount;
        } else {
            startY = localMinY + qAbs( localMaxY - localMinY ) / ( ( yGridCount + 1 ) );
            yWidth = qAbs( localMaxY - localMinY ) / ( ( yGridCount + 1 ) );
            count = yGridCount;
        }

        for ( int i = 0; i < count; i++ ) {
            double pixY;
            pen.setColor( gridColor );
            p.setPen( pen );

            if ( autoZeroCenter ) {
                pixY = ( startY + i * yWidth - localMinY ) / ( localMaxY - localMinY ) * height();

                p.drawLine( 0, height() - pixY, width(), height() - pixY );
                p.drawLine( 0, pixY, width(), pixY );
            } else {
                pixY = ( startY + i * yWidth - localMinY ) / ( localMaxY - localMinY ) * height();

                p.drawLine( 0, height() - pixY, width(), height() - pixY );
            }

            if ( showYGridValue ) {
                if ( autoZeroCenter ) {
                    double rawYB = startY + i * yWidth;
                    double rawYT = startY + ( ( 1 + count ) * 2 - i - 2 ) * yWidth;
                    font.setPixelSize( defaultFontSize );
                    p.setFont( font );

                    QString strT = QString::number( rawYT );
                    QRect frT = p.fontMetrics().boundingRect( strT );
                    QString strB = QString::number( rawYB );
                    QRect frB = p.fontMetrics().boundingRect( strB );

                    p.setPen( pen );

                    p.fillRect( width() - frT.width() - 2, pixY - frT.height() - 1, frT.width(), frT.height(), QColor( 255, 255, 255, 180 ) );
                    p.drawText( QRectF( width() - frT.width() - 2, pixY - frT.height() - 1, frT.width() + 5, frT.height() + 5 ), strT );
                    p.fillRect( width() - frB.width() - 2, height() - pixY - frB.height() - 1, frB.width(), frB.height(), QColor( 255, 255, 255, 180 ) );
                    p.drawText( QRectF( width() - frB.width() - 2, height() - pixY - frB.height() - 1, frB.width() + 5, frB.height() + 5 ), strB );
                } else {
                    double rawY = startY + i * yWidth;
                    font.setPixelSize( defaultFontSize );
                    p.setFont( font );

                    QString str = QString::number( rawY );
                    QRect fr = p.fontMetrics().boundingRect( str );

                    p.setPen( pen );

                    p.fillRect( width() - fr.width() - 2, height() - pixY - fr.height() - 1, fr.width(), fr.height(), QColor( 255, 255, 255, 180 ) );
                    p.drawText( QRectF( width() - fr.width() - 2, height() - pixY - fr.height() - 1, fr.width() + 5, fr.height() + 5 ), str );
                }
            }
        }
    }

    // Draw Top Botto Y Grid value
    if ( showYGridValue ) {
        font.setPixelSize( defaultFontSize );
        p.setFont( font );

        QString strTop = QString::number( localMaxY );
        QString strBottom = QString::number( localMinY );
        QRect frTop = p.fontMetrics().boundingRect( strTop );
        QRect frBottom = p.fontMetrics().boundingRect( strBottom );

        pen.setColor( yValueColor );
        p.setPen( pen );

        p.fillRect( width() - frTop.width() - 2, 1, frTop.width(), frTop.height(), QColor( 255, 255, 255, 180 ) );
        p.fillRect( width() - frBottom.width() - 2, height() - frBottom.height() - 1, frBottom.width(), frBottom.height(), QColor( 255, 255, 255, 180 ) );

        p.drawText( QRectF( width() - frTop.width() - 2, 1, frTop.width() + 5, frTop.height() + 5 ), strTop );
        p.drawText( QRectF( width() - frBottom.width() - 2, height() - frBottom.height() - 1, frBottom.width() + 5, frBottom.height() + 5 ), strBottom );
    }

    // Draw wave
    QList<QPolygon> polygonList;

    // Use AA
    // p.setRenderHint( QPainter::Antialiasing );

    for ( int col = 0; col < columnCount; col++ ) {
        polygonList << QPolygon();
    }

    for ( int i = 0; i < drawQueue.size(); i++ ) {
        auto now = drawQueue[i];

        // all column
        for ( int col = 0; col < columnCount; col++ ) {
            double nowY = now.second[1 + col];
            double topY;
            double bottomY;

            if ( autoUpdateYMax ) {
                topY = localMaxY;
            } else {
                topY = yMax;
            }

            if ( autoUpdateYMin ) {
                bottomY = localMinY;
            } else {
                bottomY = yMin;
            }

            // calc pix pos
            double nowDrawY = height() - ( nowY - bottomY ) / ( topY - bottomY ) * height();
            int nowDrawX = width() - now.first;

            // Add point to polygon
            polygonList[col] << QPoint( nowDrawX, nowDrawY );
        }
    }

    for ( int col = 0; col < columnCount; col++ ) {
        pen.setColor( colors[col] );
        pen.setWidth( 1 );
        p.setPen( pen );
        p.drawPolyline( polygonList[col] );
    }

    // Disable AA
    // p.setRenderHint( QPainter::Antialiasing, false );

    // Draw color filter
    for ( auto const &filter : colorFilterList ) {
        double headX = (*head)[0];
        double tailX = headX - ( width() / xScale );
        double fStart = filter.first[0];
        double fEnd = filter.first[1];

        // flip
        if ( fStart > fEnd ) {
            fStart = filter.first[1];
            fEnd   = filter.first[0];
        }

        // Check need drawing
        if ( fStart > headX || fEnd < tailX ) {
            continue;
        }

        int xStart = width() - ( headX - fStart ) * xScale + offX;
        int xEnd   = width() - ( headX - fEnd ) * xScale + offX;

        if ( xStart < 0 ) xStart = 0;
        if ( xEnd >= width() ) xEnd = width();

        p.setPen( filter.second );
        p.fillRect( xStart, 0, xEnd - xStart, height(), filter.second );
    }

    // Draw legend
    font.setPixelSize( legendFontSize );
    p.setFont( font );

    for ( int col = 0; col < columnCount; col++ ) {
        // Create str
        QString str = names[col];

        if ( showHeadValue ) {
            str = str + " : " + QString::number( (*head)[col + 1], 'f' );
        }

        QRect fr = p.fontMetrics().boundingRect( str );

        p.fillRect( legendOffsetX, legendOffsetY + col * fr.height(), fr.width() + legendLineLength + 5, fr.height(), QColor( 255, 255, 255, 200 ) );

        pen.setColor( colors[col] );
        pen.setWidth( legendLineWidth );
        p.setPen( pen );
        p.drawLine( legendOffsetX, legendOffsetY + col * fr.height() + fr.height() / 2, legendOffsetX + legendLineLength, legendOffsetY + col * fr.height() + fr.height() / 2 );

        pen.setColor( Qt::black );
        p.setPen( pen );
        p.drawText( QRectF( legendOffsetX + legendLineLength + 5, legendOffsetY + col * fr.height(), fr.width() + 5, fr.height() + 5 ), str );
    }

    // Draw cursor
    if ( showCursor && ( validCursor ) ) {
        int cursorX = width() - ( (*head)[0] - (*cursor)[0] ) * xScale + offX;

        if ( cursorX >= 0 && cursorX < width() ) {
            pen.setColor( cursorColor );
            pen.setWidth( cursorWidth );
            p.setPen( pen );
            p.drawLine( cursorX, 0, cursorX, height() );

            // Draw cursor value
            if ( showCursorValue ) {
                font.setPixelSize( defaultFontSize );
                p.setFont( font );

                for ( int col = 0; col < columnCount; col++ ) {
                    // Create str
                    QString str = names[col] + " : " + QString::number( (*cursor)[col + 1], 'f' );
                    QRect fr = p.fontMetrics().boundingRect( str );

                    p.fillRect( cursorX - fr.width() - 10, height() - ( columnCount - col ) * fr.height() - 1, fr.width(), fr.height(), QColor( 255, 255, 255, 200 ) );

                    pen.setColor( colors[col] );
                    pen.setWidth( 2 );
                    p.setPen( pen );
                    p.drawLine( cursorX - fr.width() - 10 - 10, height() - ( columnCount - 1 - col ) * fr.height() - fr.height() / 2, cursorX - fr.width() - 10 - 5, height() - ( columnCount - 1 - col ) * fr.height() - fr.height() / 2 );

                    pen.setColor( Qt::black );
                    p.setPen( pen );
                    p.drawText( QRectF( cursorX - fr.width() - 10, height() - ( columnCount - col ) * fr.height() - 1, fr.width() + 5, fr.height() + 5 ), str );
                }

                // Draw x value
                QString str = xName + " : " + QString::number( (*cursor)[0], 'f' );
                QRect fr = p.fontMetrics().boundingRect( str );

                //qDebug() << str;

                p.fillRect( cursorX - fr.width() - 10, height() - ( columnCount + 1 ) * fr.height() - 1, fr.width(), fr.height(), QColor( 255, 255, 255, 200 ) );

                pen.setColor( Qt::black );
                p.setPen( pen );
                p.drawText( QRectF( cursorX - fr.width() - 10, height() - ( columnCount + 1 ) * fr.height() - 1, fr.width() + 5, fr.height() + 5 ), str );
            }
        }
    }

    // Draw right cursor
    if ( showRightCursor && validRightCursor ) {
        int cursorX = width() - ( (*head)[0] - (*rightCursor)[0] ) * xScale;

        if ( cursorX >= 0 && cursorX <= width() ) {
            pen.setColor( cursorColor );
            pen.setWidth( cursorWidth );
            p.setPen( pen );
            p.drawLine( cursorX, 0, cursorX, height() );
        }
    }
}

void WaveGraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    // Mouse move handler
    if ( Qt::LeftButton & event->buttons() ) {
        updateCursor( event->pos().x() );

        update();
    } else if ( Qt::RightButton & event->buttons() ) {
        updateRightCursor( event->pos().x() );

        update();
    }
}


void WaveGraphWidget::mousePressEvent(QMouseEvent *event)
{
    // Mouse press handler
    if ( Qt::LeftButton & event->buttons() ) {
        updateCursor( event->pos().x() );

        update();
    } else if ( Qt::RightButton & event->buttons() ) {
        updateRightCursor( event->pos().x() );

        update();
    }
}
