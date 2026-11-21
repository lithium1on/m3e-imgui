cmake_minimum_required(VERSION 3.20)
project(m3e_imgui VERSION 0.1.0 LANGUAGES CXX)

option(M3E_BUILD_EXAMPLE "Build the renderer-free integration example" ON)
option(M3E_BUILD_GALLERY "Build the standalone Win32/DX11 gallery" ${WIN32})
option(M3E_BUILD_TESTS "Build the renderer-free interaction checks" OFF)
set(M3E_IMGUI_TARGET "" CACHE STRING "Existing Dear ImGui CMake target (must provide ImGui 1.92.5)")
set(M3E_IMGUI_SOURCE_DIR "" CACHE PATH "Dear ImGui source tree (also needed for the gallery backends)")

if(NOT M3E_IMGUI_TARGET)
    if(NOT M3E_IMGUI_SOURCE_DIR)
        include(FetchContent)
        FetchContent_Declare(dear_imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG 6d910d5487d11ca567b61c7824b0c78c569d62f0)
        FetchContent_MakeAvailable(dear_imgui)
        set(M3E_IMGUI_SOURCE_DIR "${dear_imgui_SOURCE_DIR}")
    endif()
    add_library(m3e_dear_imgui STATIC)
    target_sources(m3e_dear_imgui PRIVATE
        "${M3E_IMGUI_SOURCE_DIR}/imgui.cpp"
        "${M3E_IMGUI_SOURCE_DIR}/imgui_draw.cpp"
        "${M3E_IMGUI_SOURCE_DIR}/imgui_tables.cpp"
        "${M3E_IMGUI_SOURCE_DIR}/imgui_widgets.cpp")
    target_include_directories(m3e_dear_imgui PUBLIC "${M3E_IMGUI_SOURCE_DIR}")
    target_compile_features(m3e_dear_imgui PUBLIC cxx_std_17)
    set_target_properties(m3e_dear_imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set(M3E_IMGUI_TARGET m3e_dear_imgui)
endif()

add_library(m3e_imgui STATIC src/draw.cpp src/controls.cpp src/overlays.cpp src/android.cpp src/imgui_bridge.cpp)
target_sources(m3e_imgui PRIVATE src/components_display.cpp src/components_selection.cpp
    src/components_pickers.cpp src/components_layout.cpp src/components_actions.cpp src/components_android.cpp)
add_library(m3e::imgui ALIAS m3e_imgui)
target_compile_features(m3e_imgui PUBLIC cxx_std_17)
target_include_directories(m3e_imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(m3e_imgui PUBLIC ${M3E_IMGUI_TARGET})
set_target_properties(m3e_imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(M3E_BUILD_EXAMPLE)
    add_executable(m3e_example examples/minimal.cpp)
    target_link_libraries(m3e_example PRIVATE m3e::imgui)
endif()

if(M3E_BUILD_TESTS)
    enable_testing()
    add_executable(m3e_interactions tests/interactions.cpp)
    target_link_libraries(m3e_interactions PRIVATE m3e::imgui)
    add_test(NAME m3e_interactions COMMAND m3e_interactions "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts")
endif()

if(M3E_BUILD_GALLERY)
    if(NOT WIN32 OR NOT EXISTS "${M3E_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx11.cpp")
        message(FATAL_ERROR "The gallery requires Windows and a Dear ImGui source tree")
    endif()
    add_executable(m3e_gallery examples/gallery.cpp examples/catalog.cpp examples/win32_dx11.cpp
        "${M3E_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx11.cpp"
        "${M3E_IMGUI_SOURCE_DIR}/backends/imgui_impl_win32.cpp")
    target_include_directories(m3e_gallery PRIVATE "${M3E_IMGUI_SOURCE_DIR}/backends")
    target_compile_definitions(m3e_gallery PRIVATE NOMINMAX UNICODE _UNICODE)
    target_link_libraries(m3e_gallery PRIVATE m3e::imgui d3d11 dxgi user32)
    add_custom_command(TARGET m3e_gallery POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts" "$<TARGET_FILE_DIR:m3e_gallery>/fonts"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/licenses" "$<TARGET_FILE_DIR:m3e_gallery>/licenses")
endif()
