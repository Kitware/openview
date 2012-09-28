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
  ${Qt5Quick_INCLUDE_DIRS}
  ${Qt5OpenGL_INCLUDE_DIRS} )

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
  QVTKInteractorAdapter.h
  QVTKInteractorInternal.h
  QVTKQuickItem.h
  vtkQtConnection.h
  ovViewPlugin.h
  ovView.h )

add_library( ovviewplugin SHARED
  vtkEventQtSlotConnect.cxx
  QVTKInteractor.cxx
  QVTKInteractorAdapter.cxx
  QVTKQuickItem.cxx
  vtkQtConnection.cxx
  ovContextInteractorStyle.cxx
  ovGraphItem.cxx
  ovViewPlugin.cxx
  ovView.cxx
  ${OPENVIEW_APP_MOC_FILES} )

target_link_libraries( ovviewplugin
  vtkChartsCore
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

add_executable(OpenDemo main.cxx)
target_link_libraries(OpenDemo ovviewplugin)

file(COPY qmldir DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
