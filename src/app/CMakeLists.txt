add_executable(CleanGraduator
        main.cpp
        cameragridwidget.h
        cameragridwidget.cpp
        mainwindow.h
        mainwindow.cpp
)

target_link_libraries(CleanGraduator
        PRIVATE
        domain
        application
        ui
        infrastructure
)

target_link_libraries(CleanGraduator PRIVATE Qt5::Widgets)
