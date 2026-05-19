# CMake generated Testfile for 
# Source directory: /home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-
# Build directory: /home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[julian]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_julian")
set_tests_properties([=[julian]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;66;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[api_contract]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_api_contract")
set_tests_properties([=[api_contract]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;70;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[jpl_runtime]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_jpl_runtime")
set_tests_properties([=[jpl_runtime]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;74;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[all_constants]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_all_constants")
set_tests_properties([=[all_constants]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;78;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[all_functions]=] "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/test_all_functions")
set_tests_properties([=[all_functions]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;82;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[api_inventory]=] "/usr/bin/pwsh" "-NoProfile" "-ExecutionPolicy" "Bypass" "-File" "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/tests/test_inventory.ps1")
set_tests_properties([=[api_inventory]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;86;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
add_test([=[symbol_coverage]=] "/usr/bin/pwsh" "-NoProfile" "-ExecutionPolicy" "Bypass" "-File" "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/tests/test_symbol_coverage.ps1")
set_tests_properties([=[symbol_coverage]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;91;add_test;/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/CMakeLists.txt;0;")
