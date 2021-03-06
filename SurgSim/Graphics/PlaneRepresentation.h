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

#ifndef SURGSIM_GRAPHICS_PLANEREPRESENTATION_H
#define SURGSIM_GRAPHICS_PLANEREPRESENTATION_H

#include "SurgSim/Graphics/Representation.h"

namespace SurgSim
{

namespace Graphics
{

/// Base graphics plane representation class, which defines the basic interface for a plane that can be visualized.
/// The plane is the XZ plane, with normal +Y.
class PlaneRepresentation : public virtual Representation
{
public:
	/// Constructor
	/// \param	name	Name of the representation
	explicit PlaneRepresentation(const std::string& name) : Representation(name)
	{
	}
};

};  // namespace Graphics

};  // namespace SurgSim

#endif  // SURGSIM_GRAPHICS_PLANEREPRESENTATION_H
