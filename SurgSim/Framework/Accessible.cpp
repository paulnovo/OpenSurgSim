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

#include "SurgSim/Framework/Accessible.h"
#include "SurgSim/Math/Matrix.h"

namespace SurgSim
{
namespace Framework
{

boost::any Framework::Accessible::getValue(const std::string& name)
{
	auto element = m_getters.find(name);
	if (element != std::end(m_getters))
	{
		return element->second();
	}
	else
	{
		SURGSIM_FAILURE() << "No property with name: " << name <<" found.";
		return boost::any();
	}
}

void Framework::Accessible::setValue(const std::string& name, const boost::any& value)
{
	auto element = m_setters.find(name);
	if (element != std::end(m_setters))
	{
		element->second(value);
	}
	else
	{
		SURGSIM_FAILURE() << "Can't set property with name: " << name << ".";
	}
}


void Accessible::setGetter(const std::string& name, GetterType func)
{
	m_getters[name] = func;
}

void Accessible::setSetter(const std::string& name, SetterType func)
{
	m_setters[name] = func;
}

void Accessible::setAccessors(const std::string& name, GetterType getter, SetterType setter)
{
	setGetter(name, getter);
	setSetter(name, setter);
}

bool Accessible::isReadable(const std::string& name) const
{
	return (m_getters.cend() != m_getters.find(name));
}

bool Accessible::isWriteable(const std::string& name) const
{
	return (m_setters.cend() != m_setters.find(name));
}

void Accessible::setSerializable(const std::string& name, EncoderType encoder, DecoderType decoder)
{
	SURGSIM_ASSERT(encoder != nullptr) << "Encoder functor can't be nullptr.";
	SURGSIM_ASSERT(decoder != nullptr) << "Decoder functor can't be nullptr.";

	m_functors[name].encoder = encoder;
	m_functors[name].decoder = decoder;
}

YAML::Node Accessible::encode()
{
	YAML::Node result;
	for (auto functors = m_functors.cbegin(); functors != m_functors.cend(); ++functors)
	{
		auto encoder = functors->second.encoder;
		if (encoder != nullptr)
		{
			result[functors->first] = encoder();
		}
	}
	return result;
}

void Accessible::decode(const YAML::Node& node)
{
	for (auto functors = m_functors.cbegin(); functors != m_functors.cend(); ++functors)
	{
		auto decoder = functors->second.decoder;
		if (decoder != nullptr)
		{
			decoder(&node[functors->first]);
		}
	}
}

template<>
SurgSim::Math::Matrix44f convert(boost::any val)
{

	SurgSim::Math::Matrix44f floatResult;
	// Use try in case this conversion was created using a Matrix44f, in which case the any_cast will
	// still fail and throw an exception
	try
	{
		SurgSim::Math::Matrix44d result = boost::any_cast<SurgSim::Math::Matrix44d>(val);
		floatResult = result.cast<float>();
	}
	catch (boost::bad_any_cast &)
	{
		floatResult = boost::any_cast<SurgSim::Math::Matrix44f>(val);
	}
	return floatResult;
}


}; // Framework
}; // SurgSim
