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

#include <memory>

#include "SurgSim/Physics/MassSpringRepresentationContact.h"
#include "SurgSim/Physics/ContactConstraintData.h"
#include "SurgSim/Physics/ConstraintImplementation.h"

#include "SurgSim/Physics/Localization.h"
#include "SurgSim/Physics/MassSpringRepresentationLocalization.h"

namespace SurgSim
{

namespace Physics
{

MassSpringRepresentationContact::MassSpringRepresentationContact()
{

}

MassSpringRepresentationContact::~MassSpringRepresentationContact()
{

}

void MassSpringRepresentationContact::doBuild(double dt,
			const ConstraintData& data,
			const std::shared_ptr<Localization>& localization,
			MlcpPhysicsProblem* mlcp,
			unsigned int indexOfRepresentation,
			unsigned int indexOfConstraint,
			ConstraintSideSign sign)
{
	using SurgSim::Math::Vector3d;

	auto massSpring = std::static_pointer_cast<MassSpringRepresentation>(localization->getRepresentation());

	if ( !massSpring->isActive())
	{
		return;
	}

	unsigned int nodeId = std::static_pointer_cast<MassSpringRepresentationLocalization>(localization)->getLocalNode();
	const double scale = (sign == CONSTRAINT_POSITIVE_SIDE) ? 1.0 : -1.0;

	auto& contactData = static_cast<const ContactConstraintData&>(data);
	const Vector3d& n = contactData.getNormal();
	const double d = contactData.getDistance();

	// FRICTIONLESS CONTACT in a LCP
	//   (n, d) defines the plane of contact
	//   p(t) the point of contact (usually after free motion)
	//
	// The constraint equation for a plane is
	// U(t) = n^t.p(t) + d >= 0
	//
	// dU/dt = H.dp/dt
	// => H = n^t

	// Update b with new violation U
	Vector3d globalPosition = localization->calculatePosition();
	double violation = n.dot(globalPosition) + d;

	mlcp->b[indexOfConstraint] += violation * scale;

	Eigen::SparseVector<double> newH;
	newH.resize(massSpring->getNumDof());
	newH.reserve(3);
	newH.insert(3 * nodeId + 0) = n[0] * scale;
	newH.insert(3 * nodeId + 1) = n[1] * scale;
	newH.insert(3 * nodeId + 2) = n[2] * scale;

	mlcp->updateConstraints(newH, massSpring->getComplianceMatrix(), indexOfRepresentation, indexOfConstraint);
}

SurgSim::Math::MlcpConstraintType MassSpringRepresentationContact::getMlcpConstraintType() const
{
	return SurgSim::Math::MLCP_UNILATERAL_3D_FRICTIONLESS_CONSTRAINT;
}

SurgSim::Physics::RepresentationType MassSpringRepresentationContact::getRepresentationType() const
{
	return REPRESENTATION_TYPE_MASSSPRING;
}

unsigned int MassSpringRepresentationContact::doGetNumDof() const
{
	return 1;
}

}; // namespace Physics

}; // namespace SurgSim