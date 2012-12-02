
# Server for LMDVVYD

get_target_property(yaml-cpp YAML_INCLUDE INCLUDE_DIR)
include_directories(${YAML_INCLUDE})

# Begin Soures
set(SERVER_SRC "")
list(APPEND SERVER_SRC main.cpp 
../include/set.hpp 
../include/entity.hpp
)

# End Sources
#------------------------------------------------------
# Begin Building
add_executable(lmdvvyd ${SERVER_SRC})
target_link_libraries(lmdvvyd yaml-cpp)
# End Building