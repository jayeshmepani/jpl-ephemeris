# CALCEPH Setup

This project can build without CALCEPH, but real JPL kernel computation requires CALCEPH at build time.

## Windows via vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git E:\project\astrology\tools\vcpkg
E:\project\astrology\tools\vcpkg\bootstrap-vcpkg.bat
E:\project\astrology\tools\vcpkg\vcpkg.exe install calceph:x64-windows
```

Configure this project with CALCEPH required:

```powershell
cmake -S . -B build-vcpkg -DCMAKE_TOOLCHAIN_FILE=E:/project/astrology/tools/vcpkg/scripts/buildsystems/vcpkg.cmake -DJME_REQUIRE_CALCEPH=ON
cmake --build build-vcpkg
```

Run the integration test with a JPL SPK kernel:

```powershell
$env:JME_TEST_JPL_KERNEL = (Resolve-Path data\jpl\de440s.bsp).Path
ctest --test-dir build-vcpkg -C Debug --output-on-failure
```

The local verified setup used CALCEPH `4.0.5` through vcpkg and NASA/JPL `de440s.bsp`.
