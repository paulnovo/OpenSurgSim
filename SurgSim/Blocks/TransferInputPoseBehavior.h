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

#ifndef SURGSIM_BLOCKS_TRANSFERINPUTPOSEBEHAVIOR_H
#define SURGSIM_BLOCKS_TRANSFERINPUTPOSEBEHAVIOR_H

#include <string>
#include "SurgSim/Framework/Behavior.h"

namespace SurgSim
{

namespace Input
{
	class InputComponent;
}

namespace Framework
{
	class Representation;
}

namespace Blocks
{

class TransferInputPoseBehavior : public SurgSim::Framework::Behavior
{
public:
	/// Constructor
	/// \param	name	Name of the behavior
	explicit TransferInputPoseBehavior(const std::string& name);

	/// Set representation to get the pose from.
	/// \param	from	Representation to get the pose
	void setPoseFrom(std::shared_ptr<SurgSim::Input::InputComponent> from);

	/// Set representation to set the pose to.
	/// \param	to	Representation to set the pose
	void setPoseTo(std::shared_ptr<SurgSim::Framework::Representation> to);

	/// Set name of the pose.
	/// \param	poseName	The name of the pose.
	void setPoseName(const std::string& poseName);

	/// Update the behavior
	/// \param dt	The length of time (seconds) between update calls.
	virtual void update(double dt);

protected:
	/// Initialize the behavior
	virtual bool doInitialize();

	/// Wakeup the behavior
	virtual bool doWakeUp();

private:
	/// Representation to get the pose
	std::shared_ptr<SurgSim::Input::InputComponent> m_from;
	/// Representation to set the pose
	std::shared_ptr<SurgSim::Framework::Representation> m_to;

	std::string m_poseName;
};


};  // namespace Blocks

};  // namespace SurgSim

#endif  //SURGSIM_BLOCKS_TRANSFERINPUTPOSEBEHAVIOR_H
