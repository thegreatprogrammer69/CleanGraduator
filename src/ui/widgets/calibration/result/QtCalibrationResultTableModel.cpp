#include "QtCalibrationResultTableModel.h"

#include <QApplication>
#include <QMetaObject>
#include <QDebug>
#include <QStringList>
#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QStyle>
#include <cmath>
#include <limits>

namespace ui {

namespace {

QString buildIssuesTooltip(const std::vector<domain::common::CalibrationCellIssue>& issues,
                          const domain::common::CalibrationResultValidation::Issues& validation_issues)
{
    QStringList lines;

    if (!issues.empty()) {
        lines.push_back(QStringLiteral("Проблемы расчёта:"));
        for (const auto& issue : issues) {
            lines.push_back(QStringLiteral("• %1").arg(QString::fromStdString(issue.message)));
        }
    }

    if (!validation_issues.empty()) {
        if (!lines.isEmpty()) {
            lines.push_back(QString());
        }
        lines.push_back(QStringLiteral("Проблемы валидации:"));
        for (const auto& issue : validation_issues) {
            lines.push_back(QStringLiteral("• %1").arg(QString::fromStdString(issue.message)));
        }
    }

    return lines.join('\n');
}

std::optional<domain::common::CalibrationIssueSeverity> maxSeverityOf(
    const std::vector<domain::common::CalibrationCellIssue>& issues)
{
    if (issues.empty()) {
        return std::nullopt;
    }

    auto maxSeverity = issues.front().severity;
    for (const auto& issue : issues) {
        if (static_cast<int>(issue.severity) > static_cast<int>(maxSeverity)) {
            maxSeverity = issue.severity;
        }
    }

    return maxSeverity;
}

std::optional<domain::common::CalibrationIssueSeverity> maxValidationSeverityOf(
    const domain::common::CalibrationResultValidation::Issues& issues)
{
    if (issues.empty()) {
        return std::nullopt;
    }

    auto maxSeverity = issues.front().severity;
    for (const auto& issue : issues) {
        if (static_cast<int>(issue.severity) > static_cast<int>(maxSeverity)) {
            maxSeverity = issue.severity;
        }
    }

    return maxSeverity;
}

QVariant backgroundForSeverity(domain::common::CalibrationIssueSeverity severity)
{
    using Severity = domain::common::CalibrationIssueSeverity;

    switch (severity) {
        case Severity::Info:
            return QBrush(QColor(210, 240, 255));
        case Severity::Warning:
            return QBrush(QColor(255, 236, 179));
        case Severity::Error:
            return QBrush(QColor(255, 205, 210));
    }

    return {};
}

QIcon iconForSeverity(domain::common::CalibrationIssueSeverity severity)
{
    auto* style = QApplication::style();
    using Severity = domain::common::CalibrationIssueSeverity;

    switch (severity) {
        case Severity::Info:
            return style->standardIcon(QStyle::SP_MessageBoxInformation);
        case Severity::Warning:
            return style->standardIcon(QStyle::SP_MessageBoxWarning);
        case Severity::Error:
            return style->standardIcon(QStyle::SP_MessageBoxCritical);
    }

    return {};
}

std::optional<float> angleFor(
    const domain::common::CalibrationResult& result,
    const domain::common::PointId& point,
    const domain::common::SourceId& source,
    domain::common::MotorDirection direction)
{
    const domain::common::CalibrationCellKey key{point, source, direction};
    const auto cell = result.cell(key);
    if (!cell || !cell->angle()) {
        return std::nullopt;
    }

    return cell->angle();
}

QString formatFloat(float value, int precision = 2)
{
    return QString::number(value, 'f', precision);
}

QString formatPercent(float value)
{
    return QStringLiteral("%1%").arg(QString::number(value, 'f', 2));
}

} // namespace

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

    current_validation_sub_ = vm_.current_validation.subscribe([this](const auto& e) {
        const auto copy = e.new_value;
        QMetaObject::invokeMethod(this, [this, copy] {
            try {
                applyValidation(copy);
            } catch (...) {
                qCritical() << "Unknown exception in QtCalibrationResultTableModel::applyValidation";
            }
        }, Qt::QueuedConnection);
    });

