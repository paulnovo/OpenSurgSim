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

#ifndef SURGSIM_GRAPHICS_ACTOR_H
#define SURGSIM_GRAPHICS_ACTOR_H

#include <SurgSim/Framework/Representation.h>

#include <SurgSim/Math/RigidTransform.h>

namespace SurgSim
{

namespace Graphics
{

class Manager;
class Material;

/// Base graphics actor class, which defines the interface that all graphics actors must implement.
///
/// A Graphics::Actor is the visual Framework::Representation of a Framework::SceneElement in the Framework::Scene.
class Actor : public SurgSim::Framework::Representation
{
public:
	/// Constructor
	/// \param	name	Name of the actor
	explicit Actor(const std::string& name) : SurgSim::Framework::Representation(name),
		m_initialPose(SurgSim::Math::RigidTransform3d::Identity())
	{
	}

	/// Sets whether the actor is currently visible
	/// \param	visible	True for visible, false for invisible
	virtual void setVisible(bool visible) = 0;

	/// Gets whether the actor is currently visible
	/// \return	visible	True for visible, false for invisible
	virtual bool isVisible() const = 0;

	/// Set the initial pose of the representation
	/// \param	pose	The initial pose
	/// \note	This will reset initial, current, and final poses all to the new initial pose.
	virtual void setInitialPose(const SurgSim::Math::RigidTransform3d& pose)
	{
		m_initialPose = pose;
		setCurrentPose(m_initialPose);
	}

	/// Get the initial pose of the representation
	/// \return	The initial pose
	virtual const SurgSim::Math::RigidTransform3d& getInitialPose() const
	{
		return m_initialPose;
	}

	/// Get the current pose of the representation
	/// \param	pose Rigid transformation that describes the current pose of the representation
	/// \note	This is an intermediate pose, while getFinalPose() returns the last valid (end of timestep) pose
	virtual const SurgSim::Math::RigidTransform3d& getCurrentPose() const = 0;

	/// Get the final pose of the representation (i.e. last valid pose)
	/// \return	The final pose
	/// \note	For graphics, the final pose is just the current pose.
	virtual const SurgSim::Math::RigidTransform3d& getFinalPose() const
	{
		return getCurrentPose();
	}

	/// Updates the actor
	/// \param	dt	The time in seconds of the preceding timestep.
	virtual void update(double dt) = 0;

private:

	/// Initial pose of the representation
	SurgSim::Math::RigidTransform3d m_initialPose;
};

};  // namespace Graphics

};  // namespace SurgSim

#endif  // SURGSIM_GRAPHICS_ACTOR_H
