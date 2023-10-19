cmake_minimum_required(VERSION 3.24)
project(distributed_timer VERSION 0.1.0)
enable_language(CXX)

# Set compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ccache setup
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

# CPM cache directory handling
set(CACHE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cache" CACHE STRING "Cache directory for CPM packages and other cached data")
set(NUM_PARALLEL "20" CACHE STRING "Number of parallel threads")
set(OFFLINE_MODE OFF CACHE BOOL "Offline Mode for Air Gap. Leave SOURCE_CACHE_DIR, delete INSTALL_CACHE_DIR, BUILD_CACHE_DIR, and build and reinstall without internet.")
set(BUILD_EXAMPLES ON CACHE BOOL "Build examples (Uses cppzmq, requires libzmq)")
set(BUILD_DOC OFF CACHE BOOL "Build Documentation (Requires doxygen > 1.9.8)")
set(CREATE_PYTHON_BINDINGS ON CACHE BOOL "Build Python Bindings")


message(
    "Running with:
    \t CACHE_DIR - ${CACHE_DIR}
    \t NUM_PARALLEL - ${NUM_PARALLEL}
    \t OFFLINE_MODE - ${OFFLINE_MODE}
    \t BUILD_EXAMPLES - ${BUILD_EXAMPLES}"
)

# Directories for source,install, and build cache
set(SOURCE_CACHE_DIR "${CACHE_DIR}/sources")
set(INSTALL_CACHE_DIR "${CACHE_DIR}/install")
set(BUILD_CACHE_DIR "${CACHE_DIR}/build")

# Ensure we have Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
message(STATUS "Python3_EXECUTABLE: ${Python3_EXECUTABLE}")
message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python3_SITELIB: ${Python3_SITELIB}")

# dependencies
include(cmake/dependencies.cmake)
set(JSON_REPO_URL "https://github.com/nlohmann/json.git")
set(JSON_VERSION "v3.11.2")
set(JSON_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_CACHE_DIR} -DCMAKE_PREFIX_PATH=${INSTALL_CACHE_DIR} -DCMAKE_BUILD_TYPE=Release -DJSON_BuildTests=OFF")
set(ZMQ_REPO_URL "https://github.com/zeromq/libzmq.git")
set(ZMQ_VERSION "v4.3.2")
set(ZMQ_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_CACHE_DIR} -DCMAKE_PREFIX_PATH=${INSTALL_CACHE_DIR} -DCMAKE_BUILD_TYPE=Release -DZMQ_BUILD_TESTS=OFF -DWITH_DOCS=OFF")
set(CPPZMQ_REPO_URL "https://github.com/zeromq/cppzmq.git")
set(CPPZMQ_VERSION "v4.6.0")
set(CPPZMQ_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_CACHE_DIR} -DCMAKE_PREFIX_PATH=${INSTALL_CACHE_DIR} -DCMAKE_BUILD_TYPE=Release -DCPPZMQ_BUILD_TESTS=OFF")


# Get JSON Library 
build_and_install_dependency(
    NAME json
    LIBRARY_FILE ${INSTALL_CACHE_DIR}/include/nlohmann
    CMAKE_ARGS ${JSON_CMAKE_ARGS}
    REPO_URL ${JSON_REPO_URL}
    VERSION ${JSON_VERSION}
)

find_package(nlohmann_json REQUIRED PATHS "${INSTALL_CACHE_DIR}/share/cmake/nlohmann_json/" NO_DEFAULT_PATH)

# Compile examples
set(LIBRARY_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/timer.cpp
)
add_library(${PROJECT_NAME} STATIC ${LIBRARY_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs nlohmann_json::nlohmann_json)
target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)


set(MERGER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/merger.cpp
)
add_executable(${PROJECT_NAME}_merge_tool ${MERGER_FILES})
target_link_libraries(${PROJECT_NAME}_merge_tool PRIVATE stdc++fs nlohmann_json::nlohmann_json)

if(${BUILD_EXAMPLES})
    # Get ZMQ Library
    build_and_install_dependency(
        NAME zmq
        LIBRARY_FILE ${INSTALL_CACHE_DIR}/lib/libzmq.a
        CMAKE_ARGS ${ZMQ_CMAKE_ARGS}
        REPO_URL ${ZMQ_REPO_URL}
        VERSION ${ZMQ_VERSION}
    )
    find_package(ZeroMQ REQUIRED PATHS "${INSTALL_CACHE_DIR}/share/cmake/ZeroMQ/" NO_DEFAULT_PATH)
    # Get CPPZMQ Library 
    build_and_install_dependency(
        NAME cppzmq
        LIBRARY_FILE ${INSTALL_CACHE_DIR}/include/zmq.hpp
        CMAKE_ARGS ${CPPZMQ_CMAKE_ARGS}
        REPO_URL ${CPPZMQ_REPO_URL}
        VERSION ${CPPZMQ_VERSION}
    )
    find_package(cppzmq REQUIRED PATHS "${INSTALL_CACHE_DIR}/share/cmake/cppzmq/" NO_DEFAULT_PATH)

    set(EXAMPLE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/server.cpp
    )
    if(${cppzmq_FOUND} AND ${ZeroMQ_FOUND})
        message(STATUS "zmq Found!")
        message(STATUS "\t ZeroMQ_INCLUDE_DIR: ${ZeroMQ_INCLUDE_DIR}")
        message(STATUS "\t ZeroMQ_LIBRARY: ${ZeroMQ_LIBRARY}")
        message(STATUS "\t ZeroMQ_STATIC_LIBRARY: ${ZeroMQ_STATIC_LIBRARY}")
        message(STATUS "cppzmq Found!")
        message(STATUS "\t cppzmq_INCLUDE_DIR: ${cppzmq_INCLUDE_DIR}")
        message(STATUS "\t cppzmq_LIBRARY: ${cppzmq_LIBRARY}")
        message(STATUS "\t cppzmq_STATIC_LIBRARY: ${cppzmq_STATIC_LIBRARY}")

        foreach(SRC_FILE ${EXAMPLE_FILES})
            get_filename_component(EXE_NAME ${SRC_FILE} NAME_WE)
            add_executable(${PROJECT_NAME}_${EXE_NAME} ${SRC_FILE})
            target_include_directories(${PROJECT_NAME}_${EXE_NAME}
            PUBLIC
                "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}>"
                "$<INSTALL_INTERFACE:include>"
            PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/src"
                "/usr/local/cuda/include"
                ${TORCH_INCLUDE_DIRS}
                "${INSTALL_CACHE_DIR}/include"
                "${INSTALL_CACHE_DIR}/include/torch/csrc/api/include"
        ) 

            target_link_libraries(${PROJECT_NAME}_${EXE_NAME} PRIVATE ${PROJECT_NAME} ${cppzmq_LIBRARY} ${ZeroMQ_STATIC_LIBRARY})
            add_custom_command(TARGET ${PROJECT_NAME}_${EXE_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}_${EXE_NAME}> ${INSTALL_CACHE_DIR}/bin/
            )
            set_target_properties(${PROJECT_NAME}_${EXE_NAME} PROPERTIES CUDA_ARCHITECTURES native)
            install(TARGETS ${PROJECT_NAME}_${EXE_NAME}
                EXPORT ${PROJECT_NAME}Targets
                RUNTIME DESTINATION bin
            )
        endforeach()
    else()
        message(FATAL_ERROR "CPPZMQ or ZMQ not found. Please check paths")
    endif()
endif()


if(${CREATE_PYTHON_BINDINGS})
    # Add pybind11
    find_package(pybind11 REQUIRED PATHS "${Python3_SITELIB}/pybind11/share/cmake/pybind11" NO_DEFAULT_PATH)

    # Add the timer binding to a Python module
    pybind11_add_module(timer_python_module ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/python_bindings.cpp)
    target_link_libraries(timer_python_module PRIVATE ${PROJECT_NAME} nlohmann_json::nlohmann_json)
endif()


if(${BUILD_DOC})
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif()
endif()
