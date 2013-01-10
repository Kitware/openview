
macro(install_qt_file filename)
  get_filename_component(filename_path ${filename} PATH)
  install(FILES "${qt_path}/${filename}" DESTINATION "OpenView.app/Contents/${filename_path}")
endmacro()

if(APPLE)
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

  foreach(lib ${libs_and_execs})
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} ${install_name_changes} ${lib})")
  endforeach()

  # Use Qt's macdeployqt to fix up the bundle
  install(CODE "execute_process(COMMAND ${qt_path}/bin/macdeployqt ${CMAKE_INSTALL_PREFIX}/OpenView.app)")

endif(APPLE)

include(CPack)
