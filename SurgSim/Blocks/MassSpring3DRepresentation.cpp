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

#include "SurgSim/Blocks/MassSpring3DRepresentation.h"
#include "SurgSim/Blocks/MassSpringNDRepresentationUtils.h"
#include "SurgSim/Math/OdeState.h"
#include "SurgSim/Physics/LinearSpring.h"

using SurgSim::Physics::Mass;
using SurgSim::Math::Vector3d;

namespace SurgSim
{

namespace Blocks
{

void MassSpring3DRepresentation::init3DStretchingSprings(const std::shared_ptr<SurgSim::Math::OdeState> state,
	size_t numNodesPerDim[3], double stiffness, double damping)
{
	const size_t depthOffset = numNodesPerDim[0] * numNodesPerDim[1];
	const size_t rowOffset = numNodesPerDim[0];
	const size_t colOffset = 1;

	// Initialize the stretching springs
	if (stiffness|| damping)
	{
		// ... along X
		for (size_t depth = 0; depth < numNodesPerDim[2]; depth++)
		{
			for (size_t row = 0; row < numNodesPerDim[1]; row++)
			{
				for (size_t col = 0; col < numNodesPerDim[0] - 1; col++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + colOffset, stiffness, damping));
				}
			}
		}
		// ... along Y
		for (size_t depth = 0; depth < numNodesPerDim[2]; depth++)
		{
			for (size_t col = 0; col < numNodesPerDim[0]; col++)
			{
				for (size_t row = 0; row < numNodesPerDim[1] - 1; row++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + rowOffset, stiffness, damping));
				}
			}
		}
		// ... along Z
		for (size_t row = 0; row < numNodesPerDim[1]; row++)
		{
			for (size_t col = 0; col < numNodesPerDim[0]; col++)
			{
				for (size_t depth = 0; depth < numNodesPerDim[2] - 1; depth++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + depthOffset, stiffness, damping));
				}
			}
		}
	}
}

void MassSpring3DRepresentation::init3DBendingSprings(const std::shared_ptr<SurgSim::Math::OdeState> state,
	size_t numNodesPerDim[3], double stiffness, double damping)
{
	const size_t depthOffset = numNodesPerDim[0] * numNodesPerDim[1];
	const size_t rowOffset = numNodesPerDim[0];
	const size_t colOffset = 1;

	// Initialize the bending springs
	if (stiffness || damping)
	{
		// ... along X
		for (size_t depth = 0; depth < numNodesPerDim[2]; depth++)
		{
			for (size_t row = 0; row < numNodesPerDim[1]; row++)
			{
				for (size_t col = 0; col < numNodesPerDim[0] - 2; col++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + 2 * colOffset, stiffness, damping));
				}
			}
		}
		// ... along Y
		for (size_t depth = 0; depth < numNodesPerDim[2]; depth++)
		{
			for (size_t col = 0; col < numNodesPerDim[0]; col++)
			{
				for (size_t row = 0; row < numNodesPerDim[1] - 2; row++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + 2 * rowOffset, stiffness, damping));
				}
			}
		}
		// ... along Z
		for (size_t row = 0; row < numNodesPerDim[1]; row++)
		{
			for (size_t col = 0; col < numNodesPerDim[0]; col++)
			{
				for (size_t depth = 0; depth < numNodesPerDim[2] - 2; depth++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + 2 * depthOffset, stiffness, damping));
				}
			}
		}
	}
}

