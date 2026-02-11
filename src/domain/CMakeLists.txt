add_library(domain STATIC fmt/fmt_domain.cpp
        events/video/VideoSourceFrameCaptured.h
        events/video/VideoEvent.h
        events/video/VideoEventType.h)


target_include_directories(domain
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)