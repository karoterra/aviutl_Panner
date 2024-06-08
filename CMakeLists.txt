cmake_minimum_required(VERSION 3.21)
project(aviutl_Panner
    LANGUAGES CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/config)
add_subdirectory(src/panner)

add_custom_target(package
    COMMAND 7z a publish/${PROJECT_NAME}_v${PROJECT_VERSION}.zip
        $<TARGET_FILE:PannerConfig>
        $<TARGET_FILE:PannerMain>
        README.md
        LICENSE
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    DEPENDS PannerConfig PannerMain
)
