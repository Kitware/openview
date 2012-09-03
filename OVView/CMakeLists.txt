cmake_minimum_required(VERSION 2.8)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Qml_INCLUDE_DIRS}
  /Users/jeff/dev/qt-5.0.0-alpha/qtdeclarative/include
  /Users/jeff/dev/qt-5.0.0-alpha/qtdeclarative/include/QtQml
  ${Qt5Quick_INCLUDE_DIRS}
  /Users/jeff/dev/qt-5.0.0-alpha/qtdeclarative/include
  /Users/jeff/dev/qt-5.0.0-alpha/qtdeclarative/include/QtQuick
  ${Qt5OpenGL_INCLUDE_DIRS} )
message(${Qt5Qml_INCLUDE_DIRS})
add_definitions(
  ${Qt5Core_DEFINITIONS}
  ${Qt5Gui_DEFINITIONS}
  ${Qt5Widgets_DEFINITIONS}
  ${Qt5Network_DEFINITIONS}
  ${Qt5Qml_DEFINITIONS}
  ${Qt5Quick_DEFINITIONS}
  ${Qt5OpenGL_DEFINITIONS} )
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}
  ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Gui_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Network_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Qml_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Quick_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5OpenGL_EXECUTABLE_COMPILE_FLAGS} )

qt5_wrap_cpp( OPENVIEW_APP_MOC_FILES
  QVTKPaintedItem.h
  QVTKQuickItem.h
  ovRectItem.h
  ovViewPlugin.h
  ovView.h )

add_library( ovviewplugin SHARED
  QVTKPaintedItem.cxx
  QVTKQuickItem.cxx
  ovRectItem.cxx
  ovOpenGLContext.cxx
  ovViewPlugin.cxx
  ovView.cxx
  ${OPENVIEW_APP_MOC_FILES} )

target_link_libraries( ovviewplugin
  vtkChartsCore
  vtkGUISupportQtOpenGL
  vtkInteractionStyle
  vtkIOInfovis
  vtkRenderingCore
  vtkRenderingFreeTypeOpenGL
  vtkRenderingVolumeOpenGL
  vtkViewsContext2D
  vtkViewsInfovis
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Qml_LIBRARIES}
  ${Qt5Quick_LIBRARIES}
  ${Qt5OpenGL_LIBRARIES} )

add_executable(opendemo main.cxx)
target_link_libraries(opendemo ovviewplugin)

file(COPY qmldir DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
