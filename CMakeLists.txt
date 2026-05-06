cmake_minimum_required(VERSION 3.16)
project(GALA_hole)

# Find Geant4
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

# Find HDF5
find_package(HDF5 REQUIRED COMPONENTS CXX)
include_directories(${HDF5_INCLUDE_DIRS})

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

# Executable
add_executable(gala_hole gala_hole.cc ${sources} ${headers})
target_link_libraries(gala_hole ${Geant4_LIBRARIES} ${HDF5_CXX_LIBRARIES})

# Copy macros
set(TPC_GALA_SCRIPTS run.mac init_vis.mac vis.mac)
foreach(_script ${TPC_GALA_SCRIPTS})
  configure_file(${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script} COPYONLY)
endforeach()
  
install(TARGETS gala_hole DESTINATION bin)