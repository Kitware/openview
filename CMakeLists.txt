cmake_minimum_required(VERSION 2.8)

set(files_to_copy "")

macro(copy_file filename)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${filename}
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/${filename}
    ${CMAKE_CURRENT_BINARY_DIR}/${filename}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${filename})
  list(APPEND files_to_copy ${CMAKE_CURRENT_BINARY_DIR}/${filename})
endmacro()

copy_file(main.qml)
copy_file(FileDialog.qml)
copy_file(ViewModel.qml)

add_custom_target(copyfiles ALL DEPENDS ${files_to_copy})

add_subdirectory(OVView)
