add_library(domain STATIC fmt/fmt_domain.cpp)


target_include_directories(domain
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)