#ifndef CLEANGRADUATOR_QTLOGVIEWERWIDGET_H
#define CLEANGRADUATOR_QTLOGVIEWERWIDGET_H

#include <QWidget>
#include <QString>

#include "application/models/logging/LogEntry.h"
#include "application/models/logging/LogSource.h"
#include "viewmodels/Observable.h"


class QComboBox;
class QLineEdit;
class QTableView;
class QPushButton;
class QCheckBox;
class QLabel;
class QSortFilterProxyModel;

namespace mvvm { class LogViewerViewModel; }

namespace ui {

    class QtLogViewerWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtLogViewerWidget(mvvm::LogViewerViewModel& vm, QWidget* parent = nullptr);
        ~QtLogViewerWidget() override;

    private slots:
        void reloadSources();
        void onSourceActivated(int index);
        void clearView();
        void onFilterTextChanged(const QString& text);

    private:
        void buildUi();
        void resetFromSelected();                 // (re)load history for selected source
        void appendEntry(const application::models::LogEntry& e); // append single entry to model
        void scrollToBottomIfFollow();

    private:
        mvvm::LogViewerViewModel& vm_;

        mvvm::Observable<std::optional<application::models::LogSource>>::Subscription log_source_sub;
        mvvm::Observable<application::models::LogEntry>::Subscription new_log_sub;

        QComboBox* sources_{nullptr};
        QLineEdit* filter_{nullptr};
        QTableView* table_{nullptr};
        QPushButton* reload_{nullptr};
        QPushButton* clear_{nullptr};
        QCheckBox* follow_{nullptr};
        QLabel* status_{nullptr};

        class LogEntriesModel;
        LogEntriesModel* entries_model_{nullptr};
        QSortFilterProxyModel* proxy_{nullptr};

        QString current_source_name_;
    };

} // namespace ui

#endif // CLEANGRADUATOR_QTLOGVIEWERWIDGET_H
