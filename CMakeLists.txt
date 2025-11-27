cmake_minimum_required(VERSION 3.14)
project(Project VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Option to control building tests (CLion enables tests via CTest)
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_GUI "Build GUI application" ON)

# Add Catch2 using FetchContent only when building tests
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v3.4.0
    )
    # Maak Catch2 beschikbaar. Dit creëert targets zoals Catch2::Catch2 en Catch2::Catch2Main.
    FetchContent_MakeAvailable(Catch2)
endif()

# Add GUI dependencies if enabled
if(BUILD_GUI)
    # Download and include imgui
    include(FetchContent)
    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG        v1.89.9
    )

    # Download and include glfw
    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG        3.3.8
    )

    # Make GUI dependencies available
    FetchContent_MakeAvailable(imgui glfw)

    # Find OpenGL package
    find_package(OpenGL REQUIRED)

    # Add imgui library
    set(IMGUI_DIR ${imgui_SOURCE_DIR})
    set(IMGUI_SOURCES
            ${IMGUI_DIR}/imgui.cpp
            ${IMGUI_DIR}/imgui_demo.cpp
            ${IMGUI_DIR}/imgui_draw.cpp
            ${IMGUI_DIR}/imgui_tables.cpp
            ${IMGUI_DIR}/imgui_widgets.cpp
            ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
            ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )

    add_library(imgui_lib STATIC ${IMGUI_SOURCES})
    target_include_directories(imgui_lib PUBLIC
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends
    )

    target_link_libraries(imgui_lib PRIVATE glfw ${OPENGL_LIBRARIES})

    # GUI Application
    add_executable(quadtree_gui
            "src/main_gui.cpp"
    )

    target_include_directories(quadtree_gui PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)
    target_link_libraries(quadtree_gui PRIVATE
            QuadTreeLib
            imgui_lib
            glfw
            ${OPENGL_LIBRARIES}

            opengl32
            gdi32
            user32
            shell32
    )

    # Set C++17 for the GUI target
    set_target_properties(quadtree_gui PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )

    # Copy necessary DLLs on Windows
    if(WIN32)
        add_custom_command(TARGET quadtree_gui POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:glfw>
                $<TARGET_FILE_DIR:quadtree_gui>
        )
    endif()

endif()

# Main library
add_library(QuadTreeLib STATIC
        "src/Quadtree.cpp"
)

target_include_directories(QuadTreeLib PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

# Console application
add_executable(quadtree_app
        "src/main.cpp"
)
target_link_libraries(quadtree_app PRIVATE QuadTreeLib)

if(BUILD_TESTS)
    # Testing setup
    enable_testing()

    # Put runtime targets in a single predictable folder so IDEs (CLion) can find them
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    # Create the test executable
    add_executable(quadtree_tests
            "test/quadtree_tests.cpp"
    )

    # If using embedded Catch2
    if (EXISTS "${PROJECT_SOURCE_DIR}/include/catch2/catch.hpp")
        message(STATUS "Using embedded Catch2 header at include/catch2/catch.hpp")
        target_include_directories(quadtree_tests PRIVATE
                "${PROJECT_SOURCE_DIR}/include"
                "${PROJECT_SOURCE_DIR}/include/catch2"
        )
    endif()

    # Link libraries
    # OPGELOST: Link ALLEEN naar Catch2::Catch2 (de implementatie)
    # De 'main' functie moet in één van de testbestanden staan (bijv. met #define CATCH_CONFIG_MAIN).
    target_link_libraries(quadtree_tests PRIVATE
            QuadTreeLib
            Catch2::Catch2
    )

    # Register the test executable with CTest
    include(CTest)
    add_test(NAME quadtree_tests COMMAND quadtree_tests)
endif()