# CMakeLists.txt for g4bl in G4beamline 3.0
# Tom Roberts, 6/10/2015
#
#	On all OSs the build is 64 bits with shared libraries.
#	All shared libraries are included in the distribution.
#
project(g4bl)
cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0043 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0100 NEW)

# CMake subtlety: CMAKE_CXX_FLAGS is a string, while LIBS is a list

###
### Geant4 - GEANT4_DIR is the top-level directory (contains bin, lib, ...).
###          GEANT4_PKG is the Cmake package directory for find_package().
###
file(GLOB GEANT4_PKG ${GEANT4_DIR}/lib*/Geant4-*)
if(NOT IS_DIRECTORY ${GEANT4_PKG}/../../lib AND NOT IS_DIRECTORY ${GEANT4_PKG}/../../lib64)
	message(FATAL_ERROR "GEANT4_PKG=${GEANT4_PKG} incorrect or does not exist")
endif()
SET(Geant4_DIR ${GEANT4_PKG})  # find_package() uses this name
if(G4BL_VISUAL)
	find_package(Geant4 REQUIRED vis_all ui_all)
else()
	find_package(Geant4 REQUIRED ui_all)
endif()
include(${Geant4_USE_FILE})
set(LIBS ${LIBS} ${Geant4_LIBRARIES})

# Geant4 clears CMAKE_CXX_FLAGS, so this must come after it
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG4BLVERSION=${G4BLVERSION}")

###
### Qt5
###
if(G4BL_GUI)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG4BL_GUI")
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	cmake_policy(SET CMP0020 NEW)
	cmake_policy(SET CMP0043 NEW)
	#find_package(Qt5Widgets REQUIRED)
	#find_package(Qt5OpenGL REQUIRED)
	find_package(Qt5 COMPONENTS Core Widgets OpenGL REQUIRED)
endif()

###
### Visualization
###
if(G4BL_VISUAL)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG4BL_VISUAL")
endif()

###
### GSL
###
if(G4BL_GSL)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG4BL_GSL")
	set(GSL_DIR "$ENV{GSL_DIR}" CACHE PATH "Directory containing GSL")
	if(NOT IS_DIRECTORY ${GSL_DIR})
		message(FATAL_ERROR "GSL_DIR=${GSL_DIR} does not exist")
	endif()
	message(STATUS "GSL_DIR=${GSL_DIR}")
	include_directories(${GSL_DIR}/include)
	link_directories(${GSL_DIR}/lib)
	if(UNIX)
		set(LIBS ${LIBS} gsl gslcblas)
	else()
		set(LIBS ${LIBS} gsl gslcblas)
	endif()
endif()

###
### FFTW
###
if(G4BL_FFTW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG4BL_FFTW")
	set(FFTW_DIR "$ENV{FFTW_DIR}" CACHE PATH "Directory containing FFTW")
	if(NOT IS_DIRECTORY ${FFTW_DIR})
		message(FATAL_ERROR "FFTW_DIR=${FFTW_DIR} does not exist")
	endif()
	message(STATUS "FFTW_DIR=${FFTW_DIR}")
	include_directories(${FFTW_DIR}/include)
	link_directories(${FFTW_DIR}/lib)
	if(UNIX)
		set(LIBS ${LIBS} fftw3f)
	else()
		set(LIBS ${LIBS} libfftw3f-3)
	endif()
endif()

###
### Root
###
if(G4BL_ROOT)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG4BL_ROOT")
	list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
	find_package(ROOT REQUIRED COMPONENTS RIO)
	include(${ROOT_USE_FILE})
	set(LIBS ${LIBS} ${ROOT_LIBRARIES})
endif()

###
### NO_RPATH - MPI.cmake can set it to avoid setting RPATH
###
set(NO_RPATH OFF)

###
### MPI -- User must edit MPI.cmake for the build machine
###
if(G4BL_MPI)
	if(NOT UNIX)
	    message(FATAL_ERROR "MPI is not supported on this OS (UNIX only)")
	endif()
	include(${CMAKE_SOURCE_DIR}/MPI.cmake)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DG4BL_MPI")
endif()

###
### RPATH -- fails for a static link (so set NO_RPATH to be ON)
###
if(NOT NO_RPATH)
    if(G4BL_ROOT)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		SET(CMAKE_INSTALL_RPATH 
			"@executable_path/../root/lib;@executable_path/../lib")
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		SET(CMAKE_INSTALL_RPATH "$ORIGIN/../root/lib;$ORIGIN/../lib")
	endif()
    else()
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		SET(CMAKE_INSTALL_RPATH "@executable_path/../lib")
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
	endif()
    endif()
endif()

###
### Executable: g4bl
###
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hh ${PROJECT_SOURCE_DIR}/*.h)
include_directories(${PROJECT_SOURCE_DIR})
add_executable(g4bl ${sources} ${headers})
if(G4BL_GUI)
	#qt5_use_modules(g4bl Widgets OpenGL)
	set(LIBS ${LIBS} Qt5::Widgets Qt5::OpenGL)
endif()
target_link_libraries(g4bl ${LIBS})
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# --disable-new-dtags makes it use RPATH, not RUNPATH -- required
	target_link_libraries(g4bl -Wl,--disable-new-dtags -lpthread -lm -ldl)
endif()

###
### Compiler warnings and other options
###
if(UNIX)
	if(APPLE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
	else() # This is Linux
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
	endif()
else() # This is Windows
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP6")
endif()

###
### Installation
###
install(TARGETS g4bl RUNTIME DESTINATION bin)
if(G4BL_ROOT)
    install(DIRECTORY $ENV{ROOTSYS}/ DESTINATION root USE_SOURCE_PERMISSIONS)
endif()

if(UNIX)
    # Geant4 is linked statically
    # Qt libraries will be found by linux_libraries and macosx_libraries.
    # Qt plugins are installed in finalize.
    # All other libraries are either linked statically or found by the
    # *_libraries script.
else() # This is Windows
    file(GLOB GEANT4_DLLS ${GEANT4_DIR}/bin/G4*.dll)
    install(PROGRAMS ${GEANT4_DLLS} DESTINATION bin)
    if(G4BL_GUI)
	execute_process(COMMAND qmake -query QT_INSTALL_BINS
		OUTPUT_VARIABLE QT_BIN_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
	#@# running qmake -query doesn't work, so hard-code the path
        #@set(QT_BIN_DIR C:/Qt/5.12.0/msvc2017/bin)
        install(PROGRAMS ${QT_BIN_DIR}/Qt5Core.dll ${QT_BIN_DIR}/Qt5Gui.dll
		${QT_BIN_DIR}/Qt5Network.dll ${QT_BIN_DIR}/Qt5Widgets.dll
		${QT_BIN_DIR}/Qt5OpenGL.dll
		DESTINATION bin)
    endif()
    if(G4BL_GSL)
    	install(PROGRAMS ${GSL_DIR}/bin/gsl.dll ${GSL_DIR}/bin/gslcblas.dll 
    		DESTINATION bin)
    endif(G4BL_GSL)
    if(G4BL_FFTW)
    	install(PROGRAMS ${FFTW_DIR}/bin/libfftw3f-3.dll DESTINATION bin)
    endif(G4BL_FFTW)
    if(G4BL_ROOT)
    	# For Root 6, must include the entire root directory in the
	# distribution. Moreover, in g4blgui before running g4bl, must set
	# PATH to have g4bl_dir/root/bin before any user-installed Root.
	# This is because Root 6 finds the Root install from the directory
	# containing the libraries, not ROOTSYS.
    endif(G4BL_ROOT)
endif()

