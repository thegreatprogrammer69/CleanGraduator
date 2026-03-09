#include "QtCalibrationSeriesWidget.h"

#include <QAbstractTableModel>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QFontDatabase>
#include <QHeaderView>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QScrollBar>
#include <QTableView>
#include <QtDebug>

#include <algorithm>
#include <utility>
#include <vector>

namespace {

static QString formatNumber(float v)
{
    // Можно поменять точность при желании
    return QString::number(static_cast<double>(v), 'f', 3);
}

} // namespace

namespace ui {

class QtCalibrationSeriesWidget::SeriesTableModel final : public QAbstractTableModel
{
public:
    enum class Mode {
        Pressure,
        Angle
    };

    struct Row {
        float time = 0.0f;
        float value = 0.0f;
        bool in_session = false;
    };

public:
    explicit SeriesTableModel(QObject* parent = nullptr)
        : QAbstractTableModel(parent)
    {
    }

    int rowCount(const QModelIndex& parent = {}) const override
    {
        return parent.isValid() ? 0 : static_cast<int>(rows_.size());
    }

    int columnCount(const QModelIndex& parent = {}) const override
    {
        return parent.isValid() ? 0 : 2;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};

        switch (section) {
            case 0:
                return tr("Время");
            case 1:
                return (mode_ == Mode::Pressure) ? tr("Давление") : tr("Угол");
            default:
                return {};
        }
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid()) return {};
        const int r = index.row();
        const int c = index.column();
        if (r < 0 || r >= rowCount()) return {};

        const auto& row = rows_[static_cast<std::size_t>(r)];

        if (role == Qt::DisplayRole) {
            switch (c) {
                case 0: return formatNumber(row.time);
                case 1: return formatNumber(row.value);
                default: return {};
            }
        }

        if (role == Qt::TextAlignmentRole) {
            return int(Qt::AlignVCenter | Qt::AlignHCenter);
        }

        if (role == Qt::FontRole) {
            if (row.in_session) {
                QFont f;
                f.setBold(true);
                return f;
            }
        }

        return {};
    }

    void setMode(Mode mode)
    {
        if (mode_ == mode) return;
        beginResetModel();
        mode_ = mode;
        endResetModel();
    }

    void clearAll()
    {
        beginResetModel();
        rows_.clear();
        endResetModel();
    }

    void resetToPressure(const std::vector<mvvm::CalibrationSeriesViewModel::PressureEntry>& src)
    {
        beginResetModel();
        mode_ = Mode::Pressure;
        rows_.clear();
        rows_.reserve(src.size());
        for (const auto& e : src) {
            rows_.push_back(Row{e.time, e.pressure, e.in_session});
        }
        endResetModel();
    }

    void resetToAngle(const std::vector<mvvm::CalibrationSeriesViewModel::AngleEntry>& src)
    {
        beginResetModel();
        mode_ = Mode::Angle;
        rows_.clear();
        rows_.reserve(src.size());
        for (const auto& e : src) {
            rows_.push_back(Row{e.time, e.angle, e.in_session});
        }
        endResetModel();
    }

    void appendPressure(const mvvm::CalibrationSeriesViewModel::PressureEntry& e)
    {
        if (mode_ != Mode::Pressure) {
            beginResetModel();
            mode_ = Mode::Pressure;
            rows_.clear();
            endResetModel();
        }

        const int row = rowCount();
        beginInsertRows({}, row, row);
        rows_.push_back(Row{e.time, e.pressure, e.in_session});
        endInsertRows();
    }

    void appendAngle(const mvvm::CalibrationSeriesViewModel::AngleEntry& e)
    {
        if (mode_ != Mode::Angle) {
            beginResetModel();
            mode_ = Mode::Angle;
            rows_.clear();
            endResetModel();
        }

        const int row = rowCount();
        beginInsertRows({}, row, row);
        rows_.push_back(Row{e.time, e.angle, e.in_session});
        endInsertRows();
    }

    std::size_t size() const
    {
        return rows_.size();
    }

