cmake_minimum_required(VERSION 3.20)

project(jme
    VERSION 0.1.0
    DESCRIPTION "Independent JPL/Moshier ephemeris engine in C"
    LANGUAGES C
)

option(JME_BUILD_TESTS "Build C test programs" ON)
option(JME_REQUIRE_CALCEPH "Fail configure when CALCEPH is not found" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(calceph CONFIG QUIET)

add_library(jme
    src/api.c
    src/angle.c
    src/astrometry.c
    src/calc.c
    src/context.c
    src/coordinates.c
    src/events.c
    src/fixstar.c
    src/houses.c
    src/julian.c
    src/jpl.c
    src/metadata.c
    src/sidereal.c
    src/time.c
)

target_include_directories(jme
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (calceph_FOUND)
    message(STATUS "CALCEPH support: enabled")
    target_compile_definitions(jme PRIVATE JME_HAVE_CALCEPH=1)
    target_link_libraries(jme PRIVATE calceph)
else()
    if (JME_REQUIRE_CALCEPH)
        message(FATAL_ERROR "CALCEPH support was requested but CALCEPH was not found")
    endif()
    message(STATUS "CALCEPH support: disabled")
endif()

if (MSVC)
    target_compile_options(jme PRIVATE /W4)
else()
    target_compile_options(jme PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(jme PRIVATE m)
endif()

if (JME_BUILD_TESTS)
    enable_testing()

    add_executable(test_julian tests/test_julian.c)
    target_link_libraries(test_julian PRIVATE jme)
    add_test(NAME julian COMMAND test_julian)

    add_executable(test_api_contract tests/test_api_contract.c)
    target_link_libraries(test_api_contract PRIVATE jme)
    add_test(NAME api_contract COMMAND test_api_contract)

    add_executable(test_jpl_runtime tests/test_jpl_runtime.c)
    target_link_libraries(test_jpl_runtime PRIVATE jme)
    add_test(NAME jpl_runtime COMMAND test_jpl_runtime)

    add_executable(test_all_constants tests/test_all_constants.c)
    target_link_libraries(test_all_constants PRIVATE jme)
    add_test(NAME all_constants COMMAND test_all_constants)

    add_executable(test_all_functions tests/test_all_functions.c)
    target_link_libraries(test_all_functions PRIVATE jme)
    add_test(NAME all_functions COMMAND test_all_functions)

    find_program(PWSH_EXECUTABLE NAMES pwsh powershell)
    if (PWSH_EXECUTABLE)
        add_test(
            NAME api_inventory
            COMMAND ${PWSH_EXECUTABLE} -NoProfile -ExecutionPolicy Bypass
                    -File ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_inventory.ps1
        )
        add_test(
            NAME symbol_coverage
            COMMAND ${PWSH_EXECUTABLE} -NoProfile -ExecutionPolicy Bypass
                    -File ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_symbol_coverage.ps1
        )
    endif()
endif()
