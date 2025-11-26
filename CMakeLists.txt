cmake_minimum_required(VERSION 3.14)
project(QuadtreeProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Option to control building tests (CLion enables tests via CTest)
option(BUILD_TESTS "Build unit tests" ON)

# Add Catch2 using FetchContent only when building tests
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
    )

    # This will automatically call add_subdirectory for Catch2
    FetchContent_MakeAvailable(Catch2)
endif()

# Main library
add_library(QuadTreeLib STATIC
    "src/Quadtree.cpp"
)

target_include_directories(QuadTreeLib PUBLIC 
    "${PROJECT_SOURCE_DIR}/include"
)

# Main application
add_executable(quadtree_app
    "src/main.cpp"
)
target_link_libraries(quadtree_app PRIVATE QuadTreeLib)

if(BUILD_TESTS)
  # Testing setup
  enable_testing()

    # Put runtime targets in a single predictable folder so IDEs (CLion) can find them
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    # If the single-header Catch2 is embedded under include/catch2, use it (professor style)
    if (EXISTS "${PROJECT_SOURCE_DIR}/include/catch2/catch.hpp")
        message(STATUS "Using embedded Catch2 header at include/catch2/catch.hpp")

        # Create a test runner that provides CATCH_CONFIG_MAIN (main for tests)
        add_executable(quadtree_tests
            "test/main.cpp"
            "test/quadtree_tests.cpp"
        )

        target_include_directories(quadtree_tests PRIVATE
            "${PROJECT_SOURCE_DIR}/include"
            "${PROJECT_SOURCE_DIR}/include/catch2"
        )

        target_link_libraries(quadtree_tests PRIVATE QuadTreeLib)

        # Register the test executable with CTest so CLion can run it
        include(CTest)
        add_test(NAME quadtree_tests COMMAND quadtree_tests)

    else()
        message(STATUS "No embedded Catch2 header found — falling back to FetchContent-fetched Catch2")

        # Keep previous behavior: Fetch Catch2 and use Catch2::Catch2WithMain
        include(FetchContent)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v3.4.0
        )
        FetchContent_MakeAvailable(Catch2)

        add_executable(quadtree_tests
            "test/quadtree_tests.cpp"
        )

        target_link_libraries(quadtree_tests PRIVATE
            QuadTreeLib
            Catch2::Catch2WithMain
        )

        include(CTest)
        include(Catch)
        catch_discover_tests(quadtree_tests)

    endif()
    # End of testing block
endif()