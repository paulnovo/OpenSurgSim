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

#include <string>
#include <sstream>
#include <stdlib.h>

#include "Examples/DroppingBalls/AddRandomSphereBehavior.h"

#include "SurgSim/Framework/Behavior.h"
#include "SurgSim/Framework/Scene.h"
#include "SurgSim/Framework/SceneElement.h"
#include "SurgSim/Blocks/SphereElement.h"
#include "SurgSim/Math/Vector.h"

using SurgSim::Blocks::SphereElement;
using SurgSim::Framework::Behavior;
using SurgSim::Framework::SceneElement;
using SurgSim::Math::Vector3d;

/// \file
/// A Behavior that creates randomly positioned SphereElements at a fixed rate.
/// \sa SurgSim::Blocks::SphereElement
namespace SurgSim
{

namespace Blocks
{

AddRandomSphereBehavior::AddRandomSphereBehavior(const std::string& name):
	Behavior(name), m_totalTime(0.0), m_numElements(0),
	m_distribution_xz(0.0, 1.0), m_distribution_y(1.0, 2.0)
{
}


AddRandomSphereBehavior::~AddRandomSphereBehavior()
{
}

bool AddRandomSphereBehavior::doInitialize()
{
	return true;
}

bool AddRandomSphereBehavior::doWakeUp()
{
	return true;
}

void AddRandomSphereBehavior::update(double dt)
{
	// Accumulate the time steps since the previous sphere was created.
	m_totalTime += dt;

	if (m_totalTime > 3.0)
	{
		m_totalTime = 0.0;

		std::stringstream ss;
		ss << ++ m_numElements;

		// Generate a random position.
		double m_x = m_distribution_xz(m_generator);
		double m_y = m_distribution_y(m_generator);
		double m_z = m_distribution_xz(m_generator);

		std::string name = "sphereId_" + ss.str();
		// Create the pose, with no rotation and the previously determined position.
		SurgSim::Math::RigidTransform3d pose = SurgSim::Math::makeRigidTransform
			(SurgSim::Math::Quaterniond::Identity(), Vector3d(m_x, m_y, m_z));

		// Create the SphereElement.
		std::shared_ptr<SceneElement> element = std::make_shared<SphereElement>(name);
		element->setPose(pose);

		// Add the SphereElement to the Scene.
		getScene()->addSceneElement(element);
	}
}

};  // namespace Blocks

};  // namespace SurgSim
