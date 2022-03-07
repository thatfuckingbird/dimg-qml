cmake_minimum_required(VERSION 3.5)

project(dimg-sample LANGUAGES CXX)

add_subdirectory(dimg)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Quick QuickControls2 Xml REQUIRED)

qt_add_resources(QML_RESOURCES qml.qrc)
add_executable(dimg-sample
    main.cpp
    item.h
    item.cpp
    dimg_shims.cpp
    ${QML_RESOURCES}
)
set_target_properties(dimg-sample PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_link_libraries(dimg-sample PRIVATE Qt6::Quick Qt6::Xml $<TARGET_OBJECTS:dimg::digikam_shims>
    dimg::digikamcore dimg::DImg_PNG_Plugin dimg::DImg_TIFF_Plugin dimg::DImg_JPEG_Plugin dimg::DImg_QImage_Plugin dimg::DImg_HEIF_Plugin dimg::DImg_JPEG2000_Plugin dimg::DImg_ImageMagick_Plugin dimg::DImg_PGF_Plugin dimg::libraw dimg::DImg_RAW_Plugin)
