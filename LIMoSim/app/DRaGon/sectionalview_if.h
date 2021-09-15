#ifndef SECTIONALVIEW_IF_H
#define SECTIONALVIEW_IF_H

#include <QObject>
#include "LIMoSim/world/vector3d.h"
#include "ui/uimanager.h"
#include "sectionalview.h"

namespace LIMoSim
{
class SectionalView_if : public QObject
{
    Q_OBJECT
private:
    bool m_rx = false;
    bool m_tx = false;

    SectionalView * m_sv = nullptr;

public:
    SectionalView_if();

public slots:
    void onMouseEvent(const QVector3D &_world, int _type);

signals:
    void rxChanged(Vector3d _rx);
    void txChanged(Vector3d _tx);
};
}

#endif // SECTIONALVIEW_IF_H
