# Local patch: CMake 4.x removed the OLD behavior of CMP0048, which the
# pinned capstone revision still selects (guarded by `if (POLICY CMP0048)`,
# so the guard no longer protects anything). Rewrite it to NEW in the fetched
# source tree before the configure. CMP0048 NEW only requires project() to
# declare VERSION explicitly, which this capstone does not use — safe.
#
# Invoked as FetchContent's PATCH_COMMAND:
#   cmake -DCAPSTONE_DIR=<SOURCE_DIR> -P cmake4_compat.cmake
#
if ( NOT EXISTS "${CAPSTONE_DIR}/CMakeLists.txt" )
	message( FATAL_ERROR "capstone source dir not found: ${CAPSTONE_DIR}" )
endif ( )
file( READ "${CAPSTONE_DIR}/CMakeLists.txt" _capstone_cml )
string( REPLACE "cmake_policy (SET CMP0048 OLD)" "cmake_policy (SET CMP0048 NEW)" _capstone_cml "${_capstone_cml}" )
file( WRITE "${CAPSTONE_DIR}/CMakeLists.txt" "${_capstone_cml}" )
