cmake_minimum_required (VERSION 3.8)
set(VCPKG_TARGET_TRIPLET "x64-windows-static")
#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:YES")

set(CMAKE_TOOLCHAIN_FILE "C:/Program Files/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Path to toolchain")

project ("NieR Replicant Hardcore mod")

find_package(minhook REQUIRED CONFIG)



add_library(NieR_Replicant_Hardcore SHARED "main.cpp")
target_link_libraries(NieR_Replicant_Hardcore minhook::minhook )
add_executable(NieR_Replicant_Hardcore_launcher "launcher.cpp")


set_property(TARGET NieR_Replicant_Hardcore PROPERTY CXX_STANDARD 20)
set_property(TARGET NieR_Replicant_Hardcore_launcher PROPERTY CXX_STANDARD 20)