void MassSpring3DRepresentation::init3DFaceDiagonalSprings(const std::shared_ptr<SurgSim::Math::OdeState> state,
	size_t numNodesPerDim[3], double stiffness, double damping)
{
	const size_t depthOffset = numNodesPerDim[0] * numNodesPerDim[1];
	const size_t rowOffset = numNodesPerDim[0];
	const size_t colOffset = 1;

	// Initialize the face diagonal springs
	if (stiffness || damping)
	{
		// ... faces orthogonal to Z
		for (size_t depth = 0; depth < numNodesPerDim[2]; depth++)
		{
			for (size_t row = 0; row < numNodesPerDim[1] - 1; row++)
			{
				for (size_t col = 0; col < numNodesPerDim[0] - 1; col++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + rowOffset + colOffset, stiffness, damping));
					addSpring(createLinearSpring(state, nodeId + colOffset, nodeId + rowOffset, stiffness, damping));
				}
			}
		}
		// ... faces orthogonal to Y
		for (size_t row = 0; row < numNodesPerDim[1]; row++)
		{
			for (size_t depth = 0; depth < numNodesPerDim[2] - 1; depth++)
			{
				for (size_t col = 0; col < numNodesPerDim[0] - 1; col++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + depthOffset + colOffset, stiffness, damping));
					addSpring(createLinearSpring(state, nodeId + colOffset, nodeId + depthOffset, stiffness, damping));
				}
			}
		}
		// ... faces orthogonal to X
		for (size_t col = 0; col < numNodesPerDim[0]; col++)
		{
			for (size_t row = 0; row < numNodesPerDim[1] - 1; row++)
			{
				for (size_t depth = 0; depth < numNodesPerDim[2] - 1; depth++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + depthOffset + rowOffset, stiffness, damping));
					addSpring(createLinearSpring(state, nodeId + rowOffset, nodeId + depthOffset, stiffness, damping));
				}
			}
		}
	}
}

void MassSpring3DRepresentation::init3DVolumeDiagonalSprings(const std::shared_ptr<SurgSim::Math::OdeState> state,
	size_t numNodesPerDim[3], double stiffness, double damping)
{
	const size_t depthOffset = numNodesPerDim[0] * numNodesPerDim[1];
	const size_t rowOffset = numNodesPerDim[0];
	const size_t colOffset = 1;

	// For convenience
	double &s = stiffness;
	double &d = damping;

	// Initialize the volume diagonal springs
	if (stiffness || damping)
	{
		for (size_t col = 0; col < numNodesPerDim[0] - 1; col++)
		{
			for (size_t row = 0; row < numNodesPerDim[1] - 1; row++)
			{
				for (size_t depth = 0; depth < numNodesPerDim[2] - 1; depth++)
				{
					size_t nodeId = depth * depthOffset + row * rowOffset + col * colOffset;
					addSpring(createLinearSpring(state, nodeId, nodeId + depthOffset + rowOffset + colOffset, s, d));
					addSpring(createLinearSpring(state, nodeId + colOffset, nodeId + depthOffset + rowOffset, s, d));
					addSpring(createLinearSpring(state, nodeId + rowOffset, nodeId + depthOffset + colOffset, s, d));
					addSpring(createLinearSpring(state, nodeId + rowOffset + colOffset, nodeId + depthOffset, s, d));
				}
			}
		}
	}
}

