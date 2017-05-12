cmake_minimum_required(VERSION 3.7.0)
project(FlannCuda)

#----- CUDA -----
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

# Pass options to NVCC
set(CUDA_64_BIT_DEVICE_CODE --ON)
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -use_fast_math")

#----- OpenMP -----
find_package(OpenMP)

if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")	
endif (OPENMP_FOUND)

#----- FLANN -----
find_package(FLANN REQUIRED)


#----- rply -----	
include_directories("C:/Software/rply/include")
link_directories("C:/Software/rply/lib")


# ----- Sources -----
 file(GLOB_RECURSE SOURCES 
	"src/*.cpp" "src/*.c" "src/*.cu"
	"lz4/*.cpp" "lz4/*.c" "lz4/*.cu"
	"include/*.cpp" "include/*.c" "include/*.cu"
	
	)	

cuda_add_executable(${PROJECT_NAME} ${SOURCES})

#----- Include -----
include_directories("include")
include_directories("flann")
include_directories("utils")

# ----- Libraries -----
target_link_libraries(${PROJECT_NAME} "rply.lib")

