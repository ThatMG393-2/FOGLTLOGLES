cmake_minimum_required(VERSION 3.11)

project(FOGLTLOGLES LANGUAGES C CXX)

######## EARLY PROJECT SETTINGS ########
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

######## CCACHE ########
## https://github.com/jomof/android-ccache-example/blob/master/app/src/main/cpp/CMakeLists.txt
if( CMAKE_HOST_WIN32 )
    # Find ccache on Windows. Edit PATHS variable if MSYS2 is installed in a different location.
    find_program( CCACHE ccache PATHS "C:/msys64/usr/bin" )
    if (CCACHE)
        set( ENV{CCACHE_BASEDIR} "${CMAKE_BINARY_DIR}" )
        # As of ccache 3.4.3 there is a bug that requires the compiler path to clang have forward
        # slashes. However, CMake as of 3.12.2 passes path to clang with backslashes. To overcome
        # this limitation, a simple batch file wrap-cache.bat is used. This batch file converts
        # back-slashes to forward slashes and then invokes ccache.
        set( CMAKE_CXX_COMPILER_LAUNCHER "${CMAKE_SOURCE_DIR}/tools/wrap-ccache.bat ${CCACHE}" )
        set( CMAKE_C_COMPILER_LAUNCHER  "${CMAKE_SOURCE_DIR}/tools/wrap-ccache.bat ${CCACHE}" )
    else()
        message( INFO "\
Could not find CCache program. \
One way to get CCache on Windows is to install MSYS2 at https://www.msys2.org/. \
Once installed, use the pacman package manager to install ccache: \
pacman --sync ccache. ")
    endif()
else()
    find_program( CCACHE ccache )
    if ( CCACHE )
        message( INFO "ccache found! path=${CCACHE}" )
        set( CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE}" )
        set( CMAKE_C_COMPILER_LAUNCHER "${CCACHE}" )
    else()
        message( INFO "Could not find CCache program." )
    endif()
endif()

######## DEPENDENCIES ########
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_SKIP_TESTS ON)
## set(SHADERC_SKIP_INSTALL ON)
set(SPIRV_SKIP_TESTS ON)
set(SPIRV_CROSS_ENABLE_TESTS OFF)
set(SPIRV_CROSS_ENABLE_HLSL OFF)
set(SPIRV_CROSS_ENABLE_MSL OFF)
set(SPIRV_CROSS_CLI OFF)
set(SPIRV_CROSS_ENABLE_C_API OFF)
set(SPIRV_CROSS_ENABLE_REFLECT OFF)
set(GLSLANG_TESTS OFF)
## set(ENABLE_HLSL OFF) ## fails on libshaderc_util
set(GLM_ENABLE_SIMD_NEON ON)
set(GLM_ENABLE_FAST_MATH ON)

add_subdirectory(deps/shaderc) 
add_subdirectory(deps/spirv-cross)
add_subdirectory(deps/glm)
add_subdirectory(deps/fmt)
add_subdirectory(deps/frozen)

######## MAIN PROJECT ########
set(COMMON_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -pipe
)

set(RELEASE_FLAGS
    -O3
    -flto
    -ffunction-sections
    -fdata-sections
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -DNDEBUG
)

set(DEBUG_FLAGS
    -Og
    -g3
    -ggdb
    -fno-omit-frame-pointer
    -D_GLIBCXX_DEBUG
)

file(GLOB_RECURSE SOURCES "src/*.cpp")

set(HEADER_DIRS "src" "build_info")

add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
	CXX_VISIBILITY_PRESET hidden
	C_VISIBILITY_PRESET hidden
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
	${HEADER_DIRS}
)

######## EXTRA ########
if(CMAKE_BUILD_TYPE MATCHES Release)
    string(REPLACE ";" " " RELEASE_FLAGS_STR "${RELEASE_FLAGS}")
    string(REPLACE ";" " " COMMON_FLAGS_STR "${COMMON_FLAGS}")
    
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_FLAGS_STR} ${RELEASE_FLAGS_STR}")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -Wl,--as-needed")
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Stripping symbols from ${PROJECT_NAME}"
    )
else()
    string(REPLACE ";" " " DEBUG_FLAGS_STR "${DEBUG_FLAGS}")
    string(REPLACE ";" " " COMMON_FLAGS_STR "${COMMON_FLAGS}")
    
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_FLAGS_STR} ${DEBUG_FLAGS_STR}")
endif()

######## DEPENDENCIES ########
find_library(ANDROID_DEP android REQUIRED)
find_library(EGL_DEP EGL REQUIRED)
find_library(GLES2_DEP GLESv2 REQUIRED)
find_library(GLES3_DEP GLESv3 REQUIRED)
find_library(LOG_DEP log REQUIRED)
find_library(DL_DEP dl REQUIRED)
find_library(CXX_STATIC_DEP c++_static REQUIRED)

set(DEPENDENCIES
	${ANDROID_DEP}
	${EGL_DEP}
	${GLES2_DEP}
	${GLES3_DEP}
	${LOG_DEP}
	${DL_DEP}
	${CXX_STATIC_DEP}

	# Static ShaderC
	shaderc

	# Minimal static SPIRV-Cross
	spirv-cross-core
	spirv-cross-glsl

	# OpenMP static
	-fopenmp
	-static-openmp

    # GLM
    glm::glm

    # FMT
    fmt::fmt

    # Frozen
    frozen::frozen-headers
)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_OPTIONS -fopenmp)
target_link_libraries(
	${PROJECT_NAME} PRIVATE ${DEPENDENCIES}
)

message("Sources      : ${SOURCES}")
message("Dependencies : ${DEPENDENCIES}")

set(BUILD_INFO_DIR ${CMAKE_SOURCE_DIR}/build_info)

add_custom_target(generateBuildInfo
    ${CMAKE_COMMAND} -D SRC=${BUILD_INFO_DIR}/build_info.hpp.in
                     -D DST=${BUILD_INFO_DIR}/build_info.hpp
                     -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
                     -P ${BUILD_INFO_DIR}/GenerateBuildInfo.cmake
)

add_dependencies(${PROJECT_NAME} generateBuildInfo)