private:
    Mode mode_ = Mode::Pressure;
    std::vector<Row> rows_{};
};

QString QtCalibrationSeriesWidget::comboTextForCamera(int source_id_value)
{
    return tr("Камера %1").arg(source_id_value);
}

QtCalibrationSeriesWidget::SelectedSource QtCalibrationSeriesWidget::currentSelection() const
{
    SelectedSource sel{};

    if (!sources_ || sources_->currentIndex() < 0) {
        sel.kind = SelectedKind::Pressure;
        return sel;
    }

    const QVariant kindData = sources_->currentData(Qt::UserRole);
    const QVariant idData   = sources_->currentData(Qt::UserRole + 1);

    const QString kind = kindData.toString();
    if (kind == QStringLiteral("camera")) {
        sel.kind = SelectedKind::Camera;
        sel.camera_id = idData.toInt();
    } else {
        sel.kind = SelectedKind::Pressure;
    }

    return sel;
}

QtCalibrationSeriesWidget::QtCalibrationSeriesWidget(mvvm::CalibrationSeriesViewModel& vm, QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    buildUi();
    reloadSources();
    resetFromSelected();

    current_pressure_sub_ = vm_.current_pressure.subscribe([this](const auto& e) {
        const auto copy = e.new_value;

        QMetaObject::invokeMethod(this, [this, copy] {
            try {
                appendPressure(copy);
            } catch (...) {
                qCritical() << "Unknown exception in appendPressure";
            }
        }, Qt::QueuedConnection);
    });

    current_angle_sub_ = vm_.current_angle.subscribe([this](const auto& e) {
        const auto copy = e.new_value;

        QMetaObject::invokeMethod(this, [this, copy] {
            try {
                appendAngle(copy.first.value, copy.second);
            } catch (...) {
                qCritical() << "Unknown exception in appendAngle";
            }
        }, Qt::QueuedConnection);
    });

    source_ids_sub_ = vm_.source_ids.subscribe([this](const auto& e) {
        const auto copy = e.new_value;

        QMetaObject::invokeMethod(this, [this, copy] {
            try {
                Q_UNUSED(copy);
                rebuildSourceComboPreserveSelection();
            } catch (...) {
                qCritical() << "Unknown exception in rebuildSourceComboPreserveSelection";
            }
        }, Qt::QueuedConnection);
    });
}

QtCalibrationSeriesWidget::~QtCalibrationSeriesWidget() = default;

void QtCalibrationSeriesWidget::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    auto* top = new QHBoxLayout();
    top->setSpacing(8);
    root->addLayout(top);

    auto* srcLbl = new QLabel(tr("Серия:"), this);
    top->addWidget(srcLbl);

    sources_ = new QComboBox(this);
    sources_->setMinimumWidth(220);
    sources_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    top->addWidget(sources_, 0);

    clear_ = new QPushButton(tr("Очистить"), this);
    top->addWidget(clear_);

    follow_ = new QCheckBox(tr("Follow"), this);
    follow_->setChecked(true);
    top->addWidget(follow_);

    top->addStretch(1);

    status_ = new QLabel("0", this);
    status_->setMinimumWidth(60);
    status_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    top->addWidget(status_);

    entries_model_ = new SeriesTableModel(this);

    table_ = new QTableView(this);
    table_->setModel(entries_model_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setAlternatingRowColors(true);
    table_->setWordWrap(false);
    table_->setSortingEnabled(false);

    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->verticalHeader()->setVisible(false);

    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    table_->setFont(mono);

    root->addWidget(table_, 1);

    connect(sources_, QOverload<int>::of(&QComboBox::activated),
            this, &QtCalibrationSeriesWidget::onSourceActivated);

    connect(clear_, &QPushButton::clicked,
            this, &QtCalibrationSeriesWidget::clearView);

    connect(table_->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int v) {
        if (!follow_) return;
        const int max = table_->verticalScrollBar()->maximum();
        const bool atBottom = (v >= max);
        if (!atBottom && follow_->isChecked()) {
            follow_->setChecked(false);
        }
    });
}

