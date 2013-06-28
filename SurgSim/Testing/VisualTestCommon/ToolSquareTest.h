// This file is a part of the OpenSurgSim project.
// Copyright 2013, SimQuest Solutions Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SURGSIM_TESTING_VISUALTESTCOMMON_TOOLSQUARETEST_H
#define SURGSIM_TESTING_VISUALTESTCOMMON_TOOLSQUARETEST_H

#include <memory>
#include <SurgSim/Input/DeviceInterface.h>

void runToolSquareTest(std::shared_ptr<SurgSim::Input::DeviceInterface> toolDevice,
					   std::shared_ptr<SurgSim::Input::DeviceInterface> squareDevice,
					   const char* testDescriptionMessage);

#endif  // SURGSIM_TESTING_VISUALTESTCOMMON_TOOLSQUARETEST_H
