# This file is a part of the OpenSurgSim project.
# Copyright 2013, SimQuest Solutions Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

link_directories(
	${Boost_LIBRARY_DIRS}
)

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${OPENTHREADS_INCLUDE_DIR}"
)

set(SOURCES
	ShowScenery.cpp
)

set(HEADERS
)

# Configure the path for the data files
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.txt.in"
	"${CMAKE_CURRENT_BINARY_DIR}/config.txt"
)

surgsim_add_executable(ShowScenery "${SOURCES}" "${HEADERS}")

SET(LIBS
	SurgSimBlocks
	SurgSimGraphics
	${Boost_LIBRARIES}
	${YAML_CPP_LIBRARIES}
)

target_link_libraries(ShowScenery ${LIBS})

# Put ShowScenery into folder "Examples"
set_target_properties(ShowScenery PROPERTIES FOLDER "Examples") 
