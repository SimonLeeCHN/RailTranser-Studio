#ifndef PATHWAYGV_H
#define PATHWAYGV_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include "rfidgraphicsitem.h"
#include "carriergraphicsitem.h"
#include <QUrl>

#define SCENE_BACKGROUND_COLOR  QColor(50,50,50)

class PathwayGV : public QGraphicsView
{
    Q_OBJECT

public:
    PathwayGV(QWidget* parent = 0);
    ~PathwayGV();

    void initPathwayRfid(QList<QString> rfidList);
    void initGraphicCarrier(QList<QString> carrierList);

public slots:
    void onUpdateGraphicCarrier(int number,int status,int pos);

private:
    QGraphicsScene* m_pScene = NULL;
    QGraphicsSimpleTextItem* m_pBackgroundTextItem = NULL;
    QGraphicsPathItem* m_pPathwayPath = NULL;

    QList<RfidGraphicsItem*> m_listRfid;
    QList<CarrierGraphicsItem*> m_listGraphicCarrier;

    QPoint transPosnumToGraphicPoint(int pos);

protected:
    void wheelEvent(QWheelEvent* event);
};

#endif // PATHWAYGV_H
