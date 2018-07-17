#ifndef PATHWAYFRAME_H
#define PATHWAYFRAME_H

#include <QObject>
#include <QWidget>
#include <QFrame>

class PathwayFrame : public QFrame
{
    Q_OBJECT
public:
    explicit PathwayFrame(QWidget* parent = 0);

public:
    void loadPathwayMap(QString filename);

private:
    bool m_bPathwayLoaded = false;

protected:
    void paintEvent(QPaintEvent* event);

};

#endif // PATHWAYFRAME_H
