cmake_minimum_required(VERSION 4.1.1)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
# cmake_minimum_required(VERSION 4.3.2)
# set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_AUTORCC ON)

project(chinesechess VERSION 0.1 LANGUAGES CXX)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network)
qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(appchinesechess  WIN32)
qt_add_qml_module(appchinesechess
    URI chinesechess
    VERSION 1.0
    SOURCES     main.cpp
    QML_FILES   Main.qml StartPage.qml GamePage.qml ResultDialog.qml
    SOURCES chessboard.h chessboard.cpp
    SOURCES chesspiece.h chesspiece.cpp
    SOURCES chessmanager.h chessmanager.cpp
    RESOURCES resources.qrc
    # SOURCES
    # RESOURCES introduce.txt
)
target_compile_features(appchinesechess PRIVATE cxx_std_23)
target_link_libraries(appchinesechess
    PRIVATE Qt6::Quick Qt6::Network
)

set(CMAKE_INSTALL_PREFIX "/opt/${PROJECT_NAME}" CACHE PATH "Install path prefix" FORCE)
install(TARGETS appchinesechess
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# C++ Modules support
set(CHINESECHESS_MODULE_FILES
    # math.cppm                             # 模块文件
)
source_group("chinesechess" FILES ${CHINESECHESS_MODULE_FILES})
target_sources(appchinesechess
    PRIVATE FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}
        FILES  ${CHINESECHESS_MODULE_FILES}
)
