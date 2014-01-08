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

#ifndef SURGSIM_INPUT_COMMONDEVICE_H
#define SURGSIM_INPUT_COMMONDEVICE_H

#include <memory>
#include <string>

#include "SurgSim/Input/DeviceInterface.h"
#include "SurgSim/Input/InputConsumerInterface.h"
#include "SurgSim/DataStructures/DataGroup.h"

namespace SurgSim
{
namespace Input
{


/// A class that implements some common management code on top of the DeviceInterface.
/// Practically every class that implements DeviceInterface will likely want to inherit from CommonDevice.
class CommonDevice : public DeviceInterface
{
public:
	/// Constructor.
	///
	/// \param name The name associated with the input device.
	/// \param inputData An initial value for the application's input from the device (e.g. pose etc).
	/// 	The concrete device implementation should pass in a DataGroup whose contents has been set up, e.g. by
	/// 	using a DataGroupBuilder, to that device's supported values that it will push to the application.
	CommonDevice(const std::string& name, const SurgSim::DataStructures::DataGroup& inputData);

	/// Constructor.
	///
	/// \param name The name associated with the input device.
	/// \param inputData An initial value for the application's input from the device (e.g. pose etc).
	/// 	The concrete device implementation should pass in a DataGroup whose contents has been set up, e.g. by
	/// 	using a DataGroupBuilder, to that device's supported values that it will push to the application.
	CommonDevice(const std::string& name, SurgSim::DataStructures::DataGroup&& inputData);

	/// Destructor.
	virtual ~CommonDevice();

	/// Return a (hopefully unique) device name.
	virtual std::string getName() const override;

	/// Set the name used for calling the input consumers and output producer.
	/// By default, this will be the same as the name of the device that was passed to the constructor.
	/// \param name	The name to be used.
	void setNameForCallback(const std::string& name);

	/// Get the name used for calling the input consumers and output producer.
	/// By default, this will be the same as the name of the device that was passed to the constructor.
	/// \return	The name being used.
	std::string getNameForCallback() const;

	/// Connect this device to an InputConsumerInterface, which will receive the data that comes from this device.
	/// \param inputConsumer The InputConsumerInterface to connect with.
	/// \return true if successful
	virtual bool addInputConsumer(std::shared_ptr<InputConsumerInterface> inputConsumer) override;

	/// Disconnect this device from an InputConsumerInterface, which will no longer receive data from this device.
	/// \param inputConsumer The InputConsumerInterface to disconnect from.
	/// \return true if successful
	virtual bool removeInputConsumer(std::shared_ptr<InputConsumerInterface> inputConsumer) override;

	/// Connect this device to an OutputProducerInterface, which will send data to this device.
	/// \param outputProducer The OutputProducerInterface to connect with.
	/// \return true if successful
	virtual bool setOutputProducer(std::shared_ptr<OutputProducerInterface> outputProducer) override;

	/// Disconnect this device from an OutputProducerInterface, which will no longer send data to this device.
	/// \param outputProducer The OutputProducerInterface to disconnect from.
	/// \return true if successful
	virtual bool removeOutputProducer(std::shared_ptr<OutputProducerInterface> outputProducer) override;

	/// Getter for whether or not this device is connected with an OutputProducerInterface.
	/// \return true if an OutputProducerInterface is connected.
	virtual bool hasOutputProducer() override;

protected:

	/// Push application input to consumers.
	virtual void pushInput();

	/// Pull application output from a producer.
	virtual bool pullOutput();

	/// Getter for the initial input data \ref SurgSim::DataStructures::DataGroup "DataGroup".  This function may be
	/// called to provide initial data to input consumers (e.g., passed to the consumer's constructor).
	/// \return A copy of the initial input data.
	const SurgSim::DataStructures::DataGroup getInitialInputData() const
	{
		return m_initialInputData;
	}

	/// Getter for the input data \ref SurgSim::DataStructures::DataGroup "DataGroup".  This function is typically
	/// called by friend scaffolds, to get a DataGroup they can copy, then modify and send to the device's input
	/// consumers.
	/// \return A copy of the input data.
	const SurgSim::DataStructures::DataGroup getInputData() const
	{
		return m_inputData;
	}

	/// Setter for the input data \ref SurgSim::DataStructures::DataGroup "DataGroup".  This function is typically
	/// called by friend scaffolds, to put the data from the device's SDK into the device's member variable so it can be
	/// pushed to the device's input consumers.
	/// \return A copy of the input data.
	void setInputData(const SurgSim::DataStructures::DataGroup& inputData)
	{
		m_inputData = inputData;
	}

	/// Getter for the output data \ref SurgSim::DataStructures::DataGroup "DataGroup".  This function is typically
	/// called by friend scaffolds, to get the data that the output producer wants to send to the device (and then send
	/// that data through the device's SDK). Note that a writable variant is not provided, an output producer registered
	/// via \ref setOutputProducer will set the output data.
	/// \return A copy of the output data.
	const SurgSim::DataStructures::DataGroup getOutputData() const
	{
		return m_outputData;
	}

	/// Data used to initialize the input to an InputConsumerInterface when it is added to this device.
	SurgSim::DataStructures::DataGroup m_initialInputData;

	/// The data the device is providing to its input consumers.
	SurgSim::DataStructures::DataGroup m_inputData;

	/// The data the output producer (if any) is providing to the device.
	SurgSim::DataStructures::DataGroup m_outputData;

private:
	struct State;

	const std::string m_name;

	/// The name used for the callbacks, defaults to the device name.
	std::string m_nameForCallback;

	/// Struct to hide some of the private member variables, PImpl (Pointer to Implementation).
	std::unique_ptr<State> m_state;
};


};  // namespace Input
};  // namespace SurgSim

#endif // SURGSIM_INPUT_COMMONDEVICE_H
