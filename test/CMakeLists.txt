FILE(
    GLOB_RECURSE SRC_LIST 
    ./*.c
    ./*.cpp
)

# Exe output path
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

ADD_EXECUTABLE(${TEST_NAME} ${SRC_LIST})

# Link lib and so
TARGET_LINK_LIBRARIES(
    ${TEST_NAME}
    libcjson.so

    libglib-2.0.so
    libgstreamer-1.0.so
    libgobject-2.0.so
    libgstapp-1.0.so

    libmali.so
    libOpenCL.so

    libopencv_core.so;
    libopencv_highgui.so;
    libopencv_imgproc.so;
    libopencv_imgcodecs.so
)