    info_snapshot_sub_ = vm_.info_snapshot.subscribe([this](const auto& e) {
        const auto copy = e.new_value;
        QMetaObject::invokeMethod(this, [this, copy] {
            info_snapshot_ = copy;
            if (!current_result_) {
                beginResetModel();
                rows_.clear();
                endResetModel();
                return;
            }

            beginResetModel();
            rebuildRows(*current_result_);
            endResetModel();
        }, Qt::QueuedConnection);
    }, false);
}

int QtCalibrationResultTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return rows_.size();
}

int QtCalibrationResultTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 16;
}

QVariant QtCalibrationResultTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() < 0 || index.row() >= rows_.size()) {
        return {};
    }

    const auto& row = rows_[index.row()];

    if (index.column() < 0 || index.column() >= row.cells.size()) {
        return {};
    }

    const auto& cell = row.cells[index.column()];

    if (role == Qt::DisplayRole) {
        return cell.display;
    }

    if (role == Qt::ToolTipRole) {
        return cell.tooltip;
    }

    if (role == Qt::DecorationRole && cell.max_severity.has_value()) {
        return iconForSeverity(*cell.max_severity);
    }

    if (role == Qt::TextAlignmentRole) {
        return QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    if (role == Qt::BackgroundRole && cell.validation_severity.has_value()) {
        return backgroundForSeverity(*cell.validation_severity);
    }

    if (role == Qt::ForegroundRole && cell.validation_severity.has_value()) {
        return QBrush(Qt::black);
    }

    return {};
}

QVariant QtCalibrationResultTableModel::headerData(
    int section,
    Qt::Orientation orientation,
    int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        return tr("у.%1.%2")
            .arg(section % 2 == 0 ? tr("п") : tr("о"))
            .arg(section / 2 + 1);
    }

    if (orientation == Qt::Vertical) {
        if (section < 0 || section >= rows_.size()) {
            return {};
        }

        return rows_[section].label;
    }

    return {};
}

