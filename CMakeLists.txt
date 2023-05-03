cmake_minimum_required(VERSION 3.5)

project(BamThriving-VISION)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)



add_executable(BamThriving-VISION main.cpp dahua/dhua.cpp dahua/streamRetrieve.cpp
    serial/serial.cpp buff/BUFF.cpp buff/CircleFitting.cpp buff/kalman.cpp
    Thread/threads.cpp Thread/threads_serial.cpp Thread/threads_some.cpp
    Prediction/pnpsolve.cpp Prediction/predictor.cpp Detection/detect.cpp Detection/form_armor.cpp Detection/number.cpp Number/svm.cpp)



#--------基本库--------
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(/opt/DahuaTech/MVviewer/include)
include_directories(/usr/local/include/eigen3)
include_directories(/usr/local/include/Eigen)

target_link_libraries(BamThriving-VISION
    libpthread.so
    ${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} "/opt/DahuaTech/MVviewer/lib/libMVSDK.so")
