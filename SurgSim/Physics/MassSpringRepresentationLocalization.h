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

#ifndef SURGSIM_PHYSICS_MASSSPRINGREPRESENTATIONLOCALIZATION_H
#define SURGSIM_PHYSICS_MASSSPRINGREPRESENTATIONLOCALIZATION_H

#include "SurgSim/Physics/Localization.h"
#include "SurgSim/Physics/MassSpringRepresentation.h"

namespace SurgSim
{

namespace Physics
{

/// Implementation of Localization for MassSpringRepresentation
///
/// MassSpringRepresentationLocalization is a helper class for filling out the MlcpPhysicsProblem when applying
/// constraints to the motion of a MassSpringRepresentation.  For a concrete example, see
/// MassSpringRepresentationContact::doBuild.
///
/// MassSpringRepresentationLocalization stores a pointer to a MassSpringRepresentation in an abstract Representation
/// object.  It tracks the ID of a node contained within the associated MassSpringRepresentation, and it provides a
/// helper function MassSpringRepresentationLocalization::calculatePosition to find the node's position in global
/// coordinates.
class MassSpringRepresentationLocalization: public Localization
{
public:
	/// Default constructor
	MassSpringRepresentationLocalization();

	/// Constructor
	/// \param representation The representation to assign to this localization.
	explicit MassSpringRepresentationLocalization(std::shared_ptr<Representation> representation);

	/// Destructor
	virtual ~MassSpringRepresentationLocalization();

	/// Sets the local node.
	/// \param nodeID Node set for this localization.
	void setLocalNode(size_t nodeID);

	/// Gets the local node.
	/// \return Node set for this localization.
	const size_t& getLocalNode() const;

	/// Queries whether Representation can be assigned to this class.
	/// \param representation Representation to check.
	/// \return	true if Representation is valid.
	virtual bool isValidRepresentation(std::shared_ptr<Representation> representation) override;

private:
	/// Calculates the global position of this localization.
	/// \param time Interpolation parameter [0..1] for calcuting position between the previous time-step (0.0) and the
	/// current time-step (1.0).
	/// \return The global position of the localization at the requested interpolation.
	/// \note time can useful when dealing with CCD.
	virtual SurgSim::Math::Vector3d doCalculatePosition(double time) override;

	/// Node defining the localization.
	size_t m_nodeID;
};

};  // namespace Physics

};  // namespace SurgSim

#endif  // SURGSIM_PHYSICS_MASSSPRINGREPRESENTATIONLOCALIZATION_H
