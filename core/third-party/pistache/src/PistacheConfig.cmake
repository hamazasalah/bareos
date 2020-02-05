

set_and_check ( Pistache_INCLUDE_DIRS "")
include_directories(${Pistache_INCLUDE_DIRS})

set_and_check ( Pistache_LIBRARIES "")
link_directories(${Pistache_LIBRARIES})

include("${CMAKE_CURRENT_LIST_DIR}/PistacheTargets.cmake")
