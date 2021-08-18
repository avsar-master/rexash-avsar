add_subdirectory(3rdparty)
add_subdirectory(filesystem_helper)
add_subdirectory(vgui2_support)
add_subdirectory(engine)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
	add_compile_options(/wd4091 /wd4819 /wd4828 /wd5033 /permissive- /utf-8)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	add_compile_options(-Wno-register)
endif()

add_library(vinterface STATIC
    tier1/interface.cpp
)

target_include_directories(vinterface PUBLIC
    public
    public/tier1
)

add_library(vstdlib INTERFACE)
target_include_directories(vstdlib INTERFACE
    public/
    public/vstdlib/
)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(OSX ON)
    add_compile_options(-DOSX -D_OSX)
    set(HL2SDK_LIB_STATIC_EXT ".a")
    set(HL2SDK_LIB_SHARED_EXT ".dylib")
    set(HL2SDK_LIB_PREFIX "lib")
    set(HL2SDK_PATH ".")

    if(HL2SDK_64BIT)
        set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/osx64)
        set(HL2SDK_LIB_STATIC_SUFFIX "")
    else()
        set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/mac)
        set(HL2SDK_LIB_STATIC_SUFFIX "_i486")
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX ON)
    add_compile_options(-D_LINUX -DLINUX)
    set(HL2SDK_LIB_STATIC_EXT ".a")
    set(HL2SDK_LIB_SHARED_EXT ".so")
    set(HL2SDK_LIB_PREFIX "lib")
    set(HL2SDK_PATH ".")

    if(HL2SDK_64BIT)
        set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/linux64)
        set(HL2SDK_LIB_STATIC_SUFFIX "")
    else()
        set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/linux)
        set(HL2SDK_LIB_STATIC_SUFFIX "_i486")
    endif()
elseif(WIN32)
    set(HL2SDK_PATH ".")
    set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/public)
    set(HL2SDK_LIB_STATIC_EXT ".lib")
    set(HL2SDK_LIB_SHARED_EXT ".dll")
    set(HL2SDK_LIB_STATIC_SUFFIX "")
endif()

target_link_libraries(vstdlib INTERFACE vstdlib${HL2SDK_LIB_STATIC_SUFFIX}${HL2SDK_LIB_STATIC_EXT})
target_link_directories(vstdlib INTERFACE ${HL2SDK_LIB_DIR})

add_library(tier0_h INTERFACE)

target_include_directories(tier0_h INTERFACE
       public/
       public/tier0/
)

target_include_directories(tier0_h INTERFACE
    ./
)

add_library(tier0_src INTERFACE)

target_sources(tier0_src INTERFACE
    tier0/assert_dialog.cpp
    tier0/cpu.cpp
    tier0/dbg.cpp
    tier0/fasttimer.cpp
    tier0/mem.cpp
    tier0/memstd.cpp
    tier0/memdbg.cpp
    tier0/mem_helpers.cpp
    tier0/memblockhdr.cpp
    tier0/memvalidate.cpp
    tier0/minidump.cpp
    tier0/platform.cpp
    tier0/pme.cpp
    tier0/threadtools.cpp
    tier0/testthread.cpp
    tier0/vcrmode.cpp
    commandline.cpp
)

if( POSIX )
    target_sources(tier0_src INTERFACE tier0/cpu_posix.cpp)
    target_sources(tier0_src INTERFACE tier0/platform_posix.cpp)
    target_sources(tier0_src INTERFACE tier0/pme_posix.cpp)
    target_sources(tier0_src INTERFACE tier0/vcrmode_posix.cpp)
endif()

target_link_libraries(tier0_src INTERFACE tier0_h)

add_library(tier0_static STATIC)
target_link_libraries(tier0_static PRIVATE tier0_src)
target_link_libraries(tier0_static PUBLIC tier0_h)
target_compile_definitions(tier0_static PUBLIC
    -DSTATIC_TIER0
)

if( WIN32 )
    target_link_libraries(tier0_static PUBLIC ws2_32)
endif()

add_library(tier0 ALIAS tier0_static)


add_library(tier1 STATIC
    tier1/bitbuf.cpp
    tier1/byteswap.cpp
    tier1/characterset.cpp
    tier1/checksum_crc.cpp
    tier1/checksum_md5.cpp
    tier1/convar.cpp
    tier1/datamanager.cpp
    tier1/diff.cpp
    tier1/generichash.cpp
    tier1/KeyValues.cpp
    tier1/mempool.cpp
    tier1/memstack.cpp
    tier1/NetAdr.cpp
    tier1/processor_detect.cpp
    tier1/rangecheckedvar.cpp
    tier1/stringpool.cpp
    tier1/strtools.cpp
    tier1/tier1.cpp
    tier1/utlbuffer.cpp
    tier1/utlstring.cpp
    tier1/utlsymbol.cpp
)

target_include_directories(tier1 PUBLIC
    public/
    public/tier1/
)

add_library(mathlib STATIC
    mathlib/3dnow.cpp
    mathlib/mathlib_base.cpp
    mathlib/sse.cpp
)

if(NOT MSVC)
    target_compile_options(mathlib PRIVATE -Wno-c++11-narrowing)
endif()

target_include_directories(mathlib PUBLIC 
    public/
    public/mathlib/
)

target_link_libraries(tier1 PUBLIC vinterface tier0 mathlib)
