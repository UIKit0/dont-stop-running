FIND_PATH(SDL2_TTF_INCLUDE_DIR SDL_ttf.h HINTS $ENV{SDL2TTF}/i686-w64-mingw32 PATH_SUFFIXES include/SDL2)

FIND_LIBRARY(SDL2_TTF_LIBRARY SDL2_ttf HINTS $ENV{SDL2TTF}/i686-w64-mingw32 PATH_SUFFIXES lib)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_TTF REQUIRED_VARS SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)