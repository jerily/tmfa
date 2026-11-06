# TCL_INCLUDE_DIR and TCL_LIBRARY_DIR select a Tcl installation.
# TCL_INCLUDE_PATH, TCL_STUB_LIBRARY, and TCL_TCLSH can be set explicitly.
find_path(TCL_INCLUDE_PATH NAMES tcl.h
    HINTS ${TCL_INCLUDE_DIR}
    PATH_SUFFIXES tcl9.1 tcl9.0 tcl8.6)

if(EXISTS "${TCL_INCLUDE_PATH}/tcl.h")
    file(STRINGS "${TCL_INCLUDE_PATH}/tcl.h" TCL_PATCH_LEVEL_LINE
        REGEX "^#[ \t]*define[ \t]+TCL_PATCH_LEVEL[ \t]+\"[^\"]+\"")
    string(REGEX REPLACE ".*\"([^\"]+)\".*" "\\1" TCL_VERSION "${TCL_PATCH_LEVEL_LINE}")
endif()
if(TCL_VERSION MATCHES "^([0-9]+)\\.([0-9]+)")
    set(TCL_MAJOR_MINOR "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}")
    set(TCL_COMPACT_VERSION "${CMAKE_MATCH_1}${CMAKE_MATCH_2}")
endif()
get_filename_component(TCL_PREFIX "${TCL_INCLUDE_PATH}" DIRECTORY)
find_library(TCL_STUB_LIBRARY
    NAMES tclstub${TCL_MAJOR_MINOR} tclstub${TCL_COMPACT_VERSION} tclstub
    HINTS ${TCL_LIBRARY_DIR} "${TCL_PREFIX}/lib")
find_program(TCL_TCLSH
    NAMES tclsh${TCL_MAJOR_MINOR} tclsh${TCL_COMPACT_VERSION} tclsh${TCL_COMPACT_VERSION}t tclsh
    HINTS "${TCL_PREFIX}/bin")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TclStub
    REQUIRED_VARS TCL_INCLUDE_PATH TCL_STUB_LIBRARY
    VERSION_VAR TCL_VERSION)
mark_as_advanced(TCL_INCLUDE_PATH TCL_STUB_LIBRARY TCL_TCLSH)
