cmake_minimum_required (VERSION 3.8)
set(VCPKG_TARGET_TRIPLET "x64-windows-static")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_TOOLCHAIN_FILE "C:/Program Files/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Path to toolchain")

project ("NieR Replicant Hardcore mod")

find_package(minhook REQUIRED CONFIG)
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)


add_library(NieR_Replicant_Hardcore SHARED "main.cpp")
target_link_libraries(NieR_Replicant_Hardcore minhook::minhook ${DETOURS_LIBRARY})
target_include_directories(NieR_Replicant_Hardcore PRIVATE ${DETOURS_INCLUDE_DIRS})


add_executable(NieR_Replicant_Hardcore_launcher "launcher.cpp")
target_include_directories(NieR_Replicant_Hardcore_launcher PRIVATE ${DETOURS_INCLUDE_DIRS})
target_link_libraries(NieR_Replicant_Hardcore_launcher PRIVATE ${DETOURS_LIBRARY})




if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NieR_Replicant_Hardcore PROPERTY CXX_STANDARD 20)
  set_property(TARGET NieR_Replicant_Hardcore_launcher PROPERTY CXX_STANDARD 20)
  
endif()

