cmake_minimum_required(VERSION 2.8)

find_package(Qt4 REQUIRED)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

qt4_wrap_cpp( OPENVIEW_APP_MOC_FILES
  ovViewPlugin.h
  ovView.h )

add_library( ovviewplugin SHARED
  ovGLContext.cxx
  ovViewPlugin.cxx
  ovView.cxx
  ${OPENVIEW_APP_MOC_FILES} )

include_directories(${QT_INCLUDES})

target_link_libraries( ovviewplugin
  vtkGUISupportQtOpenGL
  vtkInteractionStyle
  vtkRenderingCore
  vtkRenderingFreeTypeOpenGL
  vtkRenderingVolumeOpenGL
  vtkViewsContext2D
  vtkViewsInfovis
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDECLARATIVE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTOPENGL_LIBRARY} )

add_executable(opendemo main.cxx)
target_link_libraries(opendemo ovviewplugin)
