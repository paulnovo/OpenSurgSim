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

#ifndef SURGSIM_GRAPHICS_OSGSCENERYREPRESENTATION_H
#define SURGSIM_GRAPHICS_OSGSCENERYREPRESENTATION_H

#include <SurgSim/Graphics/OsgRepresentation.h>
#include <SurgSim/Graphics/SceneryRepresentation.h>

#include <osg/Node>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4250)
#endif

namespace SurgSim
{
namespace Graphics
{

/// A OsgSceneryRepresentation is used to load osg object/node from file
class OsgSceneryRepresentation:	public OsgRepresentation, public SceneryRepresentation
{
public:
	friend class OsgSceneryRepresentationTest;

	/// Constructor
	/// \param name Name of OsgSceneryRepresentation
	explicit OsgSceneryRepresentation(const std::string& name);

	/// Return file name of the object
	/// \return File name of the object
	std::string getFileName() const override;
	/// Set file name of the object to be loaded
	/// \param	fileName Name of the file to be loaded
	void setFileName(const std::string& fileName) override;


private:
	virtual bool doInitialize() override;

	/// A osg::Node to hold the objet loaded from file
	osg::ref_ptr<osg::Node> m_sceneryRepresentation;

	/// Name of the object file to be loaded
	std::string m_fileName;
};

};  // namespace Graphics

};  // namespace SurgSim

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif  // SURGSIM_GRAPHICS_OSGSCENERYREPRESENTATION_H
