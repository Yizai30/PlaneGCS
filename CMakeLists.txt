cmake_minimum_required(VERSION 3.15)
project(PlaneGCS CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg integration
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# 查找依赖包
find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS locale graph)
find_package(CURL REQUIRED)
find_package(cpprestsdk REQUIRED)

# 输出包信息
message("Eigen3 version: ${Eigen3_VERSION}")
message("Eigen3 include dirs: ${Eigen3_INCLUDE_DIRS}")
message("Boost version: ${Boost_VERSION}")
message("Boost include dirs: ${Boost_INCLUDE_DIRS}")
message("Boost libraries: ${Boost_LIBRARIES}")
message("CURL version: ${CURL_VERSION_STRING}")
message("cpprestsdk found: ${cpprestsdk_FOUND}")

# 包含头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/aichater
    ${CMAKE_SOURCE_DIR}/models_for_test
)

SET(PlaneGCS_SRCS
    src/GCS.cpp
    src/GCS.h
    src/Util.h
    src/Geo.cpp
    src/Geo.h
    src/Constraints.cpp
    src/Constraints.h
    src/SubSystem.cpp
    src/SubSystem.h
    src/qp_eq.cpp
    src/qp_eq.h
)
#SOURCE_GROUP("PlaneGCS" FILES ${PlaneGCS_SRCS})

# 创建静态库（避免DLL依赖问题）
add_library(PlaneGCS STATIC ${PlaneGCS_SRCS})

# 设置PlaneGCS库的包含目录和链接库
target_include_directories(PlaneGCS PRIVATE ${Eigen3_INCLUDE_DIRS})
target_include_directories(PlaneGCS PRIVATE ${Boost_INCLUDE_DIRS})

# 链接必要的库
target_link_libraries(PlaneGCS
    Eigen3::Eigen
    Boost::boost
    Boost::locale
    Boost::graph
)

# 设置编译选项（Windows特定）
if(MSVC)
    target_compile_definitions(PlaneGCS PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
    )
    target_compile_options(PlaneGCS PRIVATE /W3)
endif()

# 数学动画演示
add_executable(math_animation_demo)
target_sources(math_animation_demo PRIVATE
    examples/math_animation_demo.cpp
)
target_link_libraries(math_animation_demo PlaneGCS)

# LLM 几何动画演示（主要示例）
#add_executable(llm_geometry_animation)
#target_sources(llm_geometry_animation PRIVATE
#    examples/llm_geometry_animation.cpp
#    aichater/AIChater.cpp
#    models_for_test/IterationState.cpp
#)

# 链接LLM几何动画的依赖库
#target_link_libraries(llm_geometry_animation
#    PlaneGCS
#    Eigen3::Eigen
#    Boost::boost
#    Boost::locale
#    CURL::libcurl
#    cpprestsdk::cpprest
#)

# 添加解决方案到动画关键帧演示程序
add_executable(solution_to_keyframes_demo
    examples/solution_to_keyframes_demo.cpp
    examples/solution_to_keyframes_core.cpp
    aichater/AIChater.cpp
    models_for_test/IterationState.cpp
)

# 链接LLM几何动画的依赖库
target_link_libraries(solution_to_keyframes_demo
    PlaneGCS
    Eigen3::Eigen
    Boost::boost
    Boost::locale
    CURL::libcurl
    cpprestsdk::cpprest
)

# 设置可执行文件的编译选项
foreach(target math_animation_demo 
    #llm_geometry_animation
    solution_to_keyframes_demo)
    if(MSVC)
        target_compile_definitions(${target} PRIVATE
            _CRT_SECURE_NO_WARNINGS
            _SCL_SECURE_NO_WARNINGS
        )
        target_compile_options(${target} PRIVATE /W3)
    endif()

    # 设置输出目录
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )
endforeach()

# 设置库的输出目录
set_target_properties(PlaneGCS PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)