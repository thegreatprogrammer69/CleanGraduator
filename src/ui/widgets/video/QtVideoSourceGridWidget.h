#ifndef CLEANGRADUATOR_QTVIDEOSOURCEGRIDWIDGET_H
#define CLEANGRADUATOR_QTVIDEOSOURCEGRIDWIDGET_H
#include <QWidget>


class QFrame;
class QGridLayout;

namespace mvvm {
    class VideoSourceGridViewModel;
}

namespace ui {

    class QtVideoSourceGridWidget final : public QWidget {
        Q_OBJECT
        public:
        explicit QtVideoSourceGridWidget(
            mvvm::VideoSourceGridViewModel& model,
            QWidget *parent = nullptr);

        void setAspectRatio(double aspectRatioWH);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void recalcSizes();
        void rebuildFromModel();

        int m_rows = 4;
        int m_cols = 2;
        double m_aspect = 16.0 / 9.0;

        QGridLayout *m_grid = nullptr;
        QVector<QFrame*> m_tiles;

        bool m_inRecalc = false;
        mvvm::VideoSourceGridViewModel& m_model;
    };

}


#endif //CLEANGRADUATOR_QTVIDEOSOURCEGRIDWIDGET_H