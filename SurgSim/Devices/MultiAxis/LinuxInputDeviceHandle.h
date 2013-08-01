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

#ifndef SURGSIM_DEVICES_MULTIAXIS_LINUXINPUTDEVICEHANDLE_H
#define SURGSIM_DEVICES_MULTIAXIS_LINUXINPUTDEVICEHANDLE_H

#include <string>
#include <memory>
#include <array>
#include <vector>

#include <SurgSim/Devices/MultiAxis/SystemInputDeviceHandle.h>


namespace SurgSim
{
namespace Device
{

/// A wrapper for system-dependent access to a input/HID device.
class LinuxInputDeviceHandle : public SystemInputDeviceHandle
{
public:
	/// Destructor.
	~LinuxInputDeviceHandle();

	/// Enumerates input devices.
	/// \param logger	The logger to be used during enumeration.
	/// \return	A list of device paths.
	static std::vector<std::string> enumerate(SurgSim::Framework::Logger* logger);

	/// Opens the given path and creates an access wrapper for the device.
	/// \param	path	Full pathname for the device.
	/// \param logger	The logger to be used by the device.
	/// \return	The created device object, or an empty unique_ptr on failure.
	static std::unique_ptr<LinuxInputDeviceHandle> open(const std::string& path,
		std::shared_ptr<SurgSim::Framework::Logger> logger);

	virtual bool updateStates(AxisStates* axisStates, ButtonStates* buttonStates, bool* updated) override;

	/// Determines if the file handle can be read from.
	/// \return	true if the handle has been open for reading.
	virtual bool canRead() const override;

	/// Determines if the file handle can be written to.
	/// \return	true if the handle has been open for writing.
	virtual bool canWrite() const override;

	/// Checks whether this object has data available to be read.
	/// \return	true if there is data currently available.
	virtual bool hasDataToRead() const override;

	/// Reads bytes from the file handle.
	/// \param [out]	dataBuffer	Buffer to read into.  Must have room for at least bytesToRead bytes of data.
	/// \param	bytesToRead	The number of bytes to try reading.  Actual number of bytes received may be smaller.
	/// \param [out]	bytesActuallyRead	The number of bytes that were actually read into the buffer.
	/// \return	true if it succeeds, false if it fails.
	virtual bool readBytes(void* dataBuffer, size_t bytesToRead, size_t* bytesActuallyRead) override;

	// XXX HORRIBLE HACK!!!
#ifndef HID_WINDDK_XXX
	virtual int get() const override;
#else /* HID_WINDDK_XXX */
	virtual void* get() const override;
#endif /* HID_WINDDK_XXX */

private:
	/// Constructor.
	/// Cannot be called directly; see open and enumerate.
	explicit LinuxInputDeviceHandle(std::shared_ptr<SurgSim::Framework::Logger>&& logger);

	/// Gets the indices of the available device buttons.
	/// \return a vector of indices.
	std::vector<int> getDeviceButtonsAndKeys();

	// Prevent copy construction and copy assignment.  (VS2012 does not support "= delete" yet.)
	LinuxInputDeviceHandle(const LinuxInputDeviceHandle& other) /*= delete*/;
	LinuxInputDeviceHandle& operator=(const LinuxInputDeviceHandle& other) /*= delete*/;

	struct State;
	std::unique_ptr<State> m_state;
};

};  // namespace Device
};  // namespace SurgSim

#endif  // SURGSIM_DEVICES_MULTIAXIS_LINUXINPUTDEVICEHANDLE_H
