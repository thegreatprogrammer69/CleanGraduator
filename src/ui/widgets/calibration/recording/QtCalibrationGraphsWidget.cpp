#include "QtCalibrationGraphsWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMetaObject>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace ui {

namespace {

QString sourceTitle(int source_id)
{
    return QObject::tr("Камера %1").arg(source_id);
}

} // namespace

class QtCalibrationGraphsWidget::PlotWidget final : public QWidget
{
public:
    explicit PlotWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumSize(480, 320);
    }

    void setData(QString title,
                 QString x_label,
                 QString y_label,
                 std::vector<SeriesData> series)
    {
        title_ = std::move(title);
        x_label_ = std::move(x_label);
        y_label_ = std::move(y_label);
        series_ = std::move(series);
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), palette().base());

        constexpr int left = 64;
        constexpr int right = 24;
        constexpr int top = 28;
        constexpr int bottom = 64;

        const QRect plot = rect().adjusted(left, top, -right, -bottom);
        if (plot.width() <= 1 || plot.height() <= 1) {
            return;
        }

        p.setPen(QPen(palette().mid().color(), 1));
        p.drawRect(plot);

        p.setPen(palette().text().color());
        p.drawText(QRect(0, 0, width(), top), Qt::AlignCenter, title_);
        p.drawText(QRect(0, height() - 26, width(), 24), Qt::AlignCenter, x_label_);

        p.save();
        p.translate(16, height() / 2);
        p.rotate(-90);
        p.drawText(QRect(-height() / 2, 0, height(), 24), Qt::AlignCenter, y_label_);
        p.restore();

        if (series_.empty()) {
            p.drawText(plot, Qt::AlignCenter, tr("Нет данных"));
            return;
        }

        double min_x = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();
        bool has_points = false;

        for (const auto& s : series_) {
            for (const auto& pt : s.points) {
                min_x = std::min(min_x, pt.x());
                max_x = std::max(max_x, pt.x());
                min_y = std::min(min_y, pt.y());
                max_y = std::max(max_y, pt.y());
                has_points = true;
            }
        }

        if (!has_points) {
            p.drawText(plot, Qt::AlignCenter, tr("Нет данных"));
            return;
        }

        const double x_range = max_x - min_x;
        if (std::abs(x_range) < 1e-6) {
            min_x -= 1.0;
            max_x += 1.0;
        } else {
            const double x_padding = x_range / 3.0;
            min_x -= x_padding;
            max_x += x_padding;
        }

        const double y_range = max_y - min_y;
        if (std::abs(y_range) < 1e-6) {
            min_y -= 1.0;
            max_y += 1.0;
        } else {
            const double y_padding = y_range / 3.0;
            min_y -= y_padding;
            max_y += y_padding;
        }

        const auto map_to_plot = [&](const QPointF& pt) {
            const double nx = (pt.x() - min_x) / (max_x - min_x);
            const double ny = (pt.y() - min_y) / (max_y - min_y);

            return QPointF(
                plot.left() + nx * plot.width(),
                plot.bottom() - ny * plot.height());
        };

        p.setPen(palette().mid().color());
        p.drawText(QRect(plot.left(), plot.bottom() + 6, 90, 20),
                   Qt::AlignLeft, QString::number(min_x, 'f', 2));
        p.drawText(QRect(plot.right() - 90, plot.bottom() + 6, 90, 20),
                   Qt::AlignRight, QString::number(max_x, 'f', 2));
        p.drawText(QRect(2, plot.top() - 8, left - 8, 20),
                   Qt::AlignRight, QString::number(max_y, 'f', 2));
        p.drawText(QRect(2, plot.bottom() - 8, left - 8, 20),
                   Qt::AlignRight, QString::number(min_y, 'f', 2));

        int legend_y = top + 8;

        for (const auto& s : series_) {
            if (s.points.empty()) {
                continue;
            }

            QPen pen(s.color, 2);
            p.setPen(pen);

            QPainterPath path;
            path.moveTo(map_to_plot(s.points.front()));
            for (std::size_t i = 1; i < s.points.size(); ++i) {
                path.lineTo(map_to_plot(s.points[i]));
            }
            p.drawPath(path);

            const QRect legend_color(width() - 180, legend_y, 12, 12);
            const QRect legend_text(width() - 162, legend_y - 2, 154, 18);
            p.fillRect(legend_color, s.color);
            p.setPen(palette().text().color());
            p.drawText(legend_text, Qt::AlignLeft | Qt::AlignVCenter, s.name);

            legend_y += 18;
        }
    }

private:
    QString title_;
    QString x_label_;
    QString y_label_;
    std::vector<SeriesData> series_;
};

