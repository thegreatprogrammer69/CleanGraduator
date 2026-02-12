add_library(viewmodels STATIC
        video/VideoSourceViewModel.cpp
        video/VideoSourceGridViewModel.cpp

        settings/CameraGridSettingsViewModel.cpp

        component/ComponentConfigViewModel.cpp

        MainWindowViewModel.cpp
)

target_include_directories(viewmodels
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(viewmodels
        PUBLIC
        domain
        application
)