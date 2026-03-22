#include "QtCalibrationResultTableModel.h"

#include <QApplication>
#include <QMetaObject>
#include <QDebug>
#include <QStringList>
#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QStyle>

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
}

} // namespace ui
