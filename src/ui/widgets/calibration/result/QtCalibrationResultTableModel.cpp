#include "QtCalibrationResultTableModel.h"

#include <QApplication>
#include <QMetaObject>
#include <algorithm>
#include <QStringList>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QIcon>
#include <QStyle>

namespace ui {

namespace {

const QColor kResultRowBackground{248, 248, 248};
const QColor kInfoRowBackground{236, 236, 236};
const QColor kHighNonlinearityBackground{255, 205, 210};
const QColor kCenterDeviationErrorBackground{255, 205, 210};
constexpr float kHighNonlinearityThresholdPercent = 10.0F;

bool isAngleSpanIssueKind(domain::common::CalibrationValidationIssueKind kind)
{
    using Kind = domain::common::CalibrationValidationIssueKind;
    return kind == Kind::AngleSpanTooHigh || kind == Kind::AngleSpanTooLow;
}

bool isMeasurementIssueKind(domain::common::CalibrationValidationIssueKind kind)
{
    return !isAngleSpanIssueKind(kind);
}

QString buildIssuesTooltip(
    const std::vector<domain::common::CalibrationCellIssue>& issues,
    const domain::common::CalibrationResultValidation::Issues* validation_issues,
    bool (*validation_predicate)(domain::common::CalibrationValidationIssueKind) = nullptr)
{
    QStringList lines;

    if (!issues.empty()) {
        lines.push_back(QStringLiteral("Проблемы расчёта:"));
        for (const auto& issue : issues) {
            lines.push_back(QStringLiteral("• %1").arg(QString::fromStdString(issue.message)));
        }
    }

    if (validation_issues) {
        bool has_validation = false;
        for (const auto& issue : *validation_issues) {
            if (!validation_predicate || validation_predicate(issue.kind)) {
                has_validation = true;
                break;
            }
        }

        if (has_validation) {
            if (!lines.isEmpty()) {
                lines.push_back(QString());
            }
            lines.push_back(QStringLiteral("Проблемы валидации:"));
            for (const auto& issue : *validation_issues) {
                if (validation_predicate && !validation_predicate(issue.kind)) {
                    continue;
                }
                lines.push_back(QStringLiteral("• %1").arg(QString::fromStdString(issue.message)));
            }
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

std::optional<domain::common::CalibrationIssueSeverity> maxSeverityOf(
    const domain::common::CalibrationResultValidation::Issues& issues,
    bool (*validation_predicate)(domain::common::CalibrationValidationIssueKind) = nullptr)
{
    std::optional<domain::common::CalibrationIssueSeverity> max_severity;
    for (const auto& issue : issues) {
        if (validation_predicate && !validation_predicate(issue.kind)) {
            continue;
        }
        if (!max_severity.has_value()
            || static_cast<int>(issue.severity) > static_cast<int>(*max_severity)) {
            max_severity = issue.severity;
        }
    }
    return max_severity;
}

std::optional<domain::common::CalibrationIssueSeverity> maxSeverityOf(
    std::optional<domain::common::CalibrationIssueSeverity> lhs,
    std::optional<domain::common::CalibrationIssueSeverity> rhs)
{
    if (!lhs.has_value()) {
        return rhs;
    }
    if (!rhs.has_value()) {
        return lhs;
    }
    return static_cast<int>(*lhs) >= static_cast<int>(*rhs) ? lhs : rhs;
}

int issueKindPriority(domain::common::CalibrationValidationIssueKind kind)
{
    using Kind = domain::common::CalibrationValidationIssueKind;
    switch (kind) {
        case Kind::AngleSpanTooHigh:
            return 3;
        case Kind::HysteresisExceeded:
            return 2;
        case Kind::AngleSpanTooLow:
            return 1;
    }
    return 0;
}

std::optional<domain::common::CalibrationValidationIssueKind> maxValidationKindOf(
    const domain::common::CalibrationResultValidation::Issues& issues,
    bool (*validation_predicate)(domain::common::CalibrationValidationIssueKind) = nullptr)
{
    std::optional<domain::common::CalibrationValidationIssueKind> selected_kind;
    for (const auto& issue : issues) {
        if (validation_predicate && !validation_predicate(issue.kind)) {
            continue;
        }
        if (!selected_kind.has_value()
            || issueKindPriority(issue.kind) > issueKindPriority(*selected_kind)) {
            selected_kind = issue.kind;
        }
    }

    return selected_kind;
}

QColor validationColor(domain::common::CalibrationValidationIssueKind kind)
{
    using Kind = domain::common::CalibrationValidationIssueKind;

    switch (kind) {
        case Kind::HysteresisExceeded:
            return QColor(255, 236, 179);
        case Kind::AngleSpanTooHigh:
            return QColor(255, 205, 210);
        case Kind::AngleSpanTooLow:
            return QColor(210, 240, 255);
    }

    return {};
}

QColor blendColors(const QColor& base, const QColor& overlay, qreal overlay_alpha = 0.35)
{
    const qreal clamped_alpha = std::clamp(overlay_alpha, 0.0, 1.0);
    const qreal base_alpha = 1.0 - clamped_alpha;
    return QColor(
        static_cast<int>(base.red() * base_alpha + overlay.red() * clamped_alpha),
        static_cast<int>(base.green() * base_alpha + overlay.green() * clamped_alpha),
        static_cast<int>(base.blue() * base_alpha + overlay.blue() * clamped_alpha));
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

const QIcon& iconForSeverityCached(domain::common::CalibrationIssueSeverity severity)
{
    static const std::array<QIcon, 3> icons{
        iconForSeverity(domain::common::CalibrationIssueSeverity::Info),
        iconForSeverity(domain::common::CalibrationIssueSeverity::Warning),
        iconForSeverity(domain::common::CalibrationIssueSeverity::Error)
    };

    return icons[static_cast<std::size_t>(severity)];
}

QString displayFloat(float value, int precision = 2, const QString& suffix = {})
{
    return QStringLiteral("%1%2")
        .arg(QString::number(value, 'f', precision), suffix);
}

} // namespace

QtCalibrationResultTableModel::QtCalibrationResultTableModel(
    QtCalibrationResultTableModelDeps deps,
    QObject* parent)
    : QAbstractTableModel(parent)
    , vm_(deps.vm)
{
    for (int section = 0; section < kColumnCount; ++section) {
        horizontal_headers_[section] = tr("у.%1.%2")
            .arg(section % 2 == 0 ? tr("п") : tr("о"))
            .arg(section / 2 + 1);
    }

    current_result_sub_ = vm_.current_result.subscribe([this](const auto& e) {
        const auto copy = e.new_value;

        QMetaObject::invokeMethod(this, [this, copy] {
            applyResult(copy);
        }, Qt::QueuedConnection);
    });

    current_validation_sub_ = vm_.current_validation.subscribe([this](const auto& e) {
        const auto copy = e.new_value;
        QMetaObject::invokeMethod(this, [this, copy] {
            applyValidation(copy);
        }, Qt::QueuedConnection);
    });

    current_info_sub_ = vm_.current_info.subscribe([this](const auto& e) {
        const auto copy = e.new_value;
        QMetaObject::invokeMethod(this, [this, copy] {
            applyInfo(copy);
        }, Qt::QueuedConnection);
    });

    selected_gauge_pressures_sub_ = vm_.selected_gauge_pressures.subscribe([this](const auto& e) {
        const auto copy = e.new_value;
        QMetaObject::invokeMethod(this, [this, copy] {
            if (selected_gauge_pressures_ == copy) {
                return;
            }
            beginResetModel();
            selected_gauge_pressures_ = copy;
            refreshRowsWithReset();
            endResetModel();
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

    return kColumnCount;
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

    if (index.column() < 0 || index.column() >= kColumnCount) {
        return {};
    }

    const auto& cell = row.cells[static_cast<std::size_t>(index.column())];

    if (role == Qt::DisplayRole) {
        return cell.display;
    }

    if (role == Qt::ToolTipRole) {
        return cell.tooltip;
    }

    if (role == Qt::DecorationRole && cell.max_severity.has_value()) {
        return iconForSeverityCached(*cell.max_severity);
    }

    if (role == Qt::TextAlignmentRole) {
        return static_cast<int>(Qt::AlignCenter);
    }

    if (role == Qt::BackgroundRole) {
        QColor background = isInfoRow(index.row()) ? kInfoRowBackground : kResultRowBackground;
        if (row.kind == RowKind::Nonlinearity
            && cell.nonlinearity_percent.has_value()
            && *cell.nonlinearity_percent > kHighNonlinearityThresholdPercent) {
            background = blendColors(background, kHighNonlinearityBackground, 0.65);
        }
        if (row.kind == RowKind::CenterDeviation
            && current_info_.centered_mark_enabled
            && cell.center_deviation_deg.has_value()
            && *cell.center_deviation_deg > current_info_.max_center_deviation_deg) {
            background = blendColors(background, kCenterDeviationErrorBackground, 0.7);
        }
        if (cell.validation_kind.has_value()) {
            background = blendColors(background, validationColor(*cell.validation_kind));
        }
        return QBrush(background);
    }

    if (role == Qt::ForegroundRole && cell.validation_kind.has_value()) {
        return QBrush(Qt::black);
    }

    return {};
}

QVariant QtCalibrationResultTableModel::headerData(
    int section,
    Qt::Orientation orientation,
    int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role != Qt::DisplayRole || section < 0 || section >= kColumnCount) {
            return {};
        }

        return horizontal_headers_[static_cast<std::size_t>(section)];
    }

    if (orientation == Qt::Vertical) {
        if (section < 0 || section >= rows_.size()) {
            return {};
        }

        if (role == Qt::DisplayRole) {
            return rows_[section].label;
        }

        if (role == Qt::FontRole && isInfoRow(section)) {
            QFont font;
            font.setBold(true);
            return font;
        }
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

bool QtCalibrationResultTableModel::isPairMergedRow(int row) const
{
    if (row < 0 || row >= rows_.size()) {
        return false;
    }

    return rows_[row].kind == RowKind::TotalAngle || rows_[row].kind == RowKind::CurrentAngle;
}

bool QtCalibrationResultTableModel::isInfoRow(int row) const
{
    if (row < 0 || row >= rows_.size()) {
        return false;
    }

    return rows_[row].kind != RowKind::Measurement;
}

void QtCalibrationResultTableModel::applyResult(
    std::optional<domain::common::CalibrationResult> result)
{
    beginResetModel();

    current_result_ = std::move(result);
    refreshRowsWithReset();

    endResetModel();
}

void QtCalibrationResultTableModel::applyValidation(
    std::optional<domain::common::CalibrationResultValidation> validation)
{
    if (!current_result_) {
        current_validation_ = std::move(validation);
        return;
    }

    if (current_validation_ == validation) {
        return;
    }

    current_validation_ = std::move(validation);
    updateRowsInPlace();
}

void QtCalibrationResultTableModel::applyInfo(const mvvm::CalibrationResultInfo& info)
{
    if (current_info_ == info) {
        return;
    }

    current_info_ = info;

    if (!current_result_) {
        return;
    }

    updateRowsInPlace();
}

void QtCalibrationResultTableModel::rebuildRows()
{
    rows_.clear();

    const bool has_result = current_result_.has_value();
    const auto measurement_row_count = has_result
        ? current_result_->points().size()
        : selected_gauge_pressures_.size();
    rows_.reserve(static_cast<qsizetype>(measurement_row_count) + 5);

    static const std::vector<domain::common::CalibrationCellIssue> empty_calc_issues;

    const auto* result = has_result ? &(*current_result_) : nullptr;

    for (std::size_t row_idx = 0; row_idx < measurement_row_count; ++row_idx) {
        Row row;
        row.kind = RowKind::Measurement;
        row.label = has_result
            ? QString::number(current_result_->points()[row_idx].pressure, 'f', 2)
            : QString::number(selected_gauge_pressures_[row_idx], 'f', 2);

        if (!has_result) {
            rows_.push_back(std::move(row));
            continue;
        }

        const auto& point = current_result_->points()[row_idx];

        for (int i = 0; i < kSourceCount; ++i) {
            for (int j = 0; j < 2; ++j) {
                domain::common::CalibrationCellKey key;
                key.point_id = point;
                key.source_id = domain::common::SourceId{i + 1};
                key.direction = static_cast<domain::common::MotorDirection>(j);

                Cell& ui_cell = row.cells[static_cast<std::size_t>(i * 2 + j)];
                const auto cell = result->cell(key);
                const auto* validation_issues = current_validation_
                    ? &current_validation_->issuesFor(key)
                    : nullptr;

                if (cell) {
                    if (cell->angle()) {
                        ui_cell.display = QString::number(*cell->angle(), 'f', 2);
                    }

                    const auto& issues = cell->issues();
                    ui_cell.tooltip = buildIssuesTooltip(issues, validation_issues, isMeasurementIssueKind);
                    ui_cell.max_severity = maxSeverityOf(
                        maxSeverityOf(issues),
                        validation_issues
                            ? maxSeverityOf(*validation_issues, isMeasurementIssueKind)
                            : std::nullopt);
                    ui_cell.validation_kind = validation_issues
                        ? maxValidationKindOf(*validation_issues, isMeasurementIssueKind)
                        : std::nullopt;
                } else {
                    ui_cell.tooltip = buildIssuesTooltip(empty_calc_issues, validation_issues, isMeasurementIssueKind);
                    ui_cell.max_severity = validation_issues
                        ? maxSeverityOf(*validation_issues, isMeasurementIssueKind)
                        : std::nullopt;
                    ui_cell.validation_kind = validation_issues
                        ? maxValidationKindOf(*validation_issues, isMeasurementIssueKind)
                        : std::nullopt;
                }
            }
        }

        rows_.push_back(std::move(row));
    }

    appendInfoRows();
}

void QtCalibrationResultTableModel::appendInfoRows()
{
    auto makeRow = [](const QString& label, RowKind kind) {
        Row row;
        row.label = label;
        row.kind = kind;
        return row;
    };

    Row total_row = makeRow(tr("общ."), RowKind::TotalAngle);
    Row nonlinearity_row = makeRow(tr("нелин."), RowKind::Nonlinearity);
    Row center_deviation_row = makeRow(tr("центр. откл."), RowKind::CenterDeviation);
    Row count_row = makeRow(tr("кол-во"), RowKind::MeasurementCount);
    Row current_angle_row = makeRow(tr("тек. уг."), RowKind::CurrentAngle);

    for (int i = 0; i < kSourceCount; ++i) {
        const auto source_id = domain::common::SourceId{i + 1};
        const int forward_col = i * 2;
        const int backward_col = forward_col + 1;

        if (const auto it = current_info_.total_angles.find(source_id); it != current_info_.total_angles.end()) {
            total_row.cells[static_cast<std::size_t>(forward_col)].display = displayFloat(it->second);
        }

        if (current_result_ && current_validation_) {
            for (const auto& point : current_result_->points()) {
                const domain::common::CalibrationCellKey key{
                    point,
                    source_id,
                    domain::common::MotorDirection::Forward
                };
                const auto& issues = current_validation_->issuesFor(key);
                const auto issue_kind = maxValidationKindOf(issues, isAngleSpanIssueKind);
                if (issue_kind.has_value()) {
                    total_row.cells[static_cast<std::size_t>(forward_col)].validation_kind = issue_kind;
                    total_row.cells[static_cast<std::size_t>(forward_col)].max_severity = maxSeverityOf(issues, isAngleSpanIssueKind);
                    total_row.cells[static_cast<std::size_t>(forward_col)].tooltip = buildIssuesTooltip({}, &issues, isAngleSpanIssueKind);
                    break;
                }
            }
        }

        if (const auto source_it = current_info_.nonlinearities.find(source_id); source_it != current_info_.nonlinearities.end()) {
            if (const auto dir_it = source_it->second.find(domain::common::MotorDirection::Forward); dir_it != source_it->second.end()) {
                nonlinearity_row.cells[static_cast<std::size_t>(forward_col)].display = displayFloat(dir_it->second, 2, QStringLiteral("%"));
                nonlinearity_row.cells[static_cast<std::size_t>(forward_col)].nonlinearity_percent = dir_it->second;
                if (dir_it->second > kHighNonlinearityThresholdPercent) {
                    nonlinearity_row.cells[static_cast<std::size_t>(forward_col)].max_severity = domain::common::CalibrationIssueSeverity::Error;
                    nonlinearity_row.cells[static_cast<std::size_t>(forward_col)].tooltip = tr("Ошибка: нелинейность выше 10%%");
                }
            }
            if (const auto dir_it = source_it->second.find(domain::common::MotorDirection::Backward); dir_it != source_it->second.end()) {
                nonlinearity_row.cells[static_cast<std::size_t>(backward_col)].display = displayFloat(dir_it->second, 2, QStringLiteral("%"));
                nonlinearity_row.cells[static_cast<std::size_t>(backward_col)].nonlinearity_percent = dir_it->second;
                if (dir_it->second > kHighNonlinearityThresholdPercent) {
                    nonlinearity_row.cells[static_cast<std::size_t>(backward_col)].max_severity = domain::common::CalibrationIssueSeverity::Error;
                    nonlinearity_row.cells[static_cast<std::size_t>(backward_col)].tooltip = tr("Ошибка: нелинейность выше 10%%");
                }
            }
        }

        if (current_info_.centered_mark_enabled) {
            if (const auto source_it = current_info_.center_deviations_deg.find(source_id);
                source_it != current_info_.center_deviations_deg.end()) {
                if (const auto dir_it = source_it->second.find(domain::common::MotorDirection::Forward); dir_it != source_it->second.end()) {
                    center_deviation_row.cells[static_cast<std::size_t>(forward_col)].display = displayFloat(dir_it->second, 2, QStringLiteral("°"));
                    center_deviation_row.cells[static_cast<std::size_t>(forward_col)].center_deviation_deg = dir_it->second;
                    if (dir_it->second > current_info_.max_center_deviation_deg) {
                        center_deviation_row.cells[forward_col].max_severity = domain::common::CalibrationIssueSeverity::Error;
                        center_deviation_row.cells[forward_col].tooltip = tr("Ошибка: отклонение центрированной метки выше заданного предела");
                    }
                }
                if (const auto dir_it = source_it->second.find(domain::common::MotorDirection::Backward); dir_it != source_it->second.end()) {
                    center_deviation_row.cells[static_cast<std::size_t>(backward_col)].display = displayFloat(dir_it->second, 2, QStringLiteral("°"));
                    center_deviation_row.cells[static_cast<std::size_t>(backward_col)].center_deviation_deg = dir_it->second;
                    if (dir_it->second > current_info_.max_center_deviation_deg) {
                        center_deviation_row.cells[backward_col].max_severity = domain::common::CalibrationIssueSeverity::Error;
                        center_deviation_row.cells[backward_col].tooltip = tr("Ошибка: отклонение центрированной метки выше заданного предела");
                    }
                }
            }
        }

        if (const auto source_it = current_info_.measurement_counts.find(source_id); source_it != current_info_.measurement_counts.end()) {
            if (const auto dir_it = source_it->second.find(domain::common::MotorDirection::Forward); dir_it != source_it->second.end()) {
                count_row.cells[static_cast<std::size_t>(forward_col)].display = dir_it->second;
            }
            if (const auto dir_it = source_it->second.find(domain::common::MotorDirection::Backward); dir_it != source_it->second.end()) {
                count_row.cells[static_cast<std::size_t>(backward_col)].display = dir_it->second;
            }
        }

        if (const auto it = current_info_.current_angles.find(source_id); it != current_info_.current_angles.end()) {
            current_angle_row.cells[static_cast<std::size_t>(forward_col)].display = displayFloat(it->second);
        }
    }

    rows_.push_back(std::move(total_row));
    rows_.push_back(std::move(nonlinearity_row));
    rows_.push_back(std::move(center_deviation_row));
    rows_.push_back(std::move(count_row));
    rows_.push_back(std::move(current_angle_row));
}

void QtCalibrationResultTableModel::refreshRowsWithReset()
{
    rows_.clear();
    if (current_result_ || !selected_gauge_pressures_.empty()) {
        rebuildRows();
    }
}

void QtCalibrationResultTableModel::updateRowsInPlace()
{
    rebuildRows();

    if (!rows_.isEmpty()) {
        emit dataChanged(index(0, 0), index(rows_.size() - 1, kColumnCount - 1));
    }
}

} // namespace ui
