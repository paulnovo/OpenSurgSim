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

#include <SurgSim/Physics/ContactConstraintData.h>
#include <SurgSim/Physics/FixedRepresentation.h>
#include <SurgSim/Physics/FixedRepresentationContact.h>
#include <SurgSim/Physics/FixedRepresentationLocalization.h>
#include <SurgSim/Physics/Localization.h>
#include <SurgSim/Physics/Representation.h>

namespace SurgSim
{

namespace Physics
{

FixedRepresentationContact::FixedRepresentationContact(std::shared_ptr<Localization> localization) :
ConstraintImplementation(localization)
{
}

FixedRepresentationContact::~FixedRepresentationContact()
{
}

unsigned int FixedRepresentationContact::doGetNumDof() const
{
	return 1;
}

void FixedRepresentationContact::doBuild(double dt,
	const ConstraintData& data,
	MlcpPhysicsProblem& mlcp,
	unsigned int indexOfRepresentation,
	unsigned int indexOfConstraint,
	ConstraintSideSign sign)
{
	Eigen::VectorXd& b = mlcp.b;

	std::shared_ptr<Representation> representation = getLocalization()->getRepresentation();
	std::shared_ptr<FixedRepresentation> fixed = std::static_pointer_cast<FixedRepresentation>(representation);

	if (! fixed->isActive())
	{
		return;
	}

	const double scale = (sign == CONSTRAINT_POSITIVE_SIDE ? 1.0 : -1.0);
	const ContactConstraintData& contactData = static_cast<const ContactConstraintData&>(data);
	const SurgSim::Math::Vector3d& n = contactData.getNormal();
	const double d = contactData.getDistance();

	SurgSim::Math::Vector3d globalPosition = getLocalization()->calculatePosition();

	// Fill up b with the constraint equation...
	double violation = n.dot(globalPosition) + d;
	b[indexOfConstraint] += violation * scale;
}

/// Gets the Mixed Linear Complementarity Problem constraint type for this ConstraintImplementation
/// \return The MLCP constraint type corresponding to this constraint implementation
SurgSim::Math::MlcpConstraintType FixedRepresentationContact::doGetMlcpConstraintType() const
{
	return SurgSim::Math::MLCP_UNILATERAL_3D_FRICTIONLESS_CONSTRAINT;
}

};  //  namespace Physics

};  //  namespace SurgSim
