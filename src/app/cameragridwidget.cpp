#include "cameragridwidget.h"

#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QResizeEvent>
#include <QtMath>

#include "ui/widgets/video/VideoOpenGLWidget.h"

static QFrame* makeCameraTile(ui::presenters::VideoStreamPresenter& presenter, QWidget *parent)
{
    auto *tile = new QFrame(parent);
    tile->setObjectName("cameraTile");
    tile->setAttribute(Qt::WA_StyledBackground, true);

    auto *label = new ui::widgets::VideoOpenGLWidget(presenter);

    auto *l = new QVBoxLayout(tile);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(label);

    return tile;
}

CameraGridWidget::CameraGridWidget(ui::presenters::VideoStreamPresenter& presenter, int rows, int cols, double aspectRatioWH, QWidget *parent)
    : QWidget(parent),
      m_rows(rows),
      m_cols(cols),
      m_aspect(aspectRatioWH)
{
    setObjectName("cameraGrid");

    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setSpacing(2); // расстояние 2px между камерами

    m_tiles.reserve(m_rows * m_cols);

    int idx = 0;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            auto *tile = makeCameraTile(presenter, this);
            m_tiles.push_back(tile);
            m_grid->addWidget(tile, r, c);
        }
    }

    // По вертикали хотим тянуться, по горизонтали — фиксироваться вычисленной шириной
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    recalcSizes();
}

void CameraGridWidget::setAspectRatio(double aspectRatioWH)
{
    if (aspectRatioWH <= 0.0)
        return;
    m_aspect = aspectRatioWH;
    recalcSizes();
}

void CameraGridWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    recalcSizes();
}

void CameraGridWidget::recalcSizes()
{
    if (m_inRecalc) return;
    m_inRecalc = true;

    const int spacing = m_grid->spacing();
    const int h = height();

    // Высота одной камеры из доступной высоты 4 рядов
    const int totalSpacingH = spacing * (m_rows - 1);
    int camH = (h - totalSpacingH) / m_rows;
    if (camH < 20) camH = 20;

    // Ширина камеры по aspect ratio (W/H)
    int camW = qRound(camH * m_aspect);
    if (camW < 20) camW = 20;

    // Применяем размеры всем камерам
    for (auto *tile : m_tiles) {
        tile->setFixedSize(camW, camH);
    }

    // Фиксируем ширину всего виджета сетки (2 колонки + spacing)
    const int totalSpacingW = spacing * (m_cols - 1);
    const int gridW = m_cols * camW + totalSpacingW;

    if (width() != gridW) {
        setFixedWidth(gridW);
    }

    m_inRecalc = false;
}
