cmake_minimum_required(VERSION 2.8)
project(OpenView)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

if(APPLE)
  find_path(COCOA_INCLUDE_DIR Cocoa.h)
  include_directories(${COCOA_INCLUDE_DIR})
endif()

include_directories(
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Quick_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/core )

add_definitions(
  ${Qt5Widgets_DEFINITIONS}
  ${Qt5Quick_DEFINITIONS} )

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}
  ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Quick_EXECUTABLE_COMPILE_FLAGS} )

if(APPLE)
  set(install_name_changes "")
  set(libs_and_execs "")
endif()

add_subdirectory(core)

qt5_wrap_cpp( OPENVIEW_MOC_FILES
  ovGraphView.h
  ovScatterPlotView.h
  ovScatterPlot3DView.h
  ovTreemapView.h
  ovTreeringView.h
  ovTreeView.h
  ovViewPlugin.h
  ovViewQuickItem.h
  ovView.h
  ovWorkflowQuickItem.h )

qt5_add_resources( OPENVIEW_RESOURCES
  resources.qrc )

set( OPENVIEW_SRCS
  ovAlgorithmItem.cxx
  ovContextInteractorStyle.cxx
  ovGraphItem.cxx
  ovGraphView.cxx
  ovScatterPlotView.cxx
  ovScatterPlot3DView.cxx
  ovTreemapItem.cxx
  ovTreemapView.cxx
  ovTreeringItem.cxx
  ovTreeringView.cxx
  ovTreeView.cxx
  ovViewQuickItem.cxx
  ovView.cxx
  ovWorkflowItem.cxx
  ovWorkflowQuickItem.cxx
  ${OPENVIEW_MOC_FILES}
  ${OPENVIEW_RESOURCES} )

set( OPENVIEW_LIBS
  ovCore
  vtkChartsCore
  vtkInteractionStyle
  vtkIOInfovis
  vtkRenderingCore
  vtkRenderingFreeTypeOpenGL
  vtkRenderingVolumeOpenGL
  vtkViewsContext2D
  vtkViewsInfovis
  vtkInfovisBoostGraphAlgorithms
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Quick_LIBRARIES} )

if(APPLE)
  add_executable(OpenView ${OPENVIEW_SRCS} main.mm)
else()
  add_executable(OpenView ${OPENVIEW_SRCS} main.cxx)
endif()
target_link_libraries(OpenView ${OPENVIEW_LIBS})

if(APPLE)
  install(TARGETS OpenView RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/MacOS/")
  set(executable_path "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/MacOS/OpenView")
  list(APPEND libs_and_execs ${executable_path})
endif(APPLE)

add_subdirectory(install)