void QtCalibrationSeriesWidget::reloadSources()
{
    rebuildSourceComboPreserveSelection();
}

void QtCalibrationSeriesWidget::rebuildSourceComboPreserveSelection()
{
    const auto prevSel = currentSelection();

    std::vector<int> camera_ids;
    camera_ids.reserve(vm_.openedSources().size());

    for (const auto& sid : vm_.openedSources()) {
        camera_ids.push_back(sid.value);
    }

    std::sort(camera_ids.begin(), camera_ids.end());
    camera_ids.erase(std::unique(camera_ids.begin(), camera_ids.end()), camera_ids.end());

    sources_->blockSignals(true);
    sources_->clear();

    // 0: pressure
    sources_->addItem(tr("Датчик давления"));
    sources_->setItemData(0, QStringLiteral("pressure"), Qt::UserRole);
    sources_->setItemData(0, -1, Qt::UserRole + 1);

    for (int camera_id : camera_ids) {
        const int row = sources_->count();
        sources_->addItem(comboTextForCamera(camera_id));
        sources_->setItemData(row, QStringLiteral("camera"), Qt::UserRole);
        sources_->setItemData(row, camera_id, Qt::UserRole + 1);
    }

    int idxToSelect = 0;

    if (prevSel.kind == SelectedKind::Camera) {
        for (int i = 0; i < sources_->count(); ++i) {
            if (sources_->itemData(i, Qt::UserRole).toString() == QStringLiteral("camera") &&
                sources_->itemData(i, Qt::UserRole + 1).toInt() == prevSel.camera_id) {
                idxToSelect = i;
                break;
            }
        }
    }

    sources_->setCurrentIndex(idxToSelect);
    sources_->blockSignals(false);
}

void QtCalibrationSeriesWidget::onSourceActivated(int index)
{
    if (index < 0) return;

    locally_cleared_ = false;
    resetFromSelected();
}

void QtCalibrationSeriesWidget::resetFromSelected()
{
    const auto sel = currentSelection();

    if (sel.kind == SelectedKind::Pressure) {
        entries_model_->resetToPressure(vm_.pressureHistory());
        status_->setText(QString::number(static_cast<qint64>(entries_model_->size())));
        scrollToBottomIfFollow();
        return;
    }

    const auto source_id = domain::common::SourceId{sel.camera_id};
    entries_model_->resetToAngle(vm_.angleHistory(source_id));
    status_->setText(QString::number(static_cast<qint64>(entries_model_->size())));
    scrollToBottomIfFollow();
}

void QtCalibrationSeriesWidget::appendPressure(const mvvm::CalibrationSeriesViewModel::PressureEntry& e)
{
    const auto sel = currentSelection();
    if (sel.kind != SelectedKind::Pressure) {
        return;
    }

    if (locally_cleared_) {
        entries_model_->clearAll();
        locally_cleared_ = false;
    }

    entries_model_->appendPressure(e);
    status_->setText(QString::number(static_cast<qint64>(entries_model_->size())));
    scrollToBottomIfFollow();
}

void QtCalibrationSeriesWidget::appendAngle(int source_id_value,
                                            const mvvm::CalibrationSeriesViewModel::AngleEntry& e)
{
    const auto sel = currentSelection();
    if (sel.kind != SelectedKind::Camera) {
        return;
    }

    if (sel.camera_id != source_id_value) {
        return;
    }

    if (locally_cleared_) {
        entries_model_->clearAll();
        locally_cleared_ = false;
    }

    entries_model_->appendAngle(e);
    status_->setText(QString::number(static_cast<qint64>(entries_model_->size())));
    scrollToBottomIfFollow();
}

void QtCalibrationSeriesWidget::scrollToBottomIfFollow()
{
    if (!follow_ || !follow_->isChecked()) return;
    if (!table_) return;
    if (entries_model_->rowCount() <= 0) return;

    table_->scrollToBottom();
}

void QtCalibrationSeriesWidget::clearView()
{
    entries_model_->clearAll();
    status_->setText("0");
    locally_cleared_ = true;
}

} // namespace ui