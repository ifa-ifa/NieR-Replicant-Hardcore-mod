cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)

set(VCPKG_TARGET_TRIPLET "x64-windows-static-md")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project ("NieR Replicant Hardcore mod")

find_package(minhook REQUIRED CONFIG)
find_package(unofficial-inih REQUIRED CONFIG)

add_library(NieR_Replicant_Hardcore SHARED "main.cpp")
target_link_libraries(NieR_Replicant_Hardcore PRIVATE minhook::minhook unofficial::inih::inireader)

add_executable(NieR_Replicant_Hardcore_launcher "launcher.cpp")