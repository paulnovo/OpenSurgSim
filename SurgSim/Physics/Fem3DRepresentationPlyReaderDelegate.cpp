// This file is a part of the OpenSurgSim project.
// Copyright 2014, SimQuest Solutions Inc.
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

#include <array>

#include "SurgSim/DataStructures/PlyReader.h"
#include "SurgSim/Physics/Fem3DElementTetrahedron.h"
#include "SurgSim/Physics/Fem3DRepresentation.h"
#include "SurgSim/Physics/Fem3DRepresentationPlyReaderDelegate.h"

using SurgSim::DataStructures::PlyReader;

namespace SurgSim
{
namespace Physics
{

Fem3DRepresentationPlyReaderDelegate::Fem3DRepresentationPlyReaderDelegate(std::shared_ptr<Fem3DRepresentation> fem)
	: FemRepresentationPlyReaderDelegate(fem)
{
}

std::string Fem3DRepresentationPlyReaderDelegate::getElementName() const
{
	return "3d_element";
}

void Fem3DRepresentationPlyReaderDelegate::processFemElement(const std::string& elementName)
{
	SURGSIM_ASSERT(m_femData.vertexCount == 4) << "Cannot process 3D element with "
											   << m_femData.vertexCount << " vertices.";

	std::array<size_t, 4> fem3DVertices;
	std::copy(m_femData.indices, m_femData.indices + 4, fem3DVertices.begin());
	m_fem->addFemElement(std::make_shared<Fem3DElementTetrahedron>(fem3DVertices));
}

}; // namespace Physics
}; // namespace SurgSim