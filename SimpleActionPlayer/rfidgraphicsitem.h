#ifndef RFIDGRAPHICSITEM_H
#define RFIDGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPoint>
#include <QRectF>
#include <QPainter>
#include <QPainterPath>

#define RFIDGI_WIDTH                    40
#define RFIDGI_HEIGHT                   40
#define RFIDGI_X                        -20
#define RFIDGI_Y                        -20
#define RFIDGI_BACKGROUND_COLOR         QColor(68, 69, 73)
#define RFIDGI_PEN_COLOR                QColor(255,255,255)
#define RFIDGI_PEN_SIZE                 20

class RfidGraphicsItem : public QObject,public QGraphicsItem
{
    Q_OBJECT
public:
    RfidGraphicsItem();
    RfidGraphicsItem(QPoint point,int number);
    ~RfidGraphicsItem();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;


    void setGraphicsPoint(QPoint point);
    void setNumber(int number);
    QPoint getGraphicsPoint() {return m_qpGraphicsPoint;}
    int getNumber() {return m_iNumber;}

private:
    int m_iNumber;                  //自身编号
    QPoint m_qpGraphicsPoint;       //在界面上的位置
};

#endif // RFIDGRAPHICSITEM_H
