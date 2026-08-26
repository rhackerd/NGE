find_path(Stb_INCLUDE_DIR
    NAMES stb_image.h
    PATHS /usr/include /usr/local/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Stb DEFAULT_MSG Stb_INCLUDE_DIR)

if(Stb_FOUND AND NOT TARGET Stb::Stb)
    add_library(Stb::Stb INTERFACE IMPORTED)
    set_target_properties(Stb::Stb PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Stb_INCLUDE_DIR}"
    )
endif()