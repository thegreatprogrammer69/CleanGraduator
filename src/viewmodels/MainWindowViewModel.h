#ifndef CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
#define CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H


namespace mvvm {
    class CameraGridSettingsViewModel;
    class VideoSourceGridViewModel;

    class MainWindowViewModel {
    public:
        MainWindowViewModel(VideoSourceGridViewModel& video_source_grid, CameraGridSettingsViewModel& camera_grid_settings);
        VideoSourceGridViewModel& videoSourceGridModel();
        CameraGridSettingsViewModel& cameraGridSettingsViewModel();

    private:
        VideoSourceGridViewModel& video_source_grid_;
        CameraGridSettingsViewModel& camera_grid_settings_;
    };
}


#endif //CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