Qt::ItemFlags QtCalibrationResultTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool QtCalibrationResultTableModel::shouldSpanPairColumns(int row) const
{
    return row >= 0 && row < rows_.size() && rows_[row].span_pair_columns;
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

void QtCalibrationResultTableModel::applyValidation(
    const std::optional<domain::common::CalibrationResultValidation>& validation)
{
    current_validation_ = validation;

    if (!current_result_) {
        return;
    }

    beginResetModel();
    rebuildRows(*current_result_);
    endResetModel();
}

QtCalibrationResultTableModel::Cell QtCalibrationResultTableModel::makePairCell(const QVariant& display) const
{
    Cell cell;
    cell.display = display;
    return cell;
}

int QtCalibrationResultTableModel::columnForSourceDirection(int source_index, domain::common::MotorDirection direction)
{
    return source_index * 2 + (direction == domain::common::MotorDirection::Backward ? 1 : 0);
}

QtCalibrationResultTableModel::Row QtCalibrationResultTableModel::buildOverallAngleRow(
    const domain::common::CalibrationResult& result) const
{
    Row row;
    row.label = QStringLiteral("общ.");
    row.span_pair_columns = true;
    row.cells.resize(columnCount());

    if (result.points().empty()) {
        return row;
    }

    const auto& first_point = result.points().front();
    const auto& last_point = result.points().back();

    for (int source_index = 0; source_index < result.sources().size(); ++source_index) {
        const auto source = result.sources()[source_index];
        const auto first_angle = angleFor(result, first_point, source, domain::common::MotorDirection::Forward);
        const auto last_angle = angleFor(result, last_point, source, domain::common::MotorDirection::Forward);
        if (!first_angle || !last_angle) {
            continue;
        }

        row.cells[columnForSourceDirection(source_index, domain::common::MotorDirection::Forward)] =
            makePairCell(formatFloat(*last_angle - *first_angle));
    }

    return row;
}

QtCalibrationResultTableModel::Row QtCalibrationResultTableModel::buildNonlinearityRow(
    const domain::common::CalibrationResult& result) const
{
    Row row;
    row.label = QStringLiteral("нелин.");
    row.cells.resize(columnCount());

    const auto& points = result.points();
    if (points.size() < 2) {
        return row;
    }

    for (int source_index = 0; source_index < result.sources().size(); ++source_index) {
        const auto source = result.sources()[source_index];
        for (const auto direction : {domain::common::MotorDirection::Forward, domain::common::MotorDirection::Backward}) {
            std::vector<float> angles;
            angles.reserve(points.size());

            for (const auto& point : points) {
                const auto angle = angleFor(result, point, source, direction);
                if (!angle) {
                    angles.clear();
                    break;
                }
                angles.push_back(*angle);
            }

            if (angles.size() < 2) {
                continue;
            }

            const float avr_delta = (angles.back() - angles.front()) / static_cast<float>(angles.size() - 1);
            if (std::abs(avr_delta) < std::numeric_limits<float>::epsilon()) {
                row.cells[columnForSourceDirection(source_index, direction)].display = QStringLiteral("—");
                continue;
            }

            float max_d = 0.0F;
            for (int i = 0; i + 1 < static_cast<int>(angles.size()); ++i) {
                max_d = std::max(max_d, std::abs((angles[i + 1] - angles[i]) - avr_delta));
            }

            row.cells[columnForSourceDirection(source_index, direction)].display = formatPercent((max_d / std::abs(avr_delta)) * 100.0F);
        }
    }

    return row;
}

QtCalibrationResultTableModel::Row QtCalibrationResultTableModel::buildMeasurementCountRow() const
{
    Row row;
    row.label = QStringLiteral("кол-во");
    row.cells.resize(columnCount());

    for (int source_index = 0; source_index < info_snapshot_.source_ids.size(); ++source_index) {
        const auto source = info_snapshot_.source_ids[source_index];
        const auto it = info_snapshot_.counts_by_source.find(source);
        if (it == info_snapshot_.counts_by_source.end()) {
            continue;
        }

        row.cells[columnForSourceDirection(source_index, domain::common::MotorDirection::Forward)].display = it->second.forward;
        row.cells[columnForSourceDirection(source_index, domain::common::MotorDirection::Backward)].display = it->second.backward;
    }

    return row;
}

QtCalibrationResultTableModel::Row QtCalibrationResultTableModel::buildCurrentAngleRow() const
{
    Row row;
    row.label = QStringLiteral("тек. уг.");
    row.span_pair_columns = true;
    row.cells.resize(columnCount());

    for (int source_index = 0; source_index < info_snapshot_.source_ids.size(); ++source_index) {
        const auto source = info_snapshot_.source_ids[source_index];
        const auto it = info_snapshot_.current_angle_by_source.find(source);
        if (it == info_snapshot_.current_angle_by_source.end()) {
            continue;
        }

        row.cells[columnForSourceDirection(source_index, domain::common::MotorDirection::Forward)] =
            makePairCell(formatFloat(it->second));
    }

    return row;
}

void QtCalibrationResultTableModel::rebuildRows(const domain::common::CalibrationResult& result)
{
    rows_.clear();

    for (const auto& point : result.points()) {
        Row row;
        row.label = QString::number(point.pressure, 'f', 2);
        row.cells.reserve(16);

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 2; ++j) {
                domain::common::CalibrationCellKey key;
                key.point_id = point;
                key.source_id = domain::common::SourceId{i + 1};
                key.direction = static_cast<domain::common::MotorDirection>(j);

                Cell uiCell;
                const auto cell = result.cell(key);
                const auto validation_issues = current_validation_
                    ? current_validation_->issuesFor(key)
                    : domain::common::CalibrationResultValidation::Issues{};

                if (cell) {
                    if (cell->angle()) {
                        uiCell.display = QString::number(*cell->angle(), 'f', 2);
                    }

                    const auto& issues = cell->issues();
                    uiCell.tooltip = buildIssuesTooltip(issues, validation_issues);
                    uiCell.max_severity = maxSeverityOf(issues);
                    uiCell.validation_severity = maxValidationSeverityOf(validation_issues);
                } else {
                    uiCell.tooltip = buildIssuesTooltip({}, validation_issues);
                    uiCell.validation_severity = maxValidationSeverityOf(validation_issues);
                }

                row.cells.push_back(std::move(uiCell));
            }
        }

        rows_.push_back(std::move(row));
    }

    rows_.push_back(buildOverallAngleRow(result));
    rows_.push_back(buildNonlinearityRow(result));
    rows_.push_back(buildMeasurementCountRow());
    rows_.push_back(buildCurrentAngleRow());
}

} // namespace ui
