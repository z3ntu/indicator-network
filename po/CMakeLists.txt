cmake_minimum_required(VERSION 2.8.9)
find_package(Gettext REQUIRED)

set(POT_FILE
  indicator-network.pot
)

file(GLOB PO_FILES
  *.po
)

set(MO_FILE indicator-network.mo)

foreach(_file ${PO_FILES})
  get_filename_component(_lang ${_file} NAME_WE)
  set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
  add_custom_command(OUTPUT ${_gmoFile}
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_file}
    COMMAND install -D ${_gmoFile} ${CMAKE_CURRENT_BINARY_DIR}/locale/${_lang}/LC_MESSAGES/${MO_FILE}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS ${_file}
  )
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo DESTINATION ${CMAKE_INSTALL_PREFIX}/share/locale/${_lang}/LC_MESSAGES/ RENAME ${MO_FILE})
  _gettext_get_unique_target_name( pofiles uniqueTargetName)
  add_custom_target(${uniqueTargetName} ALL DEPENDS ${_gmoFile})
endforeach(_file)

add_custom_command(OUTPUT ${POT_FILE}
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-indicator-network-pot
)
add_custom_target(pot_file
  DEPENDS ${POT_FILE}
)
