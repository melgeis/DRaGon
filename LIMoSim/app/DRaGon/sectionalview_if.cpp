#include "sectionalview_if.h"

namespace LIMoSim
{
SectionalView_if::SectionalView_if()
{
    m_sv = new SectionalView(Vector3d(0,0,0), Vector3d(0,0,0));

    UiManager *ui = UiManager::getInstance();
    OpenGL::OpenGLWindow* window = ui->getWindow();

    connect(window, SIGNAL(mouseEvent(QVector3D,int)), this, SLOT(onMouseEvent(QVector3D,int)));
    connect(this, SIGNAL(rxChanged(Vector3d)), m_sv, SLOT(onRxChanged(Vector3d)));
    connect(this, SIGNAL(txChanged(Vector3d)), m_sv, SLOT(onTxChanged(Vector3d)));
}

void SectionalView_if::onMouseEvent(const QVector3D &_world, int _type)
{
    if (_type == MOUSE_TYPE::PRESS_LEFT) {
        Vector3d vec;
        vec.x = _world.x();
        vec.y = _world.y();
        vec.z = _world.z() + 1.5;

        m_rx = true;
        emit rxChanged(vec);

        if (m_rx && m_tx) {
            m_sv->run();
            m_sv->plotROI();
        }

    }
    else if (_type == MOUSE_TYPE::PRESS_RIGHT) {
        Vector3d vec;
        vec.x = _world.x();
        vec.y = _world.y();
        vec.z = _world.z() + 4;

        m_tx = true;
        emit txChanged(vec);

        if (m_rx && m_tx) {
            m_sv->run();
            m_sv->plotROI();
        }
    }
}
}
