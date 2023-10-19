function(build_and_install_dependency)
    # Parse function arguments
    set(options "")
    set(oneValueArgs NAME LIBRARY_FILE CMAKE_ARGS REPO_URL VERSION)
    set(multiValueArgs "")
    cmake_parse_arguments(PARSE_ARGV 0 DEP "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT (EXISTS ${DEP_LIBRARY_FILE}))
        # Determine offline mode argument
        set(OFFLINE_MODE_ARG "")
        if(OFFLINE_MODE)
            set(OFFLINE_MODE_ARG "--offline_mode")
        endif()

        # Building dependency
        execute_process(
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/dependencies.py build ${DEP_NAME} ${DEP_REPO_URL} ${DEP_VERSION} ${SOURCE_CACHE_DIR} ${BUILD_CACHE_DIR} ${INSTALL_CACHE_DIR} ${NUM_PARALLEL} --recurse_submodules --cmake_args="${DEP_CMAKE_ARGS}" ${OFFLINE_MODE_ARG}
            RESULT_VARIABLE build_result
        )
        if(build_result)
            message(FATAL_ERROR "Building ${DEP_NAME} failed!")
        endif()

        # Installing dependency
        execute_process(
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/dependencies.py install ${DEP_NAME} ${BUILD_CACHE_DIR} ${NUM_PARALLEL}
            RESULT_VARIABLE install_result
        )
        if(install_result)
            message(FATAL_ERROR "Installing ${DEP_NAME} failed!")
        endif()
    endif()
endfunction()
