#include "QtVideoSourceGridWidget.h"

#include <QGridLayout>
#include <QFrame>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QtMath>

#include "ui/widgets/video/QtVideoSourceWidget.h"

#include "viewmodels/video/VideoSourceGridViewModel.h"

namespace {

// Плитка с видео внутри (аналог makeCameraTile из старого кода)
static QFrame* makeTile(mvvm::VideoSourceViewModel& model, QWidget* parent)
{
    auto* tile = new QFrame(parent);
    tile->setObjectName("cameraTile");
    tile->setAttribute(Qt::WA_StyledBackground, true);

    auto* video = new ui::widgets::QtVideoSourceWidget(model, tile);
    // TODO: привяжите video к vm (если нужно)
    // video->setViewModel(vm);  // пример

    auto* l = new QVBoxLayout(tile);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    l->addWidget(video);

    return tile;
}

} // namespace

namespace ui::widgets {

QtVideoSourceGridWidget::QtVideoSourceGridWidget(mvvm::VideoSourceGridViewModel& model, QWidget* parent)
    : QWidget(parent)
    , m_model(model)
{
    setObjectName("videoSourceGrid");

    m_rows   = m_model.rows();
    m_cols   = m_model.cols();
    m_aspect = m_model.aspectRatioWH();

    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setSpacing(2);

    // По вертикали хотим тянуться, по горизонтали — фиксироваться вычисленной шириной
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    rebuildFromModel();
    recalcSizes();
}

void QtVideoSourceGridWidget::setAspectRatio(double aspectRatioWH)
{
    if (aspectRatioWH <= 0.0)
        return;

    m_aspect = aspectRatioWH;
    recalcSizes();
}

void QtVideoSourceGridWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    recalcSizes();
}

void QtVideoSourceGridWidget::rebuildFromModel()
{
    // очищаем старые плитки (если перестраиваем)
    for (auto* t : m_tiles)
        t->deleteLater();
    m_tiles.clear();

    // Можно было бы также чистить layout item'ы, но т.к. виджеты удаляем,
    // layout сам «освободится» от них. Если хотите жёстко — можно пройтись и удалить items.

    const auto& video_slots = m_model.videoSlots();
    m_tiles.reserve(static_cast<int>(video_slots.size()));

    for (const auto& s : video_slots) {
        if (s.row < 0 || s.col < 0)
            continue;

        auto* tile = makeTile(s.vm, this);

        m_tiles.push_back(tile);
        m_grid->addWidget(tile, s.row, s.col);
    }
}

void QtVideoSourceGridWidget::recalcSizes()
{
    if (m_inRecalc) return;
    m_inRecalc = true;

    const int spacing = m_grid ? m_grid->spacing() : 0;
    const int h = height();

    // Высота одной плитки из доступной высоты (m_rows рядов)
    const int totalSpacingH = spacing * qMax(0, m_rows - 1);
    int tileH = (h - totalSpacingH) / qMax(1, m_rows);
    if (tileH < 20) tileH = 20;

    // Ширина плитки по aspect ratio (W/H)
    int tileW = qRound(tileH * m_aspect);
    if (tileW < 20) tileW = 20;

    // Применяем размеры всем созданным плиткам
    for (auto* tile : m_tiles) {
        if (tile)
            tile->setFixedSize(tileW, tileH);
    }

    // Фиксируем ширину всего виджета сетки (m_cols колонок + spacing)
    const int totalSpacingW = spacing * qMax(0, m_cols - 1);
    const int gridW = m_cols * tileW + totalSpacingW;

    if (width() != gridW)
        setFixedWidth(gridW);

    m_inRecalc = false;
}

} // namespace ui::widgets
