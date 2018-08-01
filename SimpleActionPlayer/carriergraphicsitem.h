#ifndef CARRIERGRAPHICSITEM_H
#define CARRIERGRAPHICSITEM_H

#include <QGraphicsItem>

class CarrierGraphicsItem : public QObject,public QGraphicsItem
{
    Q_OBJECT
public:
    CarrierGraphicsItem();
    CarrierGraphicsItem(QPoint point,int status,int number);
    ~CarrierGraphicsItem();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void setGraphicsPoint(QPoint point);
    void setNumber(int number);
    void setStatus(int status);

protected:
    void advance(int step) override;

private:
    int m_iNumber;              //自身编号
    int m_iStatus;              //自身状态编码
    QPoint m_qpGraphicsPoint;   //在界面上的位置
};

#endif // CARRIERGRAPHICSITEM_H
