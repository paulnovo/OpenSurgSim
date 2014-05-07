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

#ifndef SURGSIM_PHYSICS_FEM3DREPRESENTATION_H
#define SURGSIM_PHYSICS_FEM3DREPRESENTATION_H

#include <memory>
#include <string>
#include <unordered_map>

#include "SurgSim/Physics/FemRepresentation.h"
#include "SurgSim/Math/Matrix.h"

namespace SurgSim
{

namespace DataStructures
{
class TriangleMesh;
}

namespace Physics
{

/// Finite Element Model 3D is a fem built with 3D FemElement
class Fem3DRepresentation : public FemRepresentation
{
public:
	/// Constructor
	/// \param name The name of the Fem3DRepresentation
	explicit Fem3DRepresentation(const std::string& name);

	/// Destructor
	virtual ~Fem3DRepresentation();

	/// Sets the name of the file to be loaded
	/// \param filename The name of the file to be loaded
	void setFilename(const std::string& filename);

	/// Gets the name of the file to be loaded
	/// \return filename The name of the file to be loaded
	const std::string& getFilename() const;

	/// Loads the file
	/// \return true if successful
	/// \note This function is a temporary workaround and its usage is discouraged.
	bool loadFile();

	/// Query the representation type
	/// \return the RepresentationType for this representation
	virtual RepresentationType getType() const override;

	/// Update the Representation's current position and velocity using a time interval, dt, and change in velocity,
	/// deltaVelocity.
	///
	/// This function typically is called in the physics pipeline (PhysicsManager::doUpdate) after solving the equations
	/// that enforce constraints when collisions occur.  Specifically it is called in the PushResults::doUpdate step.
	/// \param dt The time step
	/// \param deltaVelocity The block of a vector containing the correction to be applied to the velocity
	virtual void applyCorrection(double dt, const Eigen::VectorBlock<SurgSim::Math::Vector>& deltaVelocity) override;

	virtual std::shared_ptr<Localization> createLocalization(const SurgSim::Collision::Location& location) override;

protected:
	virtual bool doWakeUp() override;

	/// Interface to be implemented by derived classes
	/// \return True if component is initialized successfully; otherwise, false.
	virtual bool doInitialize() override;

	/// Transform a state using a given transformation
	/// \param[in,out] state The state to be transformed
	/// \param transform The transformation to apply
	virtual void transformState(std::shared_ptr<DeformableRepresentationState> state,
		const SurgSim::Math::RigidTransform3d& transform) override;

	/// Determine whether the associated deformable state is valid
	/// \param state The state to check
	/// \result True if valid
	bool isValidState(const DeformableRepresentationState &state) const;

	/// Deactivate and call resetState
	void deactivateAndReset(void);

private:
	/// Produces a mapping from the provided mesh's triangle ids to this object's fem element ids. The mesh's vertices
	/// must be identical to this object's fem element nodes.
	/// \param mesh The mesh used to produce the mapping.
	/// \return A map from the mesh's triangle ids to this object's fem elements.
	std::unordered_map<size_t, size_t> createTriangleIdToElementIdMap(
		const SurgSim::DataStructures::TriangleMesh& mesh);

	/// Filename for loading the fem3d representation.
	std::string m_filename;

	/// Whether the file should be loaded or not.
	bool m_doLoadFile;

	/// Mapping from collision triangle's id to fem element id.
	std::unordered_map<size_t, size_t> m_triangleIdToElementIdMap;
};

} // namespace Physics

} // namespace SurgSim

#endif // SURGSIM_PHYSICS_FEM3DREPRESENTATION_H
