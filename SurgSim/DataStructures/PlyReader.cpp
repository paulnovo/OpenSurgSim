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

#include <algorithm>

#include "SurgSim/Datastructures/PlyReader.h"
#include "SurgSim/DataStructures/PlyReaderDelegate.h"
#include "SurgSim/DataStructures/ply.h"

#include "SurgSim/Math/Vector.h"

using SurgSim::Math::Vector3d;

namespace SurgSim
{
namespace DataStructures
{

// Pimpl data, keep ply.h out of the SurgSim include chain
struct PlyReader::Data
{
	Data() :
		plyFile(nullptr),
		elementNames(nullptr)
	{
		types[TYPE_INVALID] = PLY_START_TYPE;
		types[TYPE_CHAR] = PLY_CHAR;
		types[TYPE_SHORT] = PLY_SHORT;
		types[TYPE_INT] = PLY_INT;
		types[TYPE_UNSIGNED_CHAR] = PLY_UCHAR;
		types[TYPE_UNSIGNED_SHORT] = PLY_USHORT;
		types[TYPE_UNSIGNED_INT] = PLY_UINT;
		types[TYPE_FLOAT] = PLY_FLOAT;
		types[TYPE_DOUBLE] = PLY_DOUBLE;
	}

	PlyFile* plyFile;
	int file_type;
	float version;
	int elementCount;
	char** elementNames;
	std::unordered_map<int, int> types;
};


PlyReader::PlyReader(std::string filename) :
	m_filename(filename), m_data(new Data())
{
	m_data->plyFile = ply_open_for_reading(
		filename.data(),
		&m_data->elementCount,
		&m_data->elementNames,
		&m_data->file_type,
		&m_data->version);
	
}

PlyReader::~PlyReader()
{
	if (isValid())
	{
		for (int i=0; i<m_data->elementCount; ++i)
		{
			free(m_data->elementNames[i]);
		}
		free(m_data->elementNames);

		ply_close(m_data->plyFile);
		m_data->plyFile = nullptr;
	}
}

bool PlyReader::isValid() const
{
	return m_data->plyFile != nullptr;
}

bool PlyReader::requestElement(std::string elementName,
							   std::function<void* (const std::string&, size_t)> startElementCallback,
							   std::function<void (const std::string&)> processElementCallback,
							   std::function<void (const std::string&)> endElementCallback)
{
	SURGSIM_ASSERT(isValid()) << "Invalid .ply file encountered";

	bool result = false;

	if (hasElement(elementName) && m_requestedElements.find(elementName) == m_requestedElements.end())
	{
		ElementInfo info;
		info.name = elementName;
		info.startElementCallback = startElementCallback;
		info.endElementCallback = endElementCallback;
		info.processElementCallback = processElementCallback;

		m_requestedElements[elementName] = info;
		result = true;
	}

	return result;
}

bool PlyReader::requestScalarProperty(std::string elementName, std::string propertyName, int dataType, int dataOffset)
{
	SURGSIM_ASSERT(isScalar(elementName, propertyName)) << "Trying to access a list property as a scalar." <<
		"Element: " << elementName << " Property: " << propertyName;
	return requestProperty(elementName, propertyName, dataType, dataOffset, 0, 0);
}

bool PlyReader::requestListProperty(std::string elementName,
									std::string propertyName,
									int dataType, int dataOffset,
									int countType, int countOffset)
{
	SURGSIM_ASSERT(! isScalar(elementName, propertyName)) << "Trying to access a scalar property as a list." <<
		"Element: " << elementName << " Property: " << propertyName;
	return requestProperty(elementName, propertyName, dataType, dataOffset, countType, countOffset);
}

bool PlyReader::requestProperty(std::string elementName,
								std::string propertyName,
								int dataType, int dataOffset,
								int countType, int countOffset)
{
	SURGSIM_ASSERT(isValid()) << "Invalid .ply file encountered";
	SURGSIM_ASSERT(m_requestedElements.find(elementName) != m_requestedElements.end()) <<
		"Cannot request Properties before the element has been added.";
	SURGSIM_ASSERT(dataType < TYPE_COUNT && dataType > 0) << "Invalid type used.";

	bool result = false;

	bool scalar = isScalar(elementName, propertyName);
	bool wantScalar = (countType == 0);

	if (hasProperty(elementName, propertyName) && (scalar == wantScalar))
	{
		auto itBegin = std::begin(m_requestedElements[elementName].requestedProperties);
		auto itEnd = std::end(m_requestedElements[elementName].requestedProperties);

		bool doAdd = std::find_if(itBegin, itEnd, 
			[propertyName](PropertyInfo p){return p.propertyName == propertyName;}) == itEnd;
		
		if (doAdd)
		{
			PropertyInfo info;
			info.propertyName = propertyName;
			info.targetType = m_data->types[dataType];
			info.dataOffset = dataOffset;
			info.countType = m_data->types[countType];
			info.countOffset = countOffset;
			m_requestedElements[elementName].requestedProperties.push_back(info);
			result = true;
		}
	}

	return result;
}

bool PlyReader::setDelegate(std::shared_ptr<PlyReaderDelegate> delegate)
{
	SURGSIM_ASSERT(isValid()) << "Invalid .ply file encountered";

	bool result = false;
	if (delegate != nullptr)
	{
		if (delegate->fileIsAcceptable(*this))
		{
			result = delegate->registerDelegate(this);
		}
	}
	return result;

}

void PlyReader::parseFile()
{
	SURGSIM_ASSERT(isValid()) << "Cannot parse invalid file.";

	char* currentElementName;
	for (int elementIndex = 0; elementIndex < m_data->elementCount; ++elementIndex)
	{
		currentElementName = m_data->elementNames[elementIndex];

		int numberOfElements;
		int propertyCount;

		// Free this after we are done with it
		PlyProperty** properties = 
			ply_get_element_description(m_data->plyFile, currentElementName, &numberOfElements, &propertyCount);

		// Check if the user wanted this element, if yes process
		if (m_requestedElements.find(currentElementName) != m_requestedElements.end())
		{
			ElementInfo& elementInfo = m_requestedElements[currentElementName];

			// Build the propertyinfo structure
			for (size_t propertyIndex = 0; propertyIndex < elementInfo.requestedProperties.size(); ++propertyIndex)
			{
				PropertyInfo& propertyInfo = elementInfo.requestedProperties[propertyIndex];
				PlyProperty requestedProperty = {nullptr, 0, 0, 0, 0, 0, 0, 0};

				// Create temp char*
				std::vector<char> writable(propertyInfo.propertyName.size() + 1);
				std::copy(propertyInfo.propertyName.begin(), propertyInfo.propertyName.end(), writable.begin());
				requestedProperty.name = &writable[0];
				requestedProperty.internal_type = propertyInfo.targetType;
				requestedProperty.offset = propertyInfo.dataOffset;
				requestedProperty.count_internal = propertyInfo.countType;
				requestedProperty.count_offset = propertyInfo.countOffset;
				requestedProperty.is_list = (propertyInfo.countType != 0) ? PLY_LIST : PLY_SCALAR;

				// Tell ply that we want this property to be read and put into the readbuffer
				ply_get_property(m_data->plyFile, currentElementName, &requestedProperty);
			}

			void* readBuffer = elementInfo.startElementCallback(currentElementName, numberOfElements);

			for (int element = 0; element < numberOfElements; ++element)
			{
				ply_get_element(m_data->plyFile, readBuffer);
				if (elementInfo.processElementCallback != nullptr)
				{
					elementInfo.processElementCallback(currentElementName);
				}
			}

			if (elementInfo.endElementCallback != nullptr)
			{
				elementInfo.endElementCallback(currentElementName);
			}
		}
		else
		{
			// Inefficient way to skip an element, but there does not seem to be an
			// easy way to ignore an element
			PlyOtherElems* other = ply_get_other_element(m_data->plyFile, currentElementName, numberOfElements);

		}

		// Free the data allocated in the ply_get_element_description call
		for (int i = 0; i < propertyCount; ++i)
		{
			free(properties[i]->name);
			free(properties[i]);
		}
		free(properties);
	}
}

bool PlyReader::hasElement(std::string elementName) const
{
	SURGSIM_ASSERT(isValid()) << "Invalid .ply file encountered";

	return find_element(m_data->plyFile, elementName.c_str()) != nullptr;
}

bool PlyReader::hasProperty(std::string elementName, std::string propertyName) const
{
	SURGSIM_ASSERT(isValid()) << "Invalid .ply file encountered";

	bool result = false;
	PlyElement* element = find_element(m_data->plyFile, elementName.c_str());
	if (element != nullptr)
	{
		int index;
		result = find_property(element, propertyName.c_str(), &index) != nullptr;
	}
	return result;
}

bool PlyReader::isScalar(std::string elementName, std::string propertyName) const
{
	SURGSIM_ASSERT(isValid()) << "Invalid .ply file encountered";

	bool result = false;
	PlyElement* element = find_element(m_data->plyFile, elementName.c_str());
	if (element != nullptr)
	{
		int index;
		PlyProperty* property = find_property(element, propertyName.c_str(), &index);
		if (property != nullptr)
		{
			result = (property->is_list == PLY_SCALAR);
		}
	}
	return result;
}

}
}

