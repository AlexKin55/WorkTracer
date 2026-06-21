cmake_minimum_required(VERSION 3.12)
project(WorkTracer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

find_package(X11 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS DBus)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(CURL REQUIRED)

pkg_check_modules(TESSERACT REQUIRED tesseract)
pkg_check_modules(LEPTONICA REQUIRED lept)

set(SOURCES
    src/main.cpp
    
    src/ActivityWatcher/KWinActivityWatcher.cpp
    src/ActivityWatcher/MutterActivityWatcher.cpp
    src/ActivityWatcher/X11ActivityWatcher.cpp
    
    src/Screenshot/KWinScreenshot.cpp
    src/Screenshot/MutterScreenshot.cpp
    src/Screenshot/X11Screenshot.cpp
    
    src/TextParser/TesseractTextParser.cpp
    
    src/ReportMaker/YandexGPT.cpp
    
    src/Common/Utilities.cpp
    src/ScreenshotService.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    headers               
    src                   
    src/ActivityWatcher   
    src/Screenshot
    src/TextParser
    src/ReportMaker
    src/Common
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${X11_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${TESSERACT_INCLUDE_DIRS}
    ${LEPTONICA_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${X11_LIBRARIES}
    ${OpenCV_LIBS}
    ${TESSERACT_LIBRARIES}
    ${LEPTONICA_LIBRARIES}
    ${CURL_LIBRARIES}
    Qt6::DBus
    yaml-cpp
    nlohmann_json::nlohmann_json
)
