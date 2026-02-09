#ifndef CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
#define CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H


namespace mvvm {
    class VideoSourceGridViewModel;

    class MainWindowViewModel {
    public:
        MainWindowViewModel(VideoSourceGridViewModel& video_source_grid);
        VideoSourceGridViewModel& videoSourceGridModel();

    private:
        VideoSourceGridViewModel& video_source_grid_;
    };
}


#endif //CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
