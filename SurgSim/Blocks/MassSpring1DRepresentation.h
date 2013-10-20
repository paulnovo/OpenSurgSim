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

#ifndef SURGSIM_BLOCKS_MASSSPRING1DREPRESENTATION_H
#define SURGSIM_BLOCKS_MASSSPRING1DREPRESENTATION_H

#include <vector>

#include <SurgSim/Physics/MassSpringRepresentation.h>

namespace SurgSim
{

namespace Blocks
{

// This class defines a simple MassSpring 1D structures
class MassSpring1DRepresentation : public SurgSim::Physics::MassSpringRepresentation
{
public:
	/// Constructor
	/// \param name The model name
	explicit MassSpring1DRepresentation(const std::string& name) :
		SurgSim::Physics::MassSpringRepresentation(name)
	{
	}

	/// Initializes a 1D model
	/// \param extremities Array of 2 positions forming the extremities of the 1D model
	/// \param numNodesPerDim The number of nodes to be created for each dimension (here 1)
	/// \param boundaryConditions The list of all boundary conditions (fixed dof)
	/// \param totalMass The total mass of the mass spring (evenly spread out on the masses)
	/// \param stiffnessStretching, dampingStretching The spring param  for all stretching springs (edges)
	/// \param stiffnessBending, dampingBending The spring param for all bending springs (edges)
	/// \note Stretching springs are connecting neighbors, bending springs are connecting 1 node
	/// \note to its 2nd degree neighbors, creating a bending force around the middle node.
	void init1D(const SurgSim::Math::Vector3d extremities[2], unsigned int numNodesPerDim[1],
		std::vector<unsigned int> boundaryConditions,
		double totalMass,
		double stiffnessStretching, double dampingStretching,
		double stiffnessBending, double dampingBending);
};

};  // namespace Blocks

};  // namespace SurgSim

#endif  // SURGSIM_BLOCKS_MASSSPRING1DREPRESENTATION_H
