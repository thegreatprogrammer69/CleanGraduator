#ifndef CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
#define CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H


namespace mvvm {
    class VideoSourceGridSettingsViewModel;
    class VideoSourceGridViewModel;

    class MainWindowViewModel {
    public:
        MainWindowViewModel(VideoSourceGridViewModel& video_source_grid, VideoSourceGridSettingsViewModel& video_source_grid_settings_view_model);
        VideoSourceGridViewModel& videoSourceGridModel();
        VideoSourceGridSettingsViewModel& videoSourceGridSettingsViewModel();

    private:
        VideoSourceGridViewModel& video_source_grid_;
        VideoSourceGridSettingsViewModel& video_source_grid_settings_;
    };
}


#endif //CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
