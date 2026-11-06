cmake_minimum_required(VERSION 3.22.1)
project(tmfa VERSION 1.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
option(BUILD_SHARED_LIBS "Build a loadable Tcl extension" ON)
find_package(TclStub 8.6 REQUIRED)
find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)

add_library(tmfa library.c)
set_target_properties(tmfa PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    INSTALL_RPATH_USE_LINK_PATH ON
    PREFIX "lib")
target_include_directories(tmfa PRIVATE ${TCL_INCLUDE_PATH})
target_compile_definitions(tmfa PRIVATE TCL_THREADS PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(tmfa PRIVATE OpenSSL::Crypto)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(tmfa PRIVATE USE_TCL_STUBS)
    target_link_libraries(tmfa PRIVATE ${TCL_STUB_LIBRARY})
else()
    target_compile_definitions(tmfa PRIVATE STATIC_BUILD)
endif()

install(TARGETS tmfa
    LIBRARY DESTINATION lib/tmfa${PROJECT_VERSION}
    ARCHIVE DESTINATION lib/tmfa${PROJECT_VERSION}
    RUNTIME DESTINATION lib/tmfa${PROJECT_VERSION})
install(FILES library.h DESTINATION include/tmfa)
if(BUILD_SHARED_LIBS)
    configure_file(pkgIndex.tcl.in pkgIndex.tcl @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl DESTINATION lib/tmfa${PROJECT_VERSION})
endif()

include(CTest)
if(BUILD_TESTING AND BUILD_SHARED_LIBS)
    if(NOT TCL_TCLSH)
        message(FATAL_ERROR "Set TCL_TCLSH to run tests or disable BUILD_TESTING")
    endif()
    add_test(NAME tmfa COMMAND ${TCL_TCLSH}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/all.tcl $<TARGET_FILE:tmfa>)
endif()
