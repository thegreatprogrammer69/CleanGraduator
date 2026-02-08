add_library(application STATIC
    read/MeasurementSetupQuery.cpp
)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)