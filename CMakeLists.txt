cmake_minimum_required(VERSION 3.14)

project(DarkAgesAIBot VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_PROXY "Build as a proxy DLL" ON)
option(USE_DETOURS "Use Microsoft Detours for hooking" ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pop
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pop/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pop/network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pop/game
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pop/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/detours/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/json/include
)

# Add third-party dependencies
if(USE_DETOURS)
    add_subdirectory(external/detours)
endif()

# Add jsoncpp library
add_subdirectory(external/json)

# Find source files
file(GLOB_RECURSE SOURCES
    "src/pop/*.cpp"
    "src/pop/utils/*.cpp"
    "src/pop/network/*.cpp"
    "src/pop/game/*.cpp"
    "src/pop/ui/*.cpp"
)

# Define the target
if(BUILD_PROXY)
    set(TARGET_NAME "dinput8")  # Change this to match the DLL you want to proxy
    
    # Add proxy-specific sources
    list(APPEND SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pop/proxy/proxy_dllmain.cpp
    )
else()
    set(TARGET_NAME "DarkAgesAIBot")
endif()

# Create the DLL
add_library(${TARGET_NAME} SHARED ${SOURCES})

# Add libraries to link against
target_link_libraries(${TARGET_NAME} PRIVATE
    detours
    jsoncpp_lib
    psapi  # For process information
    winmm  # Windows multimedia
    d3d9   # DirectX 9
    dxguid # DirectX GUIDs
)

# Add preprocessor definitions
if(BUILD_PROXY)
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        BUILD_PROXY 
        PROXY_DLL_NAME="${TARGET_NAME}"
        ORIGINAL_DLL_PATH="original_${TARGET_NAME}.dll"
    )
endif()

# Set module definition file for export functions
if(BUILD_PROXY)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/pop/proxy/${TARGET_NAME}.def")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/src/pop/proxy/${TARGET_NAME}.def\"")
    endif()
endif()

# Installation rules
install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Copy additional files
install(FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION .
)

# Create data directories
install(DIRECTORY 
    DESTINATION bin/config
    DESTINATION bin/data
    DESTINATION bin/logs
)
