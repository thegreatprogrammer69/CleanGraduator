#include "QtCalibrationResultTableModel.h"

#include <QMetaObject>
#include <QStringList>
#include <QBrush>
#include <QColor>

namespace ui {

namespace {

QString buildCellIssuesTooltip(const std::vector<domain::common::CalibrationCellIssue>& issues)
{
    if (issues.empty()) {
        return {};
    }

    QStringList lines;
    lines.push_back(QStringLiteral("Проблемы расчёта:"));

    for (const auto& issue : issues) {
        lines.push_back(QStringLiteral("• %1").arg(QString::fromStdString(issue.message)));
    }

    return lines.join('\n');
}

QString buildValidationTooltip(const domain::common::ClibrationResultValidation::Issues& issues)
{
    QStringList lines;
    lines.push_back(QStringLiteral("Проблемы валидации:"));

    for (const auto& issue : issues) {
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

QString iconForSeverity(domain::common::CalibrationIssueSeverity severity)
{
    using Severity = domain::common::CalibrationIssueSeverity;

    switch (severity) {
        case Severity::Info:
            return QStringLiteral("ℹ");
        case Severity::Warning:
            return QStringLiteral("⚠");
        case Severity::Error:
            return QStringLiteral("⛔");
    }

    return {};
}

QVariant backgroundForValidationIssue(const domain::common::ClibrationResultValidation::Issues& issues)
{
    using Type = domain::common::ClibrationResultValidationIssue::Type;

    bool has_hysteresis = false;
    bool has_span = false;
    for (const auto& issue : issues) {
        has_hysteresis = has_hysteresis || issue.type == Type::HysteresisExceeded;
        has_span = has_span || issue.type == Type::ForwardAngleSpanOutOfRange;
    }

    if (has_hysteresis && has_span) {
        return QBrush(QColor(216, 180, 254));
    }
    if (has_hysteresis) {
        return QBrush(QColor(255, 224, 178));
    }
    if (has_span) {
        return QBrush(QColor(187, 222, 251));
    }

    return {};
}

QString mergeTooltips(const QString& first, const QString& second)
{
    if (first.isEmpty()) {
        return second;
    }
    if (second.isEmpty()) {
        return first;
    }
    return first + QStringLiteral("\n\n") + second;
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
    if (role == Qt::TextAlignmentRole) {
        return QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    if (role == Qt::BackgroundRole && cell.validation_issues.has_value()) {
        return backgroundForValidationIssue(*cell.validation_issues);
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
    return index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable) : Qt::NoItemFlags;
}

void QtCalibrationResultTableModel::applyResult(std::optional<domain::common::CalibrationResult> result)
{
    beginResetModel();
    current_result_ = std::move(result);
    rows_.clear();
    if (current_result_) {
        rebuildRows(*current_result_);
        rebuildValidationMarkup();
    }
    endResetModel();
}

void QtCalibrationResultTableModel::applyValidation(std::optional<domain::common::ClibrationResultValidation> validation)
{
    beginResetModel();
    current_validation_ = std::move(validation);
    rebuildValidationMarkup();
    endResetModel();
}

void QtCalibrationResultTableModel::rebuildRows(const domain::common::CalibrationResult& result)
{
    rows_.clear();

    for (const auto& point : result.points()) {
        Row row;
        row.label = QString::number(point.pressure, 'f', 2);
        row.cells.reserve(16);

        for (int source_index = 0; source_index < 8; ++source_index) {
            for (int direction_index = 0; direction_index < 2; ++direction_index) {
                domain::common::CalibrationCellKey key;
                key.point_id = point;
                key.source_id = domain::common::SourceId{source_index + 1};
                key.direction = static_cast<domain::common::MotorDirection>(direction_index);

                Cell uiCell;
                const auto cell = result.cell(key);
                if (cell) {
                    QString text;
                    if (cell->angle()) {
                        text = QString::number(*cell->angle(), 'f', 2);
                    }

                    const auto severity = maxSeverityOf(cell->issues());
                    if (severity.has_value()) {
                        if (!text.isEmpty()) {
                            text += QStringLiteral(" ");
                        }
                        text += iconForSeverity(*severity);
                    }

                    uiCell.display = text;
                    uiCell.base_tooltip = buildCellIssuesTooltip(cell->issues());
                    uiCell.tooltip = uiCell.base_tooltip;
                    uiCell.max_severity = severity;
                }

                row.cells.push_back(std::move(uiCell));
            }
        }

        rows_.push_back(std::move(row));
    }
}

void QtCalibrationResultTableModel::rebuildValidationMarkup()
{
    if (!current_result_) {
        rows_.clear();
        return;
    }

    for (int row_index = 0; row_index < rows_.size(); ++row_index) {
        auto& row = rows_[row_index];
        for (int column_index = 0; column_index < row.cells.size(); ++column_index) {
            auto& cell = row.cells[column_index];
            cell.validation_issues.reset();
            cell.tooltip = cell.base_tooltip;

            if (!current_validation_) {
                continue;
            }

            const auto point = current_result_->points().at(row_index);
            const auto source_id = domain::common::SourceId{column_index / 2 + 1};
            const auto direction = static_cast<domain::common::MotorDirection>(column_index % 2);
            const domain::common::CalibrationCellKey key{point, source_id, direction};
            const auto issues = current_validation_->issues(key);
            if (!issues) {
                continue;
            }

            cell.validation_issues = *issues;
            cell.tooltip = mergeTooltips(cell.tooltip, buildValidationTooltip(*issues));
        }
    }
}

} // namespace ui
