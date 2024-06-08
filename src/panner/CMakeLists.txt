add_library(PannerMain SHARED
    src.cpp
    ../common/version.hpp
    ../common/panlaw.hpp
)
target_include_directories(PannerMain
    PRIVATE ${PROJECT_SOURCE_DIR}/vendor/aviutl_exedit_sdk
)
target_compile_definitions(PannerMain
    PRIVATE
        VERSION="${PROJECT_VERSION}"
)
target_compile_options(PannerMain
    PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>"
        "$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:shift_jis>"
)
set_target_properties(PannerMain PROPERTIES SUFFIX .eef)
