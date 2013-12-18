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

#include "SurgSim/Devices/TrackIR/TrackIRScaffold.h"
#include "SurgSim/Devices/TrackIR/TrackIRThread.h"

namespace SurgSim
{
namespace Device
{

TrackIRThread::TrackIRThread(TrackIRScaffold* scaffold, TrackIRScaffold::DeviceData* deviceData) :
	BasicThread("TrackIR thread"),
	m_scaffold(scaffold),
	m_deviceData(deviceData)
{
	// TrackIR runs at 120FPS = 240Hz
	// Set scaffold update rate to 60Hz/30FPS
	setRate(60.0);
}

TrackIRThread::~TrackIRThread()
{
}

bool TrackIRThread::doInitialize()
{
	return true;
}

bool TrackIRThread::doStartUp()
{
	return true;
}

bool TrackIRThread::doUpdate(double dt)
{
	return m_scaffold->runInputFrame(m_deviceData);
}

};  // namespace Device
};  // namespace SurgSim
