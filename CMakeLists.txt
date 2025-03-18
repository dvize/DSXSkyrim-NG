cmake_minimum_required(VERSION 3.27)

project(
        DSXSkyrimNG
        VERSION 1.7.0
        LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_compile_definitions(SKYRIM)
set(CommonLibPath "extern/CommonLibSSE-NG")
set(CommonLibName "CommonLibSSE")

add_library(
        "${PROJECT_NAME}"
        SHARED
        ${CMAKE_CURRENT_BINARY_DIR}/include/Plugin.h
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

target_compile_features(
        "${PROJECT_NAME}"
        PRIVATE
        cxx_std_23
)

include(AddCXXFiles)
add_cxx_files("${PROJECT_NAME}")

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Plugin.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/Plugin.h
        @ONLY
)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY
)

target_sources(
        "${PROJECT_NAME}"
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/include/Plugin.h
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

target_precompile_headers(
        "${PROJECT_NAME}"
        PRIVATE
        src/PCH.h
)

add_compile_definitions(
        SKSE_SUPPORT_XBYAK
)

if(MSVC)
    if(NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
        add_compile_options(
                /MP # Build with Multiple Processes
        )
    endif()
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    add_compile_definitions(_UNICODE)

    target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

    set(SC_RELEASE_OPTS "/Zi;/fp:fast;/GL;/Gy-;/Gm-;/Gw;/sdl-;/GS-;/guard:cf-;/O2;/Ob2;/Oi;/Ot;/Oy;/fp:except-")

    target_compile_options(
            "${PROJECT_NAME}"
            PRIVATE
            /MP
            /W4
            /WX
            /permissive-
            /Zc:alignedNew
            /Zc:auto
            /Zc:__cplusplus
            /Zc:externC
            /Zc:externConstexpr
            /Zc:forScope
            /Zc:hiddenFriend
            /Zc:implicitNoexcept
            /Zc:lambda
            /Zc:noexceptTypes
            /Zc:preprocessor
            /Zc:referenceBinding
            /Zc:rvalueCast
            /Zc:sizedDealloc
            /Zc:strictStrings
            /Zc:ternary
            /Zc:threadSafeInit
            /Zc:trigraphs
            /Zc:wchar_t
            /wd4200 # nonstandard extension used : zero-sized array in struct/union
            /arch:AVX
    )

    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/fp:strict>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/ZI>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/Od>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/Gy>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${SC_RELEASE_OPTS}>")

    target_link_options(
            ${PROJECT_NAME}
            PRIVATE
            /WX
            "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
            "$<$<CONFIG:RELEASE>:/LTCG;/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
    )
endif()

add_subdirectory(${CommonLibPath} ${CommonLibName} EXCLUDE_FROM_ALL)

find_package(spdlog CONFIG REQUIRED)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
        CommonLibSSE::CommonLibSSE
)

find_package(xbyak CONFIG REQUIRED)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        xbyak::xbyak
)

# https://gitlab.kitware.com/cmake/cmake/-/issues/24922#note_1371990
if( MSVC_VERSION GREATER_EQUAL 1936 AND MSVC_IDE ) # 17.6+
    # When using /std:c++latest, "Build ISO C++23 Standard Library Modules" defaults to "Yes".
    # Default to "No" instead.
    #
    # As of CMake 3.26.4, there isn't a way to control this property
    # (https://gitlab.kitware.com/cmake/cmake/-/issues/24922),
    # We'll use the MSBuild project system instead
    # (https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-file-structure)
    file( CONFIGURE OUTPUT "${CMAKE_BINARY_DIR}/Directory.Build.props" CONTENT [==[
<Project>
  <ItemDefinitionGroup>
    <ClCompile>
      <BuildStlModules>false</BuildStlModules>
    </ClCompile>
  </ItemDefinitionGroup>
</Project>
]==] @ONLY )
endif()

