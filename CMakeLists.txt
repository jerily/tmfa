cmake_minimum_required(VERSION 3.22.1)
project(tmfa LANGUAGES CXX C)

set(TARGET tmfa)
set(CMAKE_C_STANDARD   11)
set(CMAKE_C_STANDARD_REQUIRED true)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(libcotp)

add_library(tmfa SHARED library.c)
set_target_properties(tmfa PROPERTIES POSITION_INDEPENDENT_CODE ON)

include(${CMAKE_CURRENT_LIST_DIR}/FindTCL.cmake)

if (NAVISERVER)
    include_directories(libcotp/src/ ${NAVISERVER}/include)
    link_directories(libcotp/build)
    target_link_directories(tmfa PRIVATE ${NAVISERVER}/lib)
    target_link_libraries(tmfa PRIVATE cotp ${TCL_LIBRARY} nsd nsthread)
    target_compile_options(tmfa PRIVATE -DUSE_NAVISERVER)
    install(TARGETS ${TARGET}
            LIBRARY DESTINATION ${NAVISERVER}/bin
    )
else ()
    include_directories(libcotp/src/ ${TCL_INCLUDE_PATH})
    link_directories(libcotp/build)
    target_link_libraries(tmfa PRIVATE cotp ${TCL_LIBRARY})
    get_filename_component(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
    if (TCL_LIBRARY_PATH)
        install(TARGETS ${TARGET}
                LIBRARY DESTINATION ${TCL_LIBRARY_PATH}/${TARGET}
        )
        install(FILES pkgIndex.tcl
                DESTINATION ${TCL_LIBRARY_PATH}/${TARGET}
        )
    endif()
endif()

#add_compile_options(-g -O0 -DDEBUG -Wall -Wextra -Wconversion)
