#include "QtCalibrationResultTableModel.h"

#include <QDebug>
#include <QMetaObject>
#include <QBrush>
#include <QColor>
#include <QStringList>

namespace ui {

namespace {

QString buildIssuesTooltip(const std::vector<domain::common::CalibrationCellIssue>& issues)
{
    QStringList lines;
    for (const auto& issue : issues) {
        lines.push_back(QStringLiteral("• %1").arg(QString::fromStdString(issue.message)));
    }
    return lines.join('\n');
}

QString buildValidationTooltip(const domain::common::ClibrationResultValidation::Issues* issues)
{
    if (issues == nullptr || issues->empty()) {
        return {};
    }

    QStringList lines;
    for (const auto& issue : *issues) {
        lines.push_back(QStringLiteral("• %1").arg(QString::fromStdString(issue.message)));
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

std::optional<domain::common::CalibrationIssueSeverity> maxSeverityOf(
    const domain::common::ClibrationResultValidation::Issues* issues)
{
    if (issues == nullptr || issues->empty()) {
        return std::nullopt;
    }

    auto maxSeverity = issues->front().severity;
    for (const auto& issue : *issues) {
        if (static_cast<int>(issue.severity) > static_cast<int>(maxSeverity)) {
            maxSeverity = issue.severity;
        }
    }
    return maxSeverity;
}

QString markerForSeverity(domain::common::CalibrationIssueSeverity severity)
{
    using Severity = domain::common::CalibrationIssueSeverity;
    switch (severity) {
        case Severity::Info: return QStringLiteral("ℹ");
        case Severity::Warning: return QStringLiteral("⚠");
        case Severity::Error: return QStringLiteral("⛔");
    }
    return {};
}

QVariant backgroundForValidation(domain::common::CalibrationIssueSeverity severity)
{
    using Severity = domain::common::CalibrationIssueSeverity;
    switch (severity) {
        case Severity::Info: return QBrush(QColor(207, 250, 254));
        case Severity::Warning: return QBrush(QColor(254, 243, 199));
        case Severity::Error: return QBrush(QColor(254, 202, 202));
    }
    return {};
}

QString mergeTooltips(const QString& left, const QString& right)
{
    if (left.isEmpty()) return right;
    if (right.isEmpty()) return left;
    return left + QStringLiteral("\n") + right;
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
        QMetaObject::invokeMethod(this, [this, copy] { applyResult(copy); }, Qt::QueuedConnection);
    });

    current_validation_sub_ = vm_.current_validation.subscribe([this](const auto& e) {
        const auto copy = e.new_value;
        QMetaObject::invokeMethod(this, [this, copy] { applyValidation(copy); }, Qt::QueuedConnection);
    });
}

int QtCalibrationResultTableModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : rows_.size();
}

int QtCalibrationResultTableModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 16;
}

QVariant QtCalibrationResultTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rows_.size()) {
        return {};
    }

    const auto& cell = rows_[index.row()].cells[index.column()];

    if (role == Qt::DisplayRole) {
        return cell.display;
    }
    if (role == Qt::ToolTipRole) {
        return cell.tooltip;
    }
    if (role == Qt::TextAlignmentRole) {
        return QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    if (role == Qt::BackgroundRole && cell.validation_severity) {
        return backgroundForValidation(*cell.validation_severity);
    }
    if (role == Qt::ForegroundRole) {
        return QBrush(Qt::black);
    }
    return {};
}

QVariant QtCalibrationResultTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }
    if (orientation == Qt::Horizontal) {
        return tr("у.%1.%2").arg(section % 2 == 0 ? tr("п") : tr("о")).arg(section / 2 + 1);
    }
    if (orientation == Qt::Vertical && section >= 0 && section < rows_.size()) {
        return rows_[section].label;
    }
    return {};
}

Qt::ItemFlags QtCalibrationResultTableModel::flags(const QModelIndex& index) const
{
    return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
}

void QtCalibrationResultTableModel::applyResult(std::optional<domain::common::CalibrationResult> result)
{
    beginResetModel();
    current_result_ = std::move(result);
    rows_.clear();
    if (current_result_) {
        rebuildRows(*current_result_);
        refreshValidationDecorations();
    }
    endResetModel();
}

void QtCalibrationResultTableModel::applyValidation(std::optional<domain::common::ClibrationResultValidation> validation)
{
    current_validation_ = std::move(validation);
    if (rows_.isEmpty()) {
        return;
    }
    refreshValidationDecorations();
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
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
                domain::common::CalibrationCellKey key{point, domain::common::SourceId{i + 1}, static_cast<domain::common::MotorDirection>(j)};
                Cell uiCell;
                const auto cell = result.cell(key);
                if (cell) {
                    if (cell->angle()) {
                        uiCell.display = QStringLiteral("%1 %2").arg(QString::number(*cell->angle(), 'f', 2), markerForSeverity(maxSeverityOf(cell->issues()).value_or(domain::common::CalibrationIssueSeverity::Info)));
                        if (cell->issues().empty()) {
                            uiCell.display = QString::number(*cell->angle(), 'f', 2);
                        }
                    }
                    uiCell.max_severity = maxSeverityOf(cell->issues());
                    uiCell.marker = uiCell.max_severity ? markerForSeverity(*uiCell.max_severity) : QString();
                    uiCell.tooltip = buildIssuesTooltip(cell->issues());
                }
                row.cells.push_back(std::move(uiCell));
            }
        }
        rows_.push_back(std::move(row));
    }
}

void QtCalibrationResultTableModel::refreshValidationDecorations()
{
    if (!current_result_) {
        return;
    }

    for (int rowIndex = 0; rowIndex < rows_.size(); ++rowIndex) {
        auto& row = rows_[rowIndex];
        const auto& point = current_result_->points()[rowIndex];

        for (int sourceIndex = 0; sourceIndex < 8; ++sourceIndex) {
            for (int directionIndex = 0; directionIndex < 2; ++directionIndex) {
                const int column = sourceIndex * 2 + directionIndex;
                auto& cell = row.cells[column];
                const domain::common::CalibrationCellKey key{point, domain::common::SourceId{sourceIndex + 1}, static_cast<domain::common::MotorDirection>(directionIndex)};

                const auto* issues = current_validation_ ? current_validation_->issuesFor(key) : nullptr;
                cell.validation_severity = maxSeverityOf(issues);
                cell.tooltip = mergeTooltips(buildIssuesTooltip({}), buildValidationTooltip(issues));
                if (const auto resultCell = current_result_->cell(key); resultCell) {
                    cell.tooltip = mergeTooltips(buildIssuesTooltip(resultCell->issues()), buildValidationTooltip(issues));
                    if (resultCell->angle()) {
                        const auto angleText = QString::number(*resultCell->angle(), 'f', 2);
                        cell.marker = cell.max_severity ? markerForSeverity(*cell.max_severity) : QString();
                        cell.display = cell.marker.isEmpty() ? angleText : QStringLiteral("%1 %2").arg(angleText, cell.marker);
                    }
                }
            }
        }
    }
}

} // namespace ui
