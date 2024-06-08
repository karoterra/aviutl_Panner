add_library(PannerConfig SHARED
    src.cpp
    ../common/version.hpp
    ../common/panlaw.hpp
)
target_include_directories(PannerConfig
    PRIVATE ${PROJECT_SOURCE_DIR}/vendor/aviutl_exedit_sdk
)
target_link_libraries(PannerConfig comctl32)
target_compile_definitions(PannerConfig
    PRIVATE
        VERSION="${PROJECT_VERSION}"
)
target_compile_options(PannerConfig
    PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>"
        "$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:shift_jis>"
)
set_target_properties(PannerConfig PROPERTIES SUFFIX .auf)
