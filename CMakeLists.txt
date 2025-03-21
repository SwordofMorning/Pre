PROJECT(VOCs_Pre)

CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# ===== Step 1 : Set Cross Compiler Path =====

SET(_SDK_PATH_ "/home/xjt/_Workspace_/System/")
SET(_BUILDROOT_PATH_ "${_SDK_PATH_}rk3588-linux/buildroot/output/rockchip_rk3588/host/")
SET(_COMPILER_PATH_ "${_BUILDROOT_PATH_}bin/")

SET(CMAKE_C_COMPILER ${_COMPILER_PATH_}aarch64-buildroot-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER ${_COMPILER_PATH_}aarch64-buildroot-linux-gnu-g++)

# ===== Step 2 : Set Flags =====

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s -O3 -lrt -pthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s -O3 -lrt -pthread -std=c++20")

# ===== Step 3 : Include & Lib =====

SET(_INCLUDE_PATH_ "${_BUILDROOT_PATH_}/aarch64-buildroot-linux-gnu/sysroot/usr/include/")
SET(_LIB_PATH_ "${_BUILDROOT_PATH_}/aarch64-buildroot-linux-gnu/sysroot/usr/lib/")

LINK_DIRECTORIES(
    ${_LIB_PATH_}
)

INCLUDE_DIRECTORIES(
    ${_INCLUDE_PATH_}
)

# ===== Step 4 : Set Application Name =====

SET(APP_NAME "pre")
SET(SHM_NAME "pre_shm")
SET(SHM_GST_NAME "pre_shm_gst")
SET(TEST_NAME "pre_test")

# ===== Step 5 : Add Subdirectory =====

ADD_SUBDIRECTORY(src bin)
ADD_SUBDIRECTORY(shm)
ADD_SUBDIRECTORY(shm_gst)
ADD_SUBDIRECTORY(test)

# ===== Step 6: Generate Version Info =====

# Branch
EXECUTE_PROCESS(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Last commit ID
EXECUTE_PROCESS(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# User
EXECUTE_PROCESS(
    COMMAND git config user.name
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_USER
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Git Clean
EXECUTE_PROCESS(
    COMMAND git diff-index --quiet HEAD --
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_DIRTY   
)
IF(GIT_DIRTY EQUAL 0)
    SET(GIT_CLEAN "Clean")
ELSE()
    SET(GIT_CLEAN "Dirty")
ENDIF()

# Host
EXECUTE_PROCESS(
    COMMAND hostname
    OUTPUT_VARIABLE COMPILE_HOST
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# User
EXECUTE_PROCESS(
    COMMAND whoami
    OUTPUT_VARIABLE COMPILE_USER
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Time
STRING(TIMESTAMP COMPILE_TIME "%Y-%m-%d %H:%M:%S")

CONFIGURE_FILE(
    ${CMAKE_SOURCE_DIR}/others/version/version.h.in
    ${CMAKE_SOURCE_DIR}/others/version/version.h
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/others/)

# ===== Step 7 : Copy Configuration Files =====

FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/cl)

CONFIGURE_FILE(
    ${CMAKE_SOURCE_DIR}/others/tm/tm_params.ini
    ${CMAKE_BINARY_DIR}/bin/tm_params.ini
    COPYONLY
)

FILE(GLOB CL_FILES "${CMAKE_SOURCE_DIR}/src/config/cl/*.cl")
FILE(COPY ${CL_FILES} DESTINATION ${CMAKE_BINARY_DIR}/bin/cl)