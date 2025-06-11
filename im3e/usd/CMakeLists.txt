if (NOT TARGET hdx)
    message(STATUS "OpenUSD not found, skipping targets that depend on it.")
    return()
endif()

add_subdirectory(test)