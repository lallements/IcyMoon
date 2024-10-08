if (NOT(WIN32) AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR GENERATOR_IS_MULTI_CONFIG))
    include(code_coverage/src/code_coverage.cmake)
endif()

macro(im3e_set_coverage_flags)
    option(TEST_COVERAGE "Enables test coverage targets im3e_test_coverage and im3e_unit_test_coverage")

    # Coverage compiler flags to be added BEFORE creating targets
    if(TEST_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
        append_coverage_compiler_flags()
    endif()
endmacro()

function(im3e_init_code_coverage)
    if(TEST_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
        get_property(COVERAGE_EXCLUDES GLOBAL PROPERTY COVERAGE_EXCLUDES_PROPERTY)
        set(COVERAGE_EXCLUDES 
            ${COVERAGE_EXCLUDES}
            "~/.conan/*"
            "~/.conan2/*"
            "/opt/*"
            "/usr/*"
            "build/*"
            "applications/*"
            "external/*"
            "resources/*"
        )

        setup_target_for_coverage_lcov(
        NAME 
            im3e_test_coverage
        EXECUTABLE 
            ctest
        DEPENDENCIES 
            all
        EXCLUDE
            ${COVERAGE_EXCLUDES}
        )

        setup_target_for_coverage_lcov(
        NAME 
            im3e_unit_test_coverage
        EXECUTABLE 
            ctest -L "unit_test"
        DEPENDENCIES
            all
        EXCLUDE
            ${COVERAGE_EXCLUDES}
        )
    endif()
endfunction()