QtCalibrationGraphsWidget::QtCalibrationGraphsWidget(mvvm::CalibrationSeriesViewModel& vm, QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    buildUi();
    rebuildAngleSourcesPreserveSelection();
    refreshGraph();

    current_pressure_sub_ = vm_.current_pressure.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] {
            refreshGraph();
        }, Qt::QueuedConnection);
    });

    current_angle_sub_ = vm_.current_angle.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] {
            refreshGraph();
        }, Qt::QueuedConnection);
    });

    source_ids_sub_ = vm_.source_ids.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] {
            rebuildAngleSourcesPreserveSelection();
            refreshGraph();
        }, Qt::QueuedConnection);
    });
}

QtCalibrationGraphsWidget::~QtCalibrationGraphsWidget() = default;

void QtCalibrationGraphsWidget::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    auto* controls = new QHBoxLayout();
    controls->setSpacing(8);

    controls->addWidget(new QLabel(tr("График:"), this));

    graph_type_ = new QComboBox(this);
    graph_type_->addItem(tr("Давление / время"));
    graph_type_->addItem(tr("Угол / время"));
    graph_type_->addItem(tr("Угол / давление"));
    controls->addWidget(graph_type_, 0);

    controls->addWidget(new QLabel(tr("Источники угла:"), this));

    angle_sources_ = new QListWidget(this);
    angle_sources_->setMaximumHeight(110);
    angle_sources_->setMinimumWidth(200);
    controls->addWidget(angle_sources_, 1);

    root->addLayout(controls);

    details_ = new QLabel(this);
    root->addWidget(details_);

    plot_ = new PlotWidget(this);
    root->addWidget(plot_, 1);

    connect(graph_type_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QtCalibrationGraphsWidget::onGraphTypeChanged);

    connect(angle_sources_, &QListWidget::itemChanged,
            this, &QtCalibrationGraphsWidget::onAngleSourceItemChanged);
}

void QtCalibrationGraphsWidget::onGraphTypeChanged(int)
{
    updateSourcesVisibility();
    refreshGraph();
}

void QtCalibrationGraphsWidget::onAngleSourceItemChanged(QListWidgetItem*)
{
    refreshGraph();
}

