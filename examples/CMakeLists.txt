# Use CONFIGURE_DEPENDS so CMake will re-run when files are added/removed
file(GLOB EXAMPLE_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
message(STATUS "Found example sources: ${EXAMPLE_SRCS}")

foreach(src IN LISTS EXAMPLE_SRCS)
  # get filename without extension to use as executable name
  get_filename_component(name ${src} NAME_WE)

  add_executable(${name} ${src})

  # Link the common library (or other libs)
  target_link_libraries(${name} PRIVATE dblib)

  # example-specific include path (if needed)
  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

  # Put example binaries in build/bin/examples (optional)
  # set_target_properties(${name} PROPERTIES
  #   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples
  # )
endforeach()