cmake_minimum_required(VERSION 2.8)
project(OpenView)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

if(APPLE)
  find_path(COCOA_INCLUDE_DIR Cocoa.h)
  message(${COCOA_INCLUDE_DIR})
  include_directories(${COCOA_INCLUDE_DIR})
endif()

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

qt5_wrap_cpp( OPENVIEW_MOC_FILES
  QVTKInteractorAdapter.h
  QVTKInteractorInternal.h
  QVTKQuickItem.h
  vtkQtConnection.h
  ovGraphView.h
  ovScatterPlotView.h
  ovScatterPlot3DView.h
  ovViewPlugin.h
  ovViewQuickItem.h
  ovView.h )

qt5_add_resources( OPENVIEW_RESOURCES
  resources.qrc )

set( OPENVIEW_SRCS
  vtkEventQtSlotConnect.cxx
  QVTKInteractor.cxx
  QVTKInteractorAdapter.cxx
  QVTKQuickItem.cxx
  vtkQtConnection.cxx
  ovContextInteractorStyle.cxx
  ovGraphItem.cxx
  ovGraphView.cxx
  ovScatterPlotView.cxx
  ovScatterPlot3DView.cxx
  ovViewQuickItem.cxx
  ovView.cxx
  ${OPENVIEW_MOC_FILES}
  ${OPENVIEW_RESOURCES} )

get_target_property(coredir Qt5::Core IMPORTED_LOCATION_DEBUG)
message("coredir: ${coredir}")

set( OPENVIEW_LIBS
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

if(APPLE)
  add_executable(OpenView MACOSX_BUNDLE ${OPENVIEW_SRCS} main.mm)
  target_link_libraries(OpenView "-framework Cocoa")
else()
  add_executable(OpenView ${OPENVIEW_SRCS} main.cxx)
endif()
target_link_libraries(OpenView ${OPENVIEW_LIBS})

install(TARGETS OpenView RUNTIME BUNDLE DESTINATION "bin")

include(CPack)