void QtCalibrationGraphsWidget::rebuildAngleSourcesPreserveSelection()
{
    std::unordered_map<int, bool> was_selected;

    for (int i = 0; i < angle_sources_->count(); ++i) {
        const auto* item = angle_sources_->item(i);
        if (!item) {
            continue;
        }

        const int source_id = item->data(Qt::UserRole).toInt();
        was_selected[source_id] = item->checkState() == Qt::Checked;
    }

    std::vector<int> source_ids;
    source_ids.reserve(vm_.openedSources().size());
    for (const auto& source : vm_.openedSources()) {
        source_ids.push_back(source.value);
    }

    std::sort(source_ids.begin(), source_ids.end());
    source_ids.erase(std::unique(source_ids.begin(), source_ids.end()), source_ids.end());

    angle_sources_->blockSignals(true);
    angle_sources_->clear();

    for (int source_id : source_ids) {
        auto* item = new QListWidgetItem(sourceTitle(source_id), angle_sources_);
        item->setData(Qt::UserRole, source_id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

        const bool checked = was_selected.count(source_id) == 0 || was_selected[source_id];
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }

    angle_sources_->blockSignals(false);
    updateSourcesVisibility();
}

void QtCalibrationGraphsWidget::refreshGraph()
{
    const auto graph_type = static_cast<GraphType>(graph_type_->currentIndex());
    const auto series = buildSeriesForGraphType(graph_type);

    int points_count = 0;
    for (const auto& s : series) {
        points_count += static_cast<int>(s.points.size());
    }

    details_->setText(tr("Серий: %1, точек: %2").arg(series.size()).arg(points_count));

    plot_->setData(graphTitle(graph_type),
                   xAxisLabel(graph_type),
                   yAxisLabel(graph_type),
                   series);
}

void QtCalibrationGraphsWidget::updateSourcesVisibility()
{
    const auto graph_type = static_cast<GraphType>(graph_type_->currentIndex());
    const bool needs_angle_sources = graph_type != GraphType::PressureVsTime;
    angle_sources_->setVisible(needs_angle_sources);
}

std::vector<int> QtCalibrationGraphsWidget::selectedAngleSources() const
{
    std::vector<int> selected;

    for (int i = 0; i < angle_sources_->count(); ++i) {
        const auto* item = angle_sources_->item(i);
        if (!item || item->checkState() != Qt::Checked) {
            continue;
        }

        selected.push_back(item->data(Qt::UserRole).toInt());
    }

    return selected;
}

std::vector<QtCalibrationGraphsWidget::SeriesData>
QtCalibrationGraphsWidget::buildSeriesForGraphType(GraphType graph_type) const
{
    std::vector<SeriesData> series;

    if (graph_type == GraphType::PressureVsTime) {
        auto points = buildPressureTimePoints();
        series.push_back(SeriesData{tr("Давление"), QColor(0x2E, 0x86, 0xDE), std::move(points)});
        return series;
    }

    for (int source : selectedAngleSources()) {
        const auto source_id = domain::common::SourceId{source};
        auto points = (graph_type == GraphType::AngleVsTime)
            ? buildAngleTimePoints(source_id)
            : buildAnglePressurePoints(source_id);

        series.push_back(SeriesData{sourceTitle(source), colorForSource(source), std::move(points)});
    }

    return series;
}

std::vector<QPointF> QtCalibrationGraphsWidget::buildPressureTimePoints() const
{
    std::vector<QPointF> points;
    const auto& pressure = vm_.pressureHistory();
    points.reserve(pressure.size());

    for (const auto& sample : pressure) {
        points.emplace_back(sample.time, sample.pressure);
    }

    return points;
}

std::vector<QPointF> QtCalibrationGraphsWidget::buildAngleTimePoints(domain::common::SourceId source_id) const
{
    std::vector<QPointF> points;
    const auto& angle = vm_.angleHistory(source_id);
    points.reserve(angle.size());

    for (const auto& sample : angle) {
        points.emplace_back(sample.time, sample.angle);
    }

    return points;
}

std::vector<QPointF> QtCalibrationGraphsWidget::buildAnglePressurePoints(domain::common::SourceId source_id) const
{
    std::vector<QPointF> points;

    const auto& angle = vm_.angleHistory(source_id);
    const auto& pressure = vm_.pressureHistory();
    points.reserve(angle.size());

    for (const auto& angle_sample : angle) {
        const auto pressure_value = pressureAtTime(pressure, angle_sample.time);
        if (!pressure_value.has_value()) {
            continue;
        }

        points.emplace_back(*pressure_value, angle_sample.angle);
    }

    return points;
}

QColor QtCalibrationGraphsWidget::colorForSource(int source_id)
{
    static const std::vector<QColor> palette = {
        QColor(0xE7, 0x4C, 0x3C),
        QColor(0x27, 0xAE, 0x60),
        QColor(0x8E, 0x44, 0xAD),
        QColor(0xD3, 0x54, 0x00),
        QColor(0x16, 0xA0, 0x85),
        QColor(0x2C, 0x3E, 0x50),
    };

    const std::size_t idx = static_cast<std::size_t>(std::abs(source_id)) % palette.size();
    return palette[idx];
}

QString QtCalibrationGraphsWidget::graphTitle(GraphType type)
{
    switch (type) {
        case GraphType::PressureVsTime: return tr("Давление относительно времени");
        case GraphType::AngleVsTime: return tr("Угол относительно времени");
        case GraphType::AngleVsPressure: return tr("Угол относительно давления");
    }

    return {};
}

QString QtCalibrationGraphsWidget::xAxisLabel(GraphType type)
{
    switch (type) {
        case GraphType::PressureVsTime:
        case GraphType::AngleVsTime:
            return tr("Время");
        case GraphType::AngleVsPressure:
            return tr("Давление");
    }

    return {};
}

QString QtCalibrationGraphsWidget::yAxisLabel(GraphType type)
{
    switch (type) {
        case GraphType::PressureVsTime:
            return tr("Давление");
        case GraphType::AngleVsTime:
        case GraphType::AngleVsPressure:
            return tr("Угол");
    }

    return {};
}

std::optional<double> QtCalibrationGraphsWidget::pressureAtTime(
    const std::vector<mvvm::CalibrationSeriesViewModel::PressureEntry>& pressure,
    float time)
{
    if (pressure.empty()) {
        return std::nullopt;
    }

    if (time <= pressure.front().time) {
        return pressure.front().pressure;
    }

    if (time >= pressure.back().time) {
        return pressure.back().pressure;
    }

    for (std::size_t i = 1; i < pressure.size(); ++i) {
        const auto& left = pressure[i - 1];
        const auto& right = pressure[i];

        if (time < left.time || time > right.time) {
            continue;
        }

        const float dt = right.time - left.time;
        if (std::abs(dt) < 1e-6f) {
            return left.pressure;
        }

        const double alpha = static_cast<double>(time - left.time) / static_cast<double>(dt);
        return left.pressure + alpha * static_cast<double>(right.pressure - left.pressure);
    }

    return std::nullopt;
}

} // namespace ui
