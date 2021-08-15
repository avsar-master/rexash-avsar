
add_library(filesystem_helper STATIC 
    FileSystem_Helpers.cpp
)

target_include_directories(filesystem_helper PUBLIC ../ ../public)
