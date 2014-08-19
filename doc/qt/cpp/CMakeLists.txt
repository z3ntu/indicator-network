add_subdirectory(examples)

set(EXTRACT_PRIVATE NO)
set(INTERNAL_DOCS NO)
set(SOURCE_DIRS "${CMAKE_SOURCE_DIR}/src/qt/include ${CMAKE_CURRENT_SOURCE_DIR}")

if(${PRIVATE_DOCS})
  set(EXTRACT_PRIVATE YES)
  set(INTERNAL_DOCS YES)
  set(SOURCE_DIRS "${SOURCE_DIRS} ${CMAKE_SOURCE_DIR}/src")
endif()

set(DOC_FILES
    Doxyfile.in
    pages/mainpage.dox
    pages/using.dox
    pages/networking-status.dox
)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
    )
    add_custom_target(cppdoc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        SOURCES ${DOC_FILES}
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION ${INSTALL_DOCDIR}/cpp/
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/xml
        DESTINATION ${INSTALL_DOCDIR}/cpp/
endif(DOXYGEN_FOUND)
