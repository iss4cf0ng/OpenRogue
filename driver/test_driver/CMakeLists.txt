# test_driver

set(CMAKE_MSVC_MANIFEST OFF)

set(DRIVER_NAME "test_driver")
set(WDK_ROOT "C:/Program Files (x86)/Windows Kits/10")
if(NOT DEFINED WDK_VERSION OR WDK_VERSION STREQUAL "")
    set(WDK_VERSION "10.0.28000.0")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WDK_ARCH "x64")
else()
    set(WDK_ARCH "x86")
endif()

add_executable(${DRIVER_NAME} driver.c)
set_target_properties(${DRIVER_NAME} PROPERTIES
    SUFFIX ".sys"
    MSVC_RUNTIME_LIBRARY ""
)

target_include_directories(${DRIVER_NAME} PRIVATE
    "${WDK_ROOT}/Include/${WDK_VERSION}/km"
    "${WDK_ROOT}/Include/${WDK_VERSION}/shared"
)

target_compile_definitions(${DRIVER_NAME} PRIVATE
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:_WIN64>
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:AMD64>
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:_AMD64_>
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>:_X86_>
    $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},4>:i386>
    STD_CALL
    DEPRECATE_DDK_FUNCTIONS=1
)

target_compile_options(${DRIVER_NAME} PRIVATE
    /kernel
    /GS
    /Gy
    /W4
)

target_link_directories(${DRIVER_NAME} PRIVATE
    "${WDK_ROOT}/Lib/${WDK_VERSION}/km/${WDK_ARCH}"
)

target_link_libraries(${DRIVER_NAME} PRIVATE
    ntoskrnl.lib
    hal.lib
    BufferOverflowFastFailK.lib
)

target_link_options(${DRIVER_NAME} PRIVATE
    /DRIVER
    /MANIFEST:NO
    /ENTRY:DriverEntry
    /NODEFAULTLIB
    /SUBSYSTEM:NATIVE
)