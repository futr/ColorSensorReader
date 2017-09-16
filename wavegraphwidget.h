#ifndef WAVEGRAPHWIDGET_H
#define WAVEGRAPHWIDGET_H

#include <QWidget>
#include <QColor>
#include <QList>
#include <QQueue>
#include <QPainter>
#include <QPair>
#include <QPolygonF>
#include <QMouseEvent>
#include <QLinkedList>
#include <QDebug>

class WaveGraphWidget : public QWidget
{
    Q_OBJECT

public:
    typedef QPair<QVector<double>, QColor> ColorFilter;
    typedef enum  {
        Nearest,
        SmallNearest,
        LargeNearest,
    } MoveMode;

private:
    typedef QLinkedList<QVector<double> > DataQueue;

public:
    explicit WaveGraphWidget(QWidget *parent = 0);

    QColor getBgColor() const;
    void setBgColor(const QColor &value);
    QColor getGridColor() const;
    void setGridColor(const QColor &value);
    QColor getZeroGridColor() const;
    void setZeroGridColor(const QColor &value);
    QColor getStrColor() const;
    void setStrColor(const QColor &value);
    void setUpSize(int columnCount, int queueSize );
    void updateHead();
    double getXScale() const;
    bool getAutoUpdateYMax() const;
    void setAutoUpdateYMax(bool value);
    bool getAutoUpdateYMin() const;
    void setAutoUpdateYMin(bool value);
    double getXGrid() const;
    void setXGrid(double value);
    double getYMax() const;
    void setYMax(double value);
    double getYMin() const;
    void setYMin(double value);
    double getYZero() const;
    void setYZero(double value);
    bool getAutoZeroCenter() const;
    void setAutoZeroCenter(bool value);
    QList<QColor> getColors() const;
    void setColors(const QList<QColor> &value);
    QList<QString> getNames() const;
    void setNames(const QList<QString> &value);
    void addColorFilter( double left, double right, QColor color );
    void clearColorFilter();
    bool getShowCursor() const;
    void setShowCursor(bool value);
    bool getShowRightCursor() const;
    void setShowRightCursor(bool value);
    QColor getCursorColor() const;
    void setCursorColor(const QColor &value);
    QColor getRightCursorColor() const;
    void setRightCursorColor(const QColor &value);
    int getCursorWidth() const;
    void setCursorWidth(int value);
    void clearCursor();
    void clearRightCursor();
    int getHeadIndex() const;
    int getQueueSize();
    bool getShowCursorValue() const;
    void setShowCursorValue(bool value);
    bool getShowHeadValue() const;
    void setShowHeadValue(bool value);
    QString getXName() const;
    void setXName(const QString &value);
    bool getShowYGridValue() const;
    void setShowYGridValue(bool value);
    QPair<int, QVector<double> > getCursorValue();
    QPair<int, QVector<double> > getRightCursorValue();
    QPair<int, QVector<double> > getHeadValue();
    int getLegendFontSize() const;
    void setLegendFontSize(int value);
    int getDefaultFontSize() const;
    void setDefaultFontSize(int value);
    bool getDefaultHeadUpdate() const;
    void setQueueDataFromList(QList<QVector<double> > &data , double xUnit);
    QPair<int, QVector<double> > getShiftedCursorValue(int shift , bool forceShift);
    bool getValidCursor() const;
    bool getValidRightCursor() const;
    bool getRightCursorForceBig() const;
    void setRightCursorForceBig(bool value);
    void clear();
    double getStartRawX();
    double getEndRawX();
    bool getForceRequestedRawX() const;
    void setForceRequestedRawX(bool value);
    double getRequestRawX() const;
    void setRequestRawX(double value);

    QColor getFrameColor() const;
    void setFrameColor(const QColor &value);

    QColor getYValueColor() const;
    void setYValueColor(const QColor &value);

    bool getShowXGridValue() const;
    void setShowXGridValue(bool value);

    int getYGridCount() const;
    void setYGridCount(int value);

private:
    DataQueue::iterator pixPosToIterator( int x );
    void resetIterator();
    void updateCursor( int x );
    void updateRightCursor( int x );
    int getCurrentPixXFromRawX( double x );
    void emitHeadChanged( bool indexOnly = false );
    void setHead( DataQueue::iterator head, int headIndex, bool overwriteRequest = true );
    void setHead( int headIndex );

private:
    QColor bgColor;
    QColor gridColor;
    QColor zeroGridColor;
    QColor strColor;
    QColor cursorColor;
    QColor rightCursorColor;
    QColor frameColor;
    QColor yValueColor;

    int legendOffsetX;
    int legendOffsetY;
    int legendLineWidth;
    int legendLineLength;
    int legendFontSize;

    int defaultFontSize;

    int cursorWidth;

    QList<QColor> colors;
    QList<QString> names;
    QString xName;
    DataQueue dataQueue;

    DataQueue::iterator head;
    DataQueue::iterator cursor;
    DataQueue::iterator rightCursor;

    int headIndex;

    bool validCursor;
    bool validRightCursor;

    bool autoUpdateYMax;
    bool autoUpdateYMin;
    bool autoZeroCenter;

    bool showCursor;
    bool showRightCursor;

    bool showCursorValue;
    bool showHeadValue;
    bool showYGridValue;
    bool showXGridValue;

    bool defaultHeadUpdate;

    bool rightCursorForceBig;

    bool forceRequestedRawX;

    // raw * xScale = pixel
    // xGrid * xScale = pixel

    double xScale;
    double xGrid;
    double yMax;
    double yMin;
    double yZero;

    double requestRawX;

    int queueSize;
    int columnCount;

    int yGridCount;

    QList<ColorFilter> colorFilterList;

signals:
    void moveCursor( QPair<int, QVector<double> > );
    void moveRightCursor( QPair<int, QVector<double> > );
    void headChanged( int index );
    void headChanged( double rawX );
    void queueSizeChanged( int size );
    void rangeChanged( int min, int max );

public slots:
    void clearQueue();
    void enqueueData( const QVector<double> &data, bool updateHead );
    void enqueueData( const QVector<double> &data );
    void setHeadIndex(int value);
    void setHeadFromRawX(double x, const MoveMode mode, bool emitChanged, bool owReq = true );
    void setHeadFromRawXSmall( double x );
    void setHeadFromRawXSmallForce( double x );
    void setDefaultHeadUpdate(bool value);
    void setXScale(double value);
    void setXScale(int value);
    void moveHeadToHead(bool emitSignal , bool indexOnly);

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *);

    // QWidget interface
protected:
    virtual void mouseMoveEvent(QMouseEvent *event);

    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent *event);
};

#endif // WAVEGRAPHWIDGET_H
