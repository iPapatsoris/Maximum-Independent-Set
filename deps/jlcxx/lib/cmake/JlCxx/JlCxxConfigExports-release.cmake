#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "JlCxx::cxxwrap_julia" for configuration "Release"
set_property(TARGET JlCxx::cxxwrap_julia APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(JlCxx::cxxwrap_julia PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcxxwrap_julia.so.0.12.2"
  IMPORTED_SONAME_RELEASE "libcxxwrap_julia.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS JlCxx::cxxwrap_julia )
list(APPEND _IMPORT_CHECK_FILES_FOR_JlCxx::cxxwrap_julia "${_IMPORT_PREFIX}/lib/libcxxwrap_julia.so.0.12.2" )

# Import target "JlCxx::cxxwrap_julia_stl" for configuration "Release"
set_property(TARGET JlCxx::cxxwrap_julia_stl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(JlCxx::cxxwrap_julia_stl PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcxxwrap_julia_stl.so"
  IMPORTED_SONAME_RELEASE "libcxxwrap_julia_stl.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS JlCxx::cxxwrap_julia_stl )
list(APPEND _IMPORT_CHECK_FILES_FOR_JlCxx::cxxwrap_julia_stl "${_IMPORT_PREFIX}/lib/libcxxwrap_julia_stl.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
