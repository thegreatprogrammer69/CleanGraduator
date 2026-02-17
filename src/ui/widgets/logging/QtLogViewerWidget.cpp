#include "QtLogViewerWidget.h"

#include <QAbstractTableModel>
#include <QBoxLayout>
#include <QScrollBar>
#include <QFontDatabase>
#include <QtDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMetaObject>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTableView>

#include "application/ports/logging/ILogSource.h"
#include "viewmodels/logging/LogViewerViewModel.h"

#include <chrono>
#include <vector>

namespace {

    static QString formatTimestamp(const domain::common::Timestamp& ts)
    {
        using namespace std::chrono;

        const long total_seconds = ts.asSeconds();

        const qint64 hours   = total_seconds / 3600;
        const qint64 minutes = (total_seconds % 3600) / 60;
        const qint64 seconds = total_seconds % 60;

        return QString("%1:%2:%3")
            .arg(hours,   2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }

static QString formatLevel(const application::models::LogLevel& lvl) {
    switch (lvl) {
        case application::models::LogLevel::Error:
            return QStringLiteral("ERROR");
        case application::models::LogLevel::Warn:
            return QStringLiteral("WARN");
        case application::models::LogLevel::Info:
            return QStringLiteral("INFO");
        default: ;
    }
}

} // namespace

namespace ui {

class QtLogViewerWidget::LogEntriesModel final : public QAbstractTableModel {
public:
    explicit LogEntriesModel(QObject* parent = nullptr)
        : QAbstractTableModel(parent) {}

    int rowCount(const QModelIndex& parent = {}) const override {
        return parent.isValid() ? 0 : static_cast<int>(entries_.size());
    }

    int columnCount(const QModelIndex& parent = {}) const override {
        return parent.isValid() ? 0 : 3;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
        switch (section) {
            case 0: return tr("Время");
            case 1: return tr("Уровень");
            case 2: return tr("Сообщение");
            default: return {};
        }
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid()) return {};
        const int r = index.row();
        const int c = index.column();
        if (r < 0 || r >= rowCount()) return {};

        const auto& e = entries_[static_cast<std::size_t>(r)];

        // display
        if (role == Qt::DisplayRole) {
            switch (c) {
                case 0: return formatTimestamp(e.timestamp);
                case 1: return formatLevel(e.level);
                case 2: return QString::fromStdString(e.message);
                default: return {};
            }
        }

        // unified string for filtering/search
        if (role == Qt::UserRole) {
            return QString("%1 | %2 | %3")
                .arg(formatTimestamp(e.timestamp))
                .arg(formatLevel(e.level))
                .arg(QString::fromStdString(e.message));
        }

        // align time/level center-ish, message left
        if (role == Qt::TextAlignmentRole) {
            if (c == 0 || c == 1) return int(Qt::AlignVCenter | Qt::AlignHCenter);
            return int(Qt::AlignVCenter | Qt::AlignLeft);
        }

        // color by level
        if (role == Qt::ForegroundRole) {
            switch (e.level) {
                case application::models::LogLevel::Error: return QBrush(QColor("#B91C1C")); // red-700
                case application::models::LogLevel::Warn:  return QBrush(QColor("#B45309")); // amber-700
                case application::models::LogLevel::Info:  return QBrush(QColor("#111827")); // gray-900
                default: return {};
            }
        }

        // optional: make ERROR/WARN bold
        if (role == Qt::FontRole && (c == 1)) {
            QFont f;
            if (e.level != application::models::LogLevel::Info) {
                f.setBold(true);
                return f;
            }
        }

        return {};
    }

    void clearAll() {
        beginResetModel();
        entries_.clear();
        endResetModel();
    }

    void resetTo(const std::vector<application::models::LogEntry>& src) {
        beginResetModel();
        entries_.assign(src.begin(), src.end());
        endResetModel();
    }

    void appendOne(application::models::LogEntry e) {
        const int row = rowCount();
        beginInsertRows({}, row, row);
        entries_.push_back(std::move(e));
        endInsertRows();
    }

    std::size_t size() const { return entries_.size(); }

private:
    std::vector<application::models::LogEntry> entries_{};
};


QtLogViewerWidget::QtLogViewerWidget(mvvm::LogViewerViewModel& vm, QWidget* parent)
    : QWidget(parent), vm_(vm)
{
    buildUi();
    reloadSources();

    // --- event-driven: subscribe to new logs ---
    // Ожидаемый API:
    //   Observable<T>::subscribe(void* who, std::function<void(const T&)> cb)
    //   Observable<T>::unsubscribe(void* who)
    //
    // Если у тебя токен - см. блок внизу ответа.
    new_log_sub = vm_.new_log.subscribe([this](auto e) {

        auto copy = e.new_value;   // <-- КОПИЯ ЗДЕСЬ

        QMetaObject::invokeMethod(this, [this, copy] {
            try {
                appendEntry(copy);
            } catch (...) {
                qCritical() << "Unknown exception in appendEntry";
            }
        }, Qt::QueuedConnection);
    });

    // (опционально) если хочешь реагировать на смену log_source извне (не из комбобокса):
    log_source_sub = vm_.log_source.subscribe([this](auto src) {
        const QString name = src.new_value ? QString::fromStdString(src.new_value->name) : QString();
        QMetaObject::invokeMethod(this, [this, name] {
            try {
                if (name != current_source_name_) {
                    current_source_name_ = name;
                    resetFromSelected();
                }
            } catch (...) {
                qCritical() << "Unknown exception in appendEntry";
            }
        }, Qt::QueuedConnection);
    });
}

QtLogViewerWidget::~QtLogViewerWidget()
{
}

void QtLogViewerWidget::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    // --- Top bar ---
    auto* top = new QHBoxLayout();
    top->setSpacing(8);
    root->addLayout(top);

