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
find_package(Threads QUIET)

file(GLOB JME_ERFA_VENDOR_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/erfa/vendor/*.c)

add_library(jme SHARED
    ${JME_ERFA_VENDOR_SOURCES}
    src/api.c
    src/angle.c
    src/astrometry.c
    src/calc.c
    src/context.c
    src/coordinates.c
    src/elp2000.c
    src/elp82b.c
    src/events.c
    src/erfa/dtdb.c
    src/erfa/tdbtcb.c
    src/fixstar.c
    src/houses.c
    src/julian.c
    src/jpl.c
    src/metadata.c
    src/meeus.c
    src/moshier.c
    src/moshier_full.c
    src/moshier_moon.c
    src/moshier/moonlat404.c
    src/moshier/moonlr404.c
    src/sidereal.c
    src/time.c
    src/timescale.c
    src/vsop87.c
)

set_target_properties(jme PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_include_directories(jme
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/moshier
        ${CMAKE_CURRENT_SOURCE_DIR}/src/erfa
        ${CMAKE_CURRENT_SOURCE_DIR}/src/erfa/vendor
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
    set_source_files_properties(
        src/elp82b.c
        src/moshier_full.c
        src/moshier/moonlat404.c
        src/moshier/moonlr404.c
        PROPERTIES COMPILE_FLAGS "/Od"
    )
else()
    target_compile_options(jme PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(jme PUBLIC m)
endif()

if (JME_BUILD_TESTS)
    enable_testing()

    add_executable(test_julian tests/test_julian.c)
    target_link_libraries(test_julian PRIVATE jme)
    add_test(NAME julian COMMAND test_julian)

    add_executable(test_api_contract tests/test_api_contract.c)
    target_link_libraries(test_api_contract PRIVATE jme)
    add_test(NAME api_contract COMMAND test_api_contract)

    add_executable(test_precision_contract tests/test_precision_contract.c)
    target_link_libraries(test_precision_contract PRIVATE jme)
    add_test(NAME precision_contract COMMAND test_precision_contract)

    add_executable(test_time_scale_contract tests/test_time_scale_contract.c)
    target_include_directories(test_time_scale_contract PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/erfa
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_link_libraries(test_time_scale_contract PRIVATE jme)
    add_test(NAME time_scale_contract COMMAND test_time_scale_contract)

    add_executable(test_delta_t_reference tests/test_delta_t_reference.c)
    target_include_directories(test_delta_t_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(test_delta_t_reference PRIVATE jme)
    add_test(NAME delta_t_reference COMMAND test_delta_t_reference)

    add_executable(test_vsop87_reference tests/test_vsop87_reference.c)
    target_include_directories(test_vsop87_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(test_vsop87_reference PRIVATE jme)
    add_test(NAME vsop87_reference COMMAND test_vsop87_reference)

    add_executable(test_apparent_reference tests/test_apparent_reference.c)
    target_include_directories(test_apparent_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(test_apparent_reference PRIVATE jme)
    add_test(NAME apparent_reference COMMAND test_apparent_reference)
    set_tests_properties(apparent_reference PROPERTIES SKIP_RETURN_CODE 77)

    add_executable(test_riseset_reference tests/test_riseset_reference.c)
    target_include_directories(test_riseset_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(test_riseset_reference PRIVATE jme)
    add_test(NAME riseset_reference COMMAND test_riseset_reference)
    set_tests_properties(riseset_reference PROPERTIES SKIP_RETURN_CODE 77)

    add_executable(test_star_reference tests/test_star_reference.c)
    target_include_directories(test_star_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(test_star_reference PRIVATE jme)
    add_test(NAME star_reference COMMAND test_star_reference)
    set_tests_properties(star_reference PROPERTIES SKIP_RETURN_CODE 77)

    add_executable(test_occult_reference tests/test_occult_reference.c)
    target_include_directories(test_occult_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(test_occult_reference PRIVATE jme)
    add_test(NAME occult_reference COMMAND test_occult_reference)
    set_tests_properties(occult_reference PROPERTIES SKIP_RETURN_CODE 77)

    add_executable(test_galcent_reference tests/test_galcent_reference.c)
    target_include_directories(test_galcent_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(test_galcent_reference PRIVATE jme)
    add_test(NAME galcent_reference COMMAND test_galcent_reference)

    add_executable(test_houses_reference tests/test_houses_reference.c)
    target_include_directories(test_houses_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(test_houses_reference PRIVATE jme)
    add_test(NAME houses_reference COMMAND test_houses_reference)

    add_executable(test_jpl_runtime tests/test_jpl_runtime.c)
    target_link_libraries(test_jpl_runtime PRIVATE jme)
    add_test(NAME jpl_runtime COMMAND test_jpl_runtime)

    add_executable(test_all_constants tests/test_all_constants.c)
    target_link_libraries(test_all_constants PRIVATE jme)
    add_test(NAME all_constants COMMAND test_all_constants)

    add_executable(test_all_functions tests/test_all_functions.c)
    target_link_libraries(test_all_functions PRIVATE jme)
    add_test(NAME all_functions COMMAND test_all_functions)

    add_executable(test_analytical_validation tests/test_analytical_validation.c)
    target_link_libraries(test_analytical_validation PRIVATE jme)
    add_test(NAME analytical_validation COMMAND test_analytical_validation)

    add_executable(test_extra_contract tests/test_extra_contract.c)
    target_link_libraries(test_extra_contract PRIVATE jme)
    add_test(NAME extra_contract COMMAND test_extra_contract)

    add_executable(test_mapped_adversarial tests/test_mapped_adversarial.c)
    target_link_libraries(test_mapped_adversarial PRIVATE jme)
    if (Threads_FOUND)
        target_link_libraries(test_mapped_adversarial PRIVATE Threads::Threads)
    endif()
    add_test(NAME mapped_adversarial COMMAND test_mapped_adversarial)

    add_executable(test_extra_adversarial tests/test_extra_adversarial.c)
    target_link_libraries(test_extra_adversarial PRIVATE jme)
    if (Threads_FOUND)
        target_link_libraries(test_extra_adversarial PRIVATE Threads::Threads)
    endif()
    add_test(NAME extra_adversarial COMMAND test_extra_adversarial)

    add_executable(jme_call_probe tests/jme_call_probe.c)
    target_link_libraries(jme_call_probe PRIVATE jme)

    find_program(JME_BASH_EXECUTABLE NAMES bash)
    find_program(JME_PYTHON3_EXECUTABLE NAMES python3)
    if (JME_BASH_EXECUTABLE AND JME_PYTHON3_EXECUTABLE AND NOT WIN32)
        add_test(
            NAME engine_matrix
            COMMAND ${JME_BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_engine_matrix.sh
                    --build-dir ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()

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
