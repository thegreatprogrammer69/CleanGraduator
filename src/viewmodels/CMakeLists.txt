add_library(viewmodels STATIC
        VideoSourceViewModel.cpp
        VideoSourceGridViewModel.cpp
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