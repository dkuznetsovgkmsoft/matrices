#########################################################################################
# Copyright © 2023 Dmitry Kuznetsov.                                                    #
#                                                                                       #
# All rights reserved. No part of this software may be reproduced, distributed,         #
# or transmitted in any form or by any means, including photocopying, recording,        #
# or other electronic or mechanical methods, without the prior written permission       #
# of the copyright owner.                                                               #
# Any unauthorized use, reproduction, or distribution of                                #
# this software is strictly prohibited and may # result in severe civil and criminal    #
# penalties.                                                                            #
#                                                                                       #
#########################################################################################

cmake_minimum_required(VERSION 3.15)

# workaround
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_TOOLCHAIN_FILE "./dependencies/vcpkg/scripts/buildsystems/vcpkg.cmake")
    cmake_policy(SET CMP0091 NEW)
endif(WIN32)


project ("matrices_task")

if(WIN32)
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
      set(TASK_ARCH "x64" )
      set(VCPKG_TARGET_TRIPLET "x64-windows-static")
  else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
      set(TASK_ARCH "x86" )
      set(VCPKG_TARGET_TRIPLET "x86-windows-static")
  endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
endif(WIN32)

if (WIN32)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${matrices_task_SOURCE_DIR}/lib/${TASK_ARCH}_$<CONFIG>)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${matrices_task_SOURCE_DIR}/lib/${TASK_ARCH}_$<CONFIG>)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${matrices_task_SOURCE_DIR}/bin/${TASK_ARCH}_$<CONFIG>)
else(WIN32)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${matrices_task_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${matrices_task_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${matrices_task_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

FIND_PACKAGE( Boost REQUIRED COMPONENTS program_options )

add_executable (executable 
"main.cpp" 
"program_options.h" 
"utility.h" 
"matrices.h" 
"fixed_matrix.h" 
"dynamic_matrix.h"
"serializer.h"
)

target_link_libraries(executable Boost::program_options)
target_include_directories(executable PRIVATE "${executable_SOURCE_DIR}/src")
set_target_properties(executable PROPERTIES
    CXX_STANDARD 20
)

