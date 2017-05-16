# **************************************************************************
# * Copyright(c) 1998-2014, ALICE Experiment at CERN, All rights reserved. *
# *                                                                        *
# * Author: The ALICE Off-line Project.                                    *
# * Contributors are mentioned in the code where appropriate.              *
# *                                                                        *
# * Permission to use, copy, modify and distribute this software and its   *
# * documentation strictly for non-commercial purposes is hereby granted   *
# * without fee, provided that the above copyright notice appears in all   *
# * copies and that both the copyright notice and this permission notice   *
# * appear in the supporting documentation. The authors make no claims     *
# * about the suitability of this software for any purpose. It is          *
# * provided "as is" without express or implied warranty.                  *
# **************************************************************************

# Module
set(MODULE O2)
add_definitions(-D_MODULE_="${MODULE}")

# Module include folders
include_directories(${AliRoot_SOURCE_DIR}/${MODULE})

# Additional include directories - alphabetical order except ROOT
include_directories(${ROOT_INCLUDE_DIR}
                    ${AliRoot_SOURCE_DIR}/ANALYSIS/ANALYSIS
                    ${AliRoot_SOURCE_DIR}/ANALYSIS/ANALYSISalice
                    ${AliRoot_SOURCE_DIR}/STEER/STEERBase
                    ${AliRoot_SOURCE_DIR}/STEER/STEER
		    ${AliRoot_SOURCE_DIR}/RAW/dateStream)

# Sources, globbed
#file(GLOB SRCS "*.cxx")
set(SRCS
  ESDEventConverter.cxx
  O2AnalysisManager.cxx
)

# Headers, from sources
string(REPLACE ".cxx" ".h" HDRS "${SRCS}")
set(HDRS
  ${HDRS}
  O2AnalysisTask.h
  O2ESDAnalysisTask.h
)

# Generate the dictionary
# It will create G_ARG1.cxx and G_ARG1.h / ARG1 = function first argument
get_directory_property(incdirs INCLUDE_DIRECTORIES)
generate_dictionary("O2" "${MODULE}LinkDef.h" "${HDRS}" "${incdirs}")

# TODO: set these
set(ROOT_DEPENDENCIES Core EG Geom Gpad Graf3d MathCore Matrix Physics RIO Tree)
set(ALIROOT_DEPENDENCIES ANALYSISalice STEER AOD ESD)

# Generate the ROOT map
# Dependecies
set(LIBDEPS ${ALIROOT_DEPENDENCIES} ${ROOT_DEPENDENCIES})
generate_rootmap("O2" "${LIBDEPS}" "${AliRoot_SOURCE_DIR}/${MODULE}LinkDef.h")

# Generate a PARfile target for this library
add_target_parfile(${MODULE} "${SRCS}" "${HDRS}" "${MODULE}LinkDef.h" "${LIBDEPS}")

# Create an object to be reused in case of static libraries
# Otherwise the sources will be compiled twice
add_library(${MODULE}-object OBJECT ${SRCS} G__${MODULE}.cxx)

# Add a library to the project using the object
add_library(${MODULE} SHARED $<TARGET_OBJECTS:O2-object>)
target_link_libraries(${MODULE} ${ALIROOT_DEPENDENCIES} ${ROOT_DEPENDENCIES})

# Setting the correct headers for the object as gathered from the dependencies
target_include_directories(${MODULE}-object PUBLIC $<TARGET_PROPERTY:${MODULE},INCLUDE_DIRECTORIES>)
set_target_properties(${MODULE}-object PROPERTIES COMPILE_DEFINITIONS $<TARGET_PROPERTY:${MODULE},COMPILE_DEFINITIONS>)

# Public include folders that will be propagated to the dependecies
target_include_directories(${MODULE} PUBLIC ${incdirs})

# Additional compilation flags for the object
#set_target_properties(O2-object PROPERTIES COMPILE_FLAGS "-fopenmp")

# System dependent: Modify the way the library is build
#if(${CMAKE_SYSTEM} MATCHES Darwin)
#    set_target_properties(O2 PROPERTIES LINK_FLAGS "-undefined dynamic_lookup -fopenmp")
#endif(${CMAKE_SYSTEM} MATCHES Darwin)

# Installation
install(TARGETS ${MODULE}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)
install(FILES ${HDRS} DESTINATION include)

# Static version if DA enabled
if(ALIROOT_STATIC)
    add_library(${MODULE}-static STATIC $<TARGET_OBJECTS:O2-object>)
    set_target_properties(${MODULE}-static PROPERTIES OUTPUT_NAME ${MODULE} "-fopenmp ")

    # list of shared dependencies / the name of the variable containing the list of static ones
    generate_static_dependencies("${ALIROOT_DEPENDENCIES}" "STATIC_ALIROOT_DEPENDENCIES")

    # Public include folders that will be propagated to the dependecies
    target_include_directories(${MODULE}-static PUBLIC ${incdirs})

    set_target_properties(${MODULE}-static PROPERTIES LINK_FLAGS "-Wl,--whole-archive")
    target_link_libraries(${MODULE}-static ${STATIC_ALIROOT_DEPENDENCIES} Root RootExtra)

    # Installation
    install(TARGETS ${MODULE}-static
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib)
endif(ALIROOT_STATIC)

# ECS things
install(DIRECTORY ecs DESTINATION include)
install(DIRECTORY Entities DESTINATION include)
install(DIRECTORY Components DESTINATION include)
