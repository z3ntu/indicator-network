project(server)

find_program(VALA_BIN valac)
if (NOT VALA_BIN)
    message(FATAL_ERROR "Vala compiler not found.")
endif()

add_subdirectory(src)
