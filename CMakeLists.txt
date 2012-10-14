cmake_minimum_required(VERSION 2.8)
project(OpenView)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

if(APPLE)
  find_path(COCOA_INCLUDE_DIR Cocoa.h)
  include_directories(${COCOA_INCLUDE_DIR})
endif()

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Qml_INCLUDE_DIRS}
  ${Qt5Quick_INCLUDE_DIRS}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5OpenGL_INCLUDE_DIRS} )

add_definitions(
  ${Qt5Core_DEFINITIONS}
  ${Qt5Gui_DEFINITIONS}
  ${Qt5Widgets_DEFINITIONS}
  ${Qt5Network_DEFINITIONS}
  ${Qt5Qml_DEFINITIONS}
  ${Qt5Quick_DEFINITIONS}
  ${Qt5PrintSupport_DEFINITIONS}
  ${Qt5OpenGL_DEFINITIONS} )

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}
  ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Gui_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Network_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Qml_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5Quick_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5PrintSupport_EXECUTABLE_COMPILE_FLAGS}
  ${Qt5OpenGL_EXECUTABLE_COMPILE_FLAGS} )

qt5_wrap_cpp( OPENVIEW_MOC_FILES
  QVTKInteractorAdapter.h
  QVTKInteractorInternal.h
  QVTKQuickItem.h
  vtkQtConnection.h
  ovGraphView.h
  ovScatterPlotView.h
  ovScatterPlot3DView.h
  ovTreemapView.h
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
  ovTreemapItem.cxx
  ovTreemapView.cxx
  ovViewQuickItem.cxx
  ovView.cxx
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
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Qml_LIBRARIES}
  ${Qt5Quick_LIBRARIES}
  ${Qt5PrintSupport_LIBRARIES}
  ${Qt5OpenGL_LIBRARIES} )

if(APPLE)
  add_executable(OpenView ${OPENVIEW_SRCS} main.mm)
  target_link_libraries(OpenView "-framework Cocoa")
else()
  add_executable(OpenView ${OPENVIEW_SRCS} main.cxx)
endif()
target_link_libraries(OpenView ${OPENVIEW_LIBS})

macro(install_qt_framework name)
  get_target_property(location Qt5::${name} IMPORTED_LOCATION_DEBUG)
  get_filename_component(loc_path "${location}" PATH)
  set(bundle_path "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Frameworks/Qt${name}.framework/Versions/5/Qt${name}")
  set(bundle_relative_path "@executable_path/../Frameworks/Qt${name}.framework/Versions/5/Qt${name}")
  set(install_name_changes "${install_name_changes} -change ${loc_path}/Versions/5/Qt${name} ${bundle_relative_path}")
  install(DIRECTORY ${loc_path} DESTINATION "OpenView.app/Contents/Frameworks")
  install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id  ${bundle_relative_path} ${bundle_path})")
  list(APPEND libs_and_execs ${bundle_path})
endmacro()

if(APPLE)
  install(TARGETS OpenView RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/MacOS/")

  set(install_name_changes "")
  set(libs_and_execs "")

  get_target_property(qt_path Qt5::Core IMPORTED_LOCATION_DEBUG)
  get_filename_component(qt_path ${qt_path} PATH)
  get_filename_component(qt_path ${qt_path} PATH)
  get_filename_component(qt_path ${qt_path} PATH)

  install_qt_framework(Core)
  install_qt_framework(Gui)
  install_qt_framework(Widgets)
  install_qt_framework(Network)
  install_qt_framework(OpenGL)
  install_qt_framework(Qml)
  install_qt_framework(Quick)
  install_qt_framework(PrintSupport)

  # V8 is special
  set(name "V8")
  set(loc_path "${qt_path}/lib/QtV8.framework")
  set(bundle_path "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Frameworks/Qt${name}.framework/Versions/5/Qt${name}")
  set(bundle_relative_path "@executable_path/../Frameworks/Qt${name}.framework/Versions/5/Qt${name}")
  set(install_name_changes "${install_name_changes} -change ${loc_path}/Versions/5/Qt${name} ${bundle_relative_path}")
  install(DIRECTORY ${loc_path} DESTINATION "OpenView.app/Contents/Frameworks")
  install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id  ${bundle_relative_path} ${bundle_path})")
  list(APPEND libs_and_execs ${bundle_path})


  file(GLOB qt_platform_plugins "${qt_path}/plugins/platforms/lib*.dylib")
  foreach(qt_platform_plugin ${qt_platform_plugins})
    get_filename_component(qt_plugin_file ${qt_platform_plugin} NAME)
    install(FILES ${qt_platform_plugin} DESTINATION "OpenView.app/Contents/Plugins/platforms")
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @executable_path/../Plugins/platforms/${qt_plugin_file} ${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Plugins/platforms/${qt_plugin_file})")
    list(APPEND libs_and_execs "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Plugins/platforms/${qt_plugin_file}")
  endforeach()

  file(GLOB vtk_libs "${VTK_DIR}/lib/lib*.dylib")
  foreach(vtk_lib ${vtk_libs})
    get_filename_component(vtk_lib_file ${vtk_lib} NAME)
    install(FILES ${vtk_lib} DESTINATION "OpenView.app/Contents/Libraries")
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id  @executable_path/../Libraries/${vtk_lib_file} ${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Libraries/${vtk_lib_file})")
    set(install_name_changes "${install_name_changes} -change ${vtk_lib} @executable_path/../Libraries/${vtk_lib_file}")
    list(APPEND libs_and_execs "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Libraries/${vtk_lib_file}")
  endforeach()

  install(FILES "${qt_path}/imports/QtQuick.2/libqtquick2plugin_debug.dylib" DESTINATION "OpenView.app/Contents/imports/QtQuick.2")
  install(FILES "${qt_path}/imports/QtQuick.2/qmldir" DESTINATION "OpenView.app/Contents/imports/QtQuick.2")
  install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @executable_path/../imports/QtQuick.2/libqtquick2plugin_debug.dylib ${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/imports/QtQuick.2/libqtquick2plugin_debug.dylib)")
  list(APPEND libs_and_execs "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/imports/QtQuick.2/libqtquick2plugin_debug.dylib")

  install(FILES "${qt_path}/imports/Qt/labs/folderlistmodel/libqmlfolderlistmodelplugin_debug.dylib" DESTINATION "OpenView.app/Contents/imports/Qt/labs/folderlistmodel")
  install(FILES "${qt_path}/imports/Qt/labs/folderlistmodel/qmldir" DESTINATION "OpenView.app/Contents/imports/Qt/labs/folderlistmodel")
  install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @executable_path/../imports/Qt/labs/folderlistmodel/libqmlfolderlistmodelplugin_debug.dylib ${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/imports/Qt/labs/folderlistmodel/libqmlfolderlistmodelplugin_debug.dylib)")
  list(APPEND libs_and_execs "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/imports/Qt/labs/folderlistmodel/libqmlfolderlistmodelplugin_debug.dylib")

  set(executable_path "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/MacOS/OpenView")
  list(APPEND libs_and_execs ${executable_path})

  foreach(lib ${libs_and_execs})
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} ${install_name_changes} ${lib})")
  endforeach()

  install(FILES qt.conf DESTINATION "OpenView.app/Contents/Resources")
endif(APPLE)

include(CPack)
