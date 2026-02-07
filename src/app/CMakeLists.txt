add_executable(CleanGraduator
        main.cpp
)

target_link_libraries(CleanGraduator
        PRIVATE
        domain
        application
        ui
        infrastructure
)

target_link_libraries(CleanGraduator PRIVATE Qt5::Widgets)
