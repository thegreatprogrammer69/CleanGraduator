add_library(application STATIC
    usecases/StubUseCase.h
    usecases/StubUseCase.cpp
)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)