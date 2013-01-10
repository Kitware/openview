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
  ${Qt5Quick_INCLUDE_DIRS} )

add_definitions(
  ${Qt5Widgets_DEFINITIONS}
  ${Qt5Quick_DEFINITIONS} )

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}
  ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Quick_EXECUTABLE_COMPILE_FLAGS} )

qt5_wrap_cpp( OPENVIEW_MOC_FILES
  QVTKInteractorAdapter.h
  QVTKInteractorInternal.h
  QVTKQuickItem.h
  vtkQtConnection.h
  ovGraphView.h
  ovScatterPlotView.h
  ovScatterPlot3DView.h
  ovTreemapView.h
  ovTreeringView.h
  ovTreeView.h
  ovViewPlugin.h
  ovViewQuickItem.h
  ovView.h
  ovWorkflowView.h )

qt5_add_resources( OPENVIEW_RESOURCES
  resources.qrc )

set( OPENVIEW_SRCS
  vtkEventQtSlotConnect.cxx
  QVTKInteractor.cxx
  QVTKInteractorAdapter.cxx
  QVTKQuickItem.cxx
  vtkQtConnection.cxx
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
  ovWorkflowView.cxx
  ${OPENVIEW_MOC_FILES}
  ${OPENVIEW_RESOURCES} )

set( OPENVIEW_LIBS
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

macro(install_qt_file filename)
  get_filename_component(filename_path ${filename} PATH)
  install(FILES "${qt_path}/${filename}" DESTINATION "OpenView.app/Contents/${filename_path}")
endmacro()

if(APPLE)
  install(TARGETS OpenView RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/MacOS/")

  set(install_name_changes "")
  set(libs_and_execs "")

  get_target_property(qt_path Qt5::Quick IMPORTED_LOCATION_DEBUG)
  get_filename_component(qt_path ${qt_path} PATH)
  get_filename_component(qt_path ${qt_path} PATH)
  get_filename_component(qt_path ${qt_path} PATH)

  file(GLOB vtk_libs "${VTK_DIR}/lib/lib*.dylib")
  foreach(vtk_lib ${vtk_libs})
    get_filename_component(vtk_lib_file ${vtk_lib} NAME)
    install(FILES ${vtk_lib} DESTINATION "OpenView.app/Contents/Libraries")
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id  @executable_path/../Libraries/${vtk_lib_file} ${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Libraries/${vtk_lib_file})")
    set(install_name_changes "${install_name_changes} -change ${vtk_lib} @executable_path/../Libraries/${vtk_lib_file}")
    list(APPEND libs_and_execs "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Libraries/${vtk_lib_file}")
  endforeach()

  install_qt_file("qml/QtQuick.2/libqtquick2plugin.dylib")
  install_qt_file("qml/QtQuick.2/qmldir")
  install_qt_file("qml/QtQuick.2/plugins.qmltypes")

  install_qt_file("qml/Qt/labs/folderlistmodel/libqmlfolderlistmodelplugin.dylib")
  install_qt_file("qml/Qt/labs/folderlistmodel/qmldir")
  install_qt_file("qml/Qt/labs/folderlistmodel/plugins.qmltypes")

  set(executable_path "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/MacOS/OpenView")
  list(APPEND libs_and_execs ${executable_path})

  foreach(lib ${libs_and_execs})
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} ${install_name_changes} ${lib})")
  endforeach()

  # Use Qt's macdeployqt to fix up the bundle
  install(CODE "execute_process(COMMAND ${qt_path}/bin/macdeployqt ${CMAKE_INSTALL_PREFIX}/OpenView.app)")

endif(APPLE)

include(CPack)
