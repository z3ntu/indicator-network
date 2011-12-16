project(examples)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/run-example.sh.in"
               "${CMAKE_CURRENT_BINARY_DIR}/run-example.sh" @ONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/run-client-app.sh.in"
               "${CMAKE_CURRENT_BINARY_DIR}/run-client-app.sh" @ONLY)

