cmake_minimum_required(VERSION 3.16)

project(OmniView3D VERSION 0.1 LANGUAGES CXX)

# ========== АВТОГЕНЕРАЦИЯ .clangd ==========
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Получаем версию GCC
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
        OUTPUT_VARIABLE GCC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    # Получаем путь к компилятору
    get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    
    # Для MinGW: C:/msys64/ucrt64/bin/g++.exe
    # Нам нужно получить C:/msys64/ucrt64
    get_filename_component(MSYS2_ROOT "${COMPILER_DIR}/.." ABSOLUTE)
    
    # Пути к заголовкам
    set(CXX_INCLUDE_DIR "${MSYS2_ROOT}/include/c++/${GCC_VERSION}")
    set(COMPILER_INCLUDE_DIR "${MSYS2_ROOT}/lib/gcc/x86_64-w64-mingw32/${GCC_VERSION}/include")
    set(INCLUDE_FIXED_DIR "${MSYS2_ROOT}/lib/gcc/x86_64-w64-mingw32/${GCC_VERSION}/include-fixed")
    set(SYSTEM_INCLUDE_DIR "${MSYS2_ROOT}/include")
    
    # Нормализуем пути (заменяем  на /)
    string(REPLACE "\" "/" CXX_INCLUDE_DIR "${CXX_INCLUDE_DIR}")
    string(REPLACE "\" "/" COMPILER_INCLUDE_DIR "${COMPILER_INCLUDE_DIR}")
    string(REPLACE "\" "/" INCLUDE_FIXED_DIR "${INCLUDE_FIXED_DIR}")
    string(REPLACE "\" "/" SYSTEM_INCLUDE_DIR "${SYSTEM_INCLUDE_DIR}")
    
    # Генерируем файл .clangd
    file(WRITE "${CMAKE_SOURCE_DIR}/.clangd"
"---
CompileFlags:
  Add:
    - -I${CXX_INCLUDE_DIR}
    - -I${CXX_INCLUDE_DIR}/x86_64-w64-mingw32
    - -I${CXX_INCLUDE_DIR}/backward
    - -I${COMPILER_INCLUDE_DIR}
    - -I${INCLUDE_FIXED_DIR}
    - -I${SYSTEM_INCLUDE_DIR}
---
")
    message(STATUS "✓ Generated .clangd for GCC ${GCC_VERSION}")
    message(STATUS "  MSYS2 root: ${MSYS2_ROOT}")
    message(STATUS "  C++ include: ${CXX_INCLUDE_DIR}")
endif()
# ===========================================

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для MSVC
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Z7")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /Ot /GL")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/Os /Oi /Ot /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /DEBUG:NONE /PDB:NONE")
endif()

# Для GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -s")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets
    Qml
    OpenGLWidgets
)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(OmniView3D
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        fileparser.cpp fileparser.h
        glwidget.cpp glwidget.h
        IconTools.h
        lightdialog.cpp lightdialog.h lightdialog.ui
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        base_shader.vert
        base_shader.frag
        axes_shader.frag
        axes_shader.vert
        Convert3D.h
    )

# После add_executable
target_include_directories(OmniView3D PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/OmniView3D_autogen/include
)

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET OmniView3D APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(OmniView3D SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(OmniView3D
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(OmniView3D PRIVATE 
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        OpenGL32.lib
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.OmniView3D)
endif()
set_target_properties(OmniView3D PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS OmniView3D
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(OmniView3D)
endif()
