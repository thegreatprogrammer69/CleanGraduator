#include "QtCalibrationResultTableModel.h"

#include <QApplication>
#include <QMetaObject>
#include <QDebug>
#include <QStringList>
#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QStyle>
#include <algorithm>
#include <cmath>
#include <limits>

namespace ui {

namespace {

using domain::common::CalibrationCellKey;
using domain::common::CalibrationIssueSeverity;
using domain::common::CalibrationResult;
using domain::common::CalibrationResultValidation;
using domain::common::MotorDirection;
using domain::common::PointId;
using domain::common::SourceId;

QString buildIssuesTooltip(const std::vector<domain::common::CalibrationCellIssue>& issues,
                          const CalibrationResultValidation::Issues& validation_issues)
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

std::optional<CalibrationIssueSeverity> maxSeverityOf(
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

std::optional<CalibrationIssueSeverity> maxValidationSeverityOf(
    const CalibrationResultValidation::Issues& issues)
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

QVariant backgroundForSeverity(CalibrationIssueSeverity severity)
{
    switch (severity) {
        case CalibrationIssueSeverity::Info:
            return QBrush(QColor(210, 240, 255));
        case CalibrationIssueSeverity::Warning:
            return QBrush(QColor(255, 236, 179));
        case CalibrationIssueSeverity::Error:
            return QBrush(QColor(255, 205, 210));
    }

    return {};
}

QIcon iconForSeverity(CalibrationIssueSeverity severity)
{
    auto* style = QApplication::style();

    switch (severity) {
        case CalibrationIssueSeverity::Info:
            return style->standardIcon(QStyle::SP_MessageBoxInformation);
        case CalibrationIssueSeverity::Warning:
            return style->standardIcon(QStyle::SP_MessageBoxWarning);
        case CalibrationIssueSeverity::Error:
            return style->standardIcon(QStyle::SP_MessageBoxCritical);
    }

    return {};
}

std::optional<float> angleFor(const CalibrationResult& result, SourceId source_id, MotorDirection direction, PointId point)
{
    CalibrationCellKey key;
    key.source_id = source_id;
    key.direction = direction;
    key.point_id = point;

    const auto cell = result.cell(key);
    if (!cell || !cell->angle()) {
        return std::nullopt;
    }

    return *cell->angle();
}

QString formatFloat(std::optional<float> value, int precision = 2, const QString& suffix = {})
{
    if (!value) {
        return {};
    }

    return QStringLiteral("%1%2")
        .arg(QString::number(*value, 'f', precision), suffix);
}

std::optional<float> totalAngle(const CalibrationResult& result, SourceId source_id)
{
    const auto& points = result.points();
    if (points.size() < 2) {
        return std::nullopt;
    }

    const auto first = angleFor(result, source_id, MotorDirection::Forward, points.front());
    const auto last = angleFor(result, source_id, MotorDirection::Forward, points.back());
    if (!first || !last) {
        return std::nullopt;
    }

    return *last - *first;
}

std::optional<float> nonlinearity(const CalibrationResult& result, SourceId source_id, MotorDirection direction)
{
    const auto& points = result.points();
    if (points.size() < 2) {
        return std::nullopt;
    }

    std::vector<float> angles;
    angles.reserve(points.size());
    for (const auto& point : points) {
        const auto angle = angleFor(result, source_id, direction, point);
        if (!angle) {
            return std::nullopt;
        }
        angles.push_back(*angle);
    }

    const float avrDelta = (angles.back() - angles.front()) / static_cast<float>(angles.size() - 1);
    if (std::fabs(avrDelta) <= std::numeric_limits<float>::epsilon()) {
        return std::nullopt;
    }

    float maxD = 0.0f;
    for (size_t i = 0; i + 1 < angles.size(); ++i) {
        const float delta = angles[i + 1] - angles[i];
        maxD = std::max(maxD, std::fabs(delta - avrDelta));
    }

    return (maxD / std::fabs(avrDelta)) * 100.0f;
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

    supplemental_revision_sub_ = vm_.supplemental_revision.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] {
            try {
                notifySupplementalChanged();
            } catch (...) {
                qCritical() << "Unknown exception in QtCalibrationResultTableModel::notifySupplementalChanged";
            }
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

bool QtCalibrationResultTableModel::shouldSpanPair(int row) const
{
    if (row < 0 || row >= rows_.size()) {
        return false;
    }

    return rows_[row].kind == Row::Kind::TotalAngle
        || rows_[row].kind == Row::Kind::CurrentAngle;
}

void QtCalibrationResultTableModel::applyResult(
    std::optional<CalibrationResult> result)
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
    const std::optional<CalibrationResultValidation>& validation)
{
    current_validation_ = validation;

    if (!current_result_) {
        return;
    }

    beginResetModel();
    rebuildRows(*current_result_);
    endResetModel();
}

void QtCalibrationResultTableModel::notifySupplementalChanged()
{
    if (!current_result_) {
        return;
    }

    beginResetModel();
    rebuildRows(*current_result_);
    endResetModel();
}

void QtCalibrationResultTableModel::rebuildRows(const CalibrationResult& result)
{
    rows_.clear();

    for (const auto& point : result.points()) {
        Row row;
        row.label = QString::number(point.pressure, 'f', 2);
        row.cells.reserve(16);

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 2; ++j) {
                CalibrationCellKey key;
                key.point_id = point;
                key.source_id = SourceId{i + 1};
                key.direction = static_cast<MotorDirection>(j);

                Cell uiCell;
                const auto cell = result.cell(key);
                const auto validation_issues = current_validation_
                    ? current_validation_->issuesFor(key)
                    : CalibrationResultValidation::Issues{};

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

    Row totalRow;
    totalRow.kind = Row::Kind::TotalAngle;
    totalRow.label = QStringLiteral("общ.");
    totalRow.cells.resize(16);
    for (int source = 0; source < 8; ++source) {
        totalRow.cells[source * 2].display = formatFloat(totalAngle(result, SourceId{source + 1}));
    }
    rows_.push_back(std::move(totalRow));

    Row nonlinearityRow;
    nonlinearityRow.kind = Row::Kind::Nonlinearity;
    nonlinearityRow.label = QStringLiteral("нелин.");
    nonlinearityRow.cells.resize(16);
    for (int source = 0; source < 8; ++source) {
        nonlinearityRow.cells[source * 2].display = formatFloat(nonlinearity(result, SourceId{source + 1}, MotorDirection::Forward), 2, QStringLiteral("%"));
        nonlinearityRow.cells[source * 2 + 1].display = formatFloat(nonlinearity(result, SourceId{source + 1}, MotorDirection::Backward), 2, QStringLiteral("%"));
    }
    rows_.push_back(std::move(nonlinearityRow));

    Row countRow;
    countRow.kind = Row::Kind::Count;
    countRow.label = QStringLiteral("кол-во");
    countRow.cells.resize(16);
    for (int source = 0; source < 8; ++source) {
        countRow.cells[source * 2].display = vm_.angleMeasurementCount(SourceId{source + 1}, MotorDirection::Forward);
        countRow.cells[source * 2 + 1].display = vm_.angleMeasurementCount(SourceId{source + 1}, MotorDirection::Backward);
    }
    rows_.push_back(std::move(countRow));

    Row currentAngleRow;
    currentAngleRow.kind = Row::Kind::CurrentAngle;
    currentAngleRow.label = QStringLiteral("тек. уг.");
    currentAngleRow.cells.resize(16);
    for (int source = 0; source < 8; ++source) {
        currentAngleRow.cells[source * 2].display = formatFloat(vm_.currentAngle(SourceId{source + 1}));
    }
    rows_.push_back(std::move(currentAngleRow));
}

} // namespace ui
