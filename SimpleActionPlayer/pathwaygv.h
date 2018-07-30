#ifndef PATHWAYGV_H
#define PATHWAYGV_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include "rfidgraphicsitem.h"
#include <QUrl>

#define SCENE_BACKGROUND_COLOR  QColor(50,50,50)

class PathwayGV : public QGraphicsView
{
    Q_OBJECT

public:
    PathwayGV(QWidget* parent = 0);
    ~PathwayGV();

    void initPathwayRfid(QList<QString> rfidList);

private:
    QGraphicsScene* m_pScene = NULL;
    QGraphicsSimpleTextItem* m_pBackgroundTextItem = NULL;

protected:
    void wheelEvent(QWheelEvent* event);
};

#endif // PATHWAYGV_H