void MassSpring3DRepresentation::init3D(
	const std::array<std::array<std::array<SurgSim::Math::Vector3d, 2>, 2>, 2> extremities,
	size_t numNodesPerDim[3],
	std::vector<size_t> nodeBoundaryConditions,
	double totalMass,
	double stiffnessStretching, double dampingStretching,
	double stiffnessBending, double dampingBending,
	double stiffnessFaceDiagonal, double dampingFaceDiagonal,
	double stiffnessVolumeDiagonal, double dampingVolumeDiagonal)
{
	std::shared_ptr<SurgSim::Math::OdeState> state;
	state = std::make_shared<SurgSim::Math::OdeState>();
	state->setNumDof(getNumDofPerNode(), numNodesPerDim[0] * numNodesPerDim[1] * numNodesPerDim[2]);

	// Nodes distribution is done by column 1st, row 2nd, depth 3rd
	// Example: given a nodeId
	//   Its neighbor on the next column (colOffset)   is nodeId + 1
	//   Its neighbor on the next row    (rowOffset)   is nodeId + numNodesPerDim[0]
	//   Its neighbor on the next depth  (depthOffset) is nodeId + numNodesPerDim[0] * numNodesPerDim[1]
	SURGSIM_ASSERT(numNodesPerDim[0] > 0) << "Number of nodes for dimension 1 is incorrect: " << numNodesPerDim[0];
	SURGSIM_ASSERT(numNodesPerDim[1] > 0) << "Number of nodes for dimension 2 is incorrect: " << numNodesPerDim[1];
	SURGSIM_ASSERT(numNodesPerDim[2] > 0) << "Number of nodes for dimension 3 is incorrect: " << numNodesPerDim[2];

	const size_t numNodes = numNodesPerDim[0] * numNodesPerDim[1] * numNodesPerDim[2];

	// Initialize the nodes position, velocity and mass
	// Note: no need to apply the initialPose here, initialize will take care of it !
	Vector3d depthExtremitiesDelta[2][2] =
	{{(extremities[0][0][1] - extremities[0][0][0]) / static_cast<double>(numNodesPerDim[2] - 1) ,
	(extremities[1][0][1] - extremities[1][0][0]) / static_cast<double>(numNodesPerDim[2] - 1)}
	,
	{(extremities[0][1][1] - extremities[0][1][0]) / static_cast<double>(numNodesPerDim[2] - 1) ,
	(extremities[1][1][1] - extremities[1][1][0]) / static_cast<double>(numNodesPerDim[2] - 1)}};

	size_t nodeId = 0;
	for (size_t depth = 0; depth < numNodesPerDim[2]; depth++)
	{
		Vector3d depthExtremities[2][2];
		depthExtremities[0][0] = extremities[0][0][0] + depthExtremitiesDelta[0][0] * static_cast<double>(depth);
		depthExtremities[1][0] = extremities[1][0][0] + depthExtremitiesDelta[0][1] * static_cast<double>(depth);
		depthExtremities[0][1] = extremities[0][1][0] + depthExtremitiesDelta[1][0] * static_cast<double>(depth);
		depthExtremities[1][1] = extremities[1][1][0] + depthExtremitiesDelta[1][1] * static_cast<double>(depth);

		Vector3d rowExtremitiesDelta[2] =
		{(depthExtremities[0][1] - depthExtremities[0][0]) / static_cast<double>(numNodesPerDim[1] - 1) ,
		(depthExtremities[1][1] - depthExtremities[1][0]) / static_cast<double>(numNodesPerDim[1] - 1)};
		for (size_t row = 0; row < numNodesPerDim[1]; row++)
		{
			Vector3d rowExtremities[2];
			rowExtremities[0] = depthExtremities[0][0] + rowExtremitiesDelta[0] * static_cast<double>(row);
			rowExtremities[1] = depthExtremities[1][0] + rowExtremitiesDelta[1] * static_cast<double>(row);

			Vector3d delta = (rowExtremities[1] - rowExtremities[0]) / static_cast<double>(numNodesPerDim[0] - 1);
			for (size_t col = 0; col < numNodesPerDim[0]; col++)
			{
				addMass(std::make_shared<Mass>(totalMass / static_cast<double>(numNodes)));

				Vector3d position(rowExtremities[0] + static_cast<double>(col) * delta);
				SurgSim::Math::setSubVector(position, nodeId, 3, &state->getPositions());

				nodeId++;
			}
		}
	}

	// Initialize all the stretching springs
	init3DStretchingSprings(state, numNodesPerDim, stiffnessStretching, dampingStretching);

	// Initialize all the bending springs
	init3DBendingSprings(state, numNodesPerDim, stiffnessBending, dampingBending);

	// Initialize all the face diagonal springs
	init3DFaceDiagonalSprings(state, numNodesPerDim, stiffnessFaceDiagonal, dampingFaceDiagonal);

	// Initialize all the volume diagonal springs
	init3DVolumeDiagonalSprings(state, numNodesPerDim, stiffnessVolumeDiagonal, dampingVolumeDiagonal);

	// Sets the boundary conditions
	for (auto boundaryCondition = std::begin(nodeBoundaryConditions);
		boundaryCondition != std::end(nodeBoundaryConditions);
		boundaryCondition++)
	{
		state->addBoundaryCondition(*boundaryCondition);
	}

	// setInitialState: Initialize all the states + apply initialPose if any
	setInitialState(state);
}

}; // namespace Blocks

}; // namespace SurgSim