    auto* srcLbl = new QLabel(tr("Источник:"), this);
    top->addWidget(srcLbl);

    sources_ = new QComboBox(this);
    sources_->setMinimumWidth(220);
    sources_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    top->addWidget(sources_, 0);

    reload_ = new QPushButton(tr("Обновить"), this);
    top->addWidget(reload_);

    top->addSpacing(12);

    auto* filterLbl = new QLabel(tr("Фильтр:"), this);
    top->addWidget(filterLbl);

    filter_ = new QLineEdit(this);
    filter_->setClearButtonEnabled(true);
    filter_->setPlaceholderText(tr("поиск по времени/уровню/сообщению…"));
    filter_->setMinimumWidth(260);
    top->addWidget(filter_, 1);

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

    // --- model/proxy ---
    entries_model_ = new LogEntriesModel(this);

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setSourceModel(entries_model_);
    proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy_->setFilterKeyColumn(-1);
    proxy_->setFilterRole(Qt::UserRole);

    // --- table ---
    table_ = new QTableView(this);
    table_->setModel(proxy_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setAlternatingRowColors(true);
    table_->setWordWrap(false);
    table_->setSortingEnabled(false);

    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table_->verticalHeader()->setVisible(false);

    // моноширинный шрифт
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    table_->setFont(mono);

    root->addWidget(table_, 1);

    connect(reload_, &QPushButton::clicked, this, &QtLogViewerWidget::reloadSources);
    connect(clear_,  &QPushButton::clicked, this, &QtLogViewerWidget::clearView);
    connect(filter_, &QLineEdit::textChanged, this, &QtLogViewerWidget::onFilterTextChanged);

    connect(sources_, QOverload<int>::of(&QComboBox::activated),
            this, &QtLogViewerWidget::onSourceActivated);

    // бонус: если юзер скроллит вверх — follow отключаем автоматически
    connect(table_->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int v) {
        if (!follow_) return;
        const int max = table_->verticalScrollBar()->maximum();
        const bool atBottom = (v >= max);
        if (!atBottom && follow_->isChecked()) {
            follow_->setChecked(false);
        }
    });
}


void QtLogViewerWidget::reloadSources()
{
    const QString prev = sources_->currentText();

    auto names = vm_.loadNames();
    std::sort(names.begin(), names.end());

    sources_->blockSignals(true);
    sources_->clear();
    for (const auto& n : names)
        sources_->addItem(QString::fromStdString(n));
    sources_->blockSignals(false);

    if (!prev.isEmpty()) {
        const int idx = sources_->findText(prev);
        if (idx >= 0) sources_->setCurrentIndex(idx);
    }

    if (sources_->currentIndex() < 0 && sources_->count() > 0)
        sources_->setCurrentIndex(0);

    if (sources_->count() > 0)
        onSourceActivated(sources_->currentIndex());
    else {
        current_source_name_.clear();
        entries_model_->clearAll();
        status_->setText("0");
    }
}

void QtLogViewerWidget::onSourceActivated(int index)
{
    if (index < 0) return;

    const QString name = sources_->itemText(index);
    if (name.isEmpty()) return;

    // Switch source via VM
    vm_.selectLogSource(name.toStdString());

    current_source_name_ = name;
    resetFromSelected();
}

void QtLogViewerWidget::resetFromSelected()
{
    const auto opt = vm_.log_source.get_copy();
    if (!opt || !opt->source) {
        entries_model_->clearAll();
        status_->setText("0");
        return;
    }

    // IMPORTANT: history() must be safe to read concurrently
    const auto& hist = opt->source->history();
    entries_model_->resetTo(hist);
    status_->setText(QString::number(static_cast<qint64>(hist.size())));

    scrollToBottomIfFollow();
}

void QtLogViewerWidget::appendEntry(const application::models::LogEntry& e)
{
    // “простая” семантика: добавляем любую пришедшую запись.
    // Если хочешь строго, чтобы добавлялось только для current_source_name_,
    // нужно чтобы LogEntry нес в себе source name/id, или VM фильтровал сам.

    entries_model_->appendOne(e);
    status_->setText(QString::number(static_cast<qint64>(entries_model_->size())));

    scrollToBottomIfFollow();
}

    void QtLogViewerWidget::scrollToBottomIfFollow()
{
    if (!follow_->isChecked()) return;
    if (!filter_->text().isEmpty()) return;           // при активном фильтре — не мешаем
    if (proxy_->rowCount() <= 0) return;

    table_->scrollToBottom();
}


void QtLogViewerWidget::clearView()
{
    entries_model_->clearAll();
    status_->setText("0");
}

void QtLogViewerWidget::onFilterTextChanged(const QString& text)
{
    proxy_->setFilterFixedString(text);
}

} // namespace ui
