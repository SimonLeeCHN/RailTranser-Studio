#ifndef RFIDGRAPHICSITEM_H
#define RFIDGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPoint>
#include <QRectF>
#include <QPainter>
#include <QPainterPath>

class RfidGraphicsItem : public QObject,public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

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
