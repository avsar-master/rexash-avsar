add_subdirectory(external)
add_subdirectory(filesystem_helper)
add_subdirectory(engine)

set(CMAKE_CXX_STANDARD 17)

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
    common/commandline.cpp
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

add_library(vgui_controls STATIC
	vgui_controls/AnalogBar.cpp
	vgui_controls/AnimatingImagePanel.cpp
	vgui_controls/AnimationController.cpp
	vgui_controls/BitmapImagePanel.cpp
	vgui_controls/BuildFactoryHelper.cpp
	vgui_controls/BuildGroup.cpp
	vgui_controls/BuildModeDialog.cpp
	vgui_controls/Button.cpp
	vgui_controls/CheckButton.cpp
	vgui_controls/CheckButtonList.cpp
	vgui_controls/CircularProgressBar.cpp
	vgui_controls/ComboBox.cpp
	vgui_controls/consoledialog.cpp
	vgui_controls/ControllerMap.cpp
	vgui_controls/controls.cpp
	vgui_controls/DirectorySelectDialog.cpp
	vgui_controls/Divider.cpp
	vgui_controls/EditablePanel.cpp
	vgui_controls/ExpandButton.cpp
	vgui_controls/FileOpenDialog.cpp
	vgui_controls/FocusNavGroup.cpp
	vgui_controls/Frame.cpp
	vgui_controls/GraphPanel.cpp
	vgui_controls/HTML.cpp
	vgui_controls/Image.cpp
	vgui_controls/ImageList.cpp
	vgui_controls/ImagePanel.cpp
	vgui_controls/InputDialog.cpp
	vgui_controls/KeyRepeat.cpp
	vgui_controls/Label.cpp
	vgui_controls/ListPanel.cpp
	vgui_controls/ListViewPanel.cpp
	vgui_controls/Menu.cpp
	vgui_controls/MenuBar.cpp
	vgui_controls/MenuButton.cpp
	vgui_controls/MenuItem.cpp
	vgui_controls/MessageBox.cpp
	vgui_controls/MessageDialog.cpp
	vgui_controls/Panel.cpp
	vgui_controls/PanelListPanel.cpp
	vgui_controls/ProgressBar.cpp
	vgui_controls/ProgressBox.cpp
	vgui_controls/PropertyDialog.cpp
	vgui_controls/PropertyPage.cpp
	vgui_controls/PropertySheet.cpp
	vgui_controls/QueryBox.cpp
	vgui_controls/RadioButton.cpp
	vgui_controls/RichText.cpp
	vgui_controls/RotatingProgressBar.cpp
	vgui_controls/savedocumentquery.cpp
	vgui_controls/ScrollableEditablePanel.cpp
	vgui_controls/ScrollBar.cpp
	vgui_controls/ScrollBarSlider.cpp
	vgui_controls/SectionedListPanel.cpp
	vgui_controls/Slider.cpp
	vgui_controls/Splitter.cpp
	vgui_controls/subrectimage.cpp
	vgui_controls/TextEntry.cpp
	vgui_controls/TextImage.cpp
	vgui_controls/ToggleButton.cpp
	vgui_controls/Tooltip.cpp
	vgui_controls/ToolWindow.cpp
	vgui_controls/TreeView.cpp
	vgui_controls/TreeViewListControl.cpp
	vgui_controls/URLLabel.cpp
	vgui_controls/vgui_controls.cpp
	vgui_controls/WizardPanel.cpp
	vgui_controls/WizardSubPanel.cpp
)

target_include_directories(vgui_controls PRIVATE 
    ./
    ./public
    ./common
    ./engine
    ./pm_shared
)

target_link_libraries(vgui_controls PRIVATE filesystem_helper tier0 tier1 wrapper)

add_library(wrapper STATIC
    wrapper/deadfilesystem.cpp
    wrapper/deadinput.cpp
    wrapper/deadipanel.cpp
    wrapper/deadscheme.cpp
    wrapper/deadsurface.cpp
)

target_include_directories(wrapper PRIVATE 
    ./
    public
    public/tier1/
    common
    engine
    pm_shared
)

target_link_libraries(wrapper PRIVATE tier0)

if(NOT MSVC)
    target_compile_options(mathlib PRIVATE -Wno-c++11-narrowing)
endif()

target_include_directories(mathlib PUBLIC 
    public/
    public/mathlib/
)

target_link_libraries(tier1 PUBLIC tier0 mathlib)
