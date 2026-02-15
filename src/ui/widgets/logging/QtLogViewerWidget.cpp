#include "QtLogViewerWidget.h"

#include <QAbstractTableModel>
#include <QBoxLayout>
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

#include "application/ports/outbound/logging/ILogSource.h"
#include "viewmodels/logging/LogViewerViewModel.h"

#include <chrono>
#include <vector>

namespace {

    static QString formatTimestamp(const domain::common::Timestamp& ts)
    {
        using namespace std::chrono;

        const auto total_ms = duration_cast<milliseconds>(ts.toDuration()).count();
        const auto total_seconds = total_ms / 1000;

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

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return static_cast<int>(entries_.size());
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return 3; // time, level, message
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
        switch (section) {
            case 0: return tr("Время");
            case 1: return tr("Уровень");
            case 2: return tr("Сообщение");
            default: return {};
        }
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) return {};
        const auto& e = entries_[static_cast<std::size_t>(index.row())];

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case 0: return formatTimestamp(e.timestamp);
                case 1: return formatLevel(e.level);
                case 2: return QString::fromStdString(e.message);
                default: return {};
            }
        }

        if (role == Qt::UserRole) {
            return QString("%1 | %2 | %3")
                .arg(formatTimestamp(e.timestamp))
                .arg(formatLevel(e.level))
                .arg(QString::fromStdString(e.message));
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

    void appendOne(const application::models::LogEntry& e) {
        const int row = rowCount();
        beginInsertRows(QModelIndex(), row, row);
        entries_.push_back(e);
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

    auto* top = new QHBoxLayout();
    root->addLayout(top);

    top->addWidget(new QLabel(tr("Источник:"), this));

    sources_ = new QComboBox(this);
    sources_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    top->addWidget(sources_, 0);

    reload_ = new QPushButton(tr("Обновить"), this);
    top->addWidget(reload_);

    top->addSpacing(12);
    top->addWidget(new QLabel(tr("Фильтр:"), this));

    filter_ = new QLineEdit(this);
    filter_->setPlaceholderText(tr("введите для фильтрации…"));
    top->addWidget(filter_, 1);

    clear_ = new QPushButton(tr("Очистить окно"), this);
    top->addWidget(clear_);

    follow_ = new QCheckBox(tr("Следовать"), this);
    follow_->setChecked(true);
    top->addWidget(follow_);

    status_ = new QLabel("0", this);
    top->addWidget(status_);

    entries_model_ = new LogEntriesModel(this);

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setSourceModel(entries_model_);
    proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy_->setFilterKeyColumn(-1);
    proxy_->setFilterRole(Qt::UserRole);

    table_ = new QTableView(this);
    table_->setModel(proxy_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setAlternatingRowColors(true);
    table_->setWordWrap(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table_->verticalHeader()->setVisible(false);

    root->addWidget(table_, 1);

    connect(reload_, &QPushButton::clicked, this, &QtLogViewerWidget::reloadSources);
    connect(clear_, &QPushButton::clicked, this, &QtLogViewerWidget::clearView);
    connect(filter_, &QLineEdit::textChanged, this, &QtLogViewerWidget::onFilterTextChanged);

    connect(sources_, QOverload<int>::of(&QComboBox::activated),
            this, &QtLogViewerWidget::onSourceActivated);
}

void QtLogViewerWidget::reloadSources()
{
    const QString prev = sources_->currentText();

    const auto names = vm_.loadNames();

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
    if (proxy_->rowCount() <= 0) return;

    table_->scrollTo(proxy_->index(proxy_->rowCount() - 1, 0));
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
