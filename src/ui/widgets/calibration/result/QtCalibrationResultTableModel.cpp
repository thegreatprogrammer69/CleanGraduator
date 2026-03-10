#include "QtCalibrationResultTableModel.h"

#include <QMetaObject>
#include <QDebug>

namespace ui {

QtCalibrationResultTableModel::QtCalibrationResultTableModel(
    QtCalibrationResultTableModelDeps deps,
    QObject* parent)
    : QAbstractTableModel(parent)
    , vm_(deps.vm)
{
    current_result_sub_ = vm_.current_result.subscribe([this](const auto& e) {
        const auto copy = e.new_value;

        QMetaObject::invokeMethod(this, [this, copy] {
            try {
                applyResult(copy);
            } catch (...) {
                qCritical() << "Unknown exception in QtCalibrationResultTableModel::applyResult";
            }
        }, Qt::QueuedConnection);
    });
}

int QtCalibrationResultTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return rows_.size();
}

int QtCalibrationResultTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return 16;
}

    QVariant QtCalibrationResultTableModel::data(const QModelIndex& index, int role) const {

    if (!index.isValid())
        return {};

    if (index.row() >= rows_.size())
        return {};

    const auto& row = rows_[index.row()];

    if (role == Qt::DisplayRole)
        return row.values[index.column()];

    if (role == Qt::TextAlignmentRole)
        return QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter);

    return {};
}

QVariant QtCalibrationResultTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        return QStringLiteral("у.%1.%2")
            .arg(section % 2 == 0 ? QStringLiteral("п") : QStringLiteral("о"))
            .arg(section / 2 + 1);
    }


    if (orientation == Qt::Vertical) {

        if (section >= rows_.size())
            return {};

        return rows_[section].label;
    }

    return {};
}

Qt::ItemFlags QtCalibrationResultTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void QtCalibrationResultTableModel::applyResult(
    std::optional<domain::common::CalibrationResult> result)
{
    beginResetModel();

    current_result_ = std::move(result);
    rows_.clear();

    if (current_result_.has_value()) {
        rebuildRows(*current_result_);
    }

    endResetModel();
}

void QtCalibrationResultTableModel::rebuildRows(const domain::common::CalibrationResult& result)
{
    rows_.clear();

    for (const auto& point : result.points()) {
        Row row;
        row.label = QString::number(point.pressure, 'f', 2);
        row.values.reserve(16);

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 2; ++j) {
                domain::common::CalibrationCellKey key;
                key.point_id = point;
                key.source_id = domain::common::SourceId{i + 1};
                key.direction = static_cast<domain::common::MotorDirection>(j);
                const auto cell = result.cell(key);
                if (cell && cell->angle()) {
                    row.values.push_back(QString::number(*cell->angle(), 'f', 2));
                }
                else {
                    row.values.push_back({});
                }
            }
        }

        rows_.push_back(std::move(row));
    }
}

} // namespace ui