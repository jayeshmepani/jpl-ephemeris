# CMake generated Testfile for 
# Source directory: /home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-
# Build directory: /home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[julian]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_julian")
set_tests_properties([=[julian]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;79;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[api_contract]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_api_contract")
set_tests_properties([=[api_contract]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;83;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[jpl_runtime]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_jpl_runtime")
set_tests_properties([=[jpl_runtime]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;87;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[all_constants]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_all_constants")
set_tests_properties([=[all_constants]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;91;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[all_functions]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_all_functions")
set_tests_properties([=[all_functions]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;95;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[analytical_validation]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_analytical_validation")
set_tests_properties([=[analytical_validation]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;99;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[extra_contract]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_extra_contract")
set_tests_properties([=[extra_contract]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;103;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[mapped_adversarial]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_mapped_adversarial")
set_tests_properties([=[mapped_adversarial]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;110;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[extra_adversarial]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_extra_adversarial")
set_tests_properties([=[extra_adversarial]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;117;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[engine_matrix]=] "/bin/bash" "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/tools/run_engine_matrix.sh")
set_tests_properties([=[engine_matrix]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;122;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[api_inventory]=] "/usr/bin/pwsh" "-NoProfile" "-ExecutionPolicy" "Bypass" "-File" "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/tests/test_inventory.ps1")
set_tests_properties([=[api_inventory]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;129;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[symbol_coverage]=] "/usr/bin/pwsh" "-NoProfile" "-ExecutionPolicy" "Bypass" "-File" "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/tests/test_symbol_coverage.ps1")
set_tests_properties([=[symbol_coverage]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;134;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
