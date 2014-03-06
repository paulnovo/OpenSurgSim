// This file is a part of the OpenSurgSim project.
// Copyright 2013, SimQuest LLC.
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

#include "SurgSim/Framework/SceneElement.h"
#include "SurgSim/Framework/Runtime.h"
#include "SurgSim/Framework/Component.h"
#include "SurgSim/Framework/Log.h"

#include "SurgSim/Framework/FrameworkConvert.h"

#include <yaml-cpp/yaml.h>

namespace SurgSim
{
namespace Framework
{

SceneElement::SceneElement(const std::string& name) : m_name(name), m_isInitialized(false)
{

}

SceneElement::~SceneElement()
{

}

bool SceneElement::addComponent(std::shared_ptr<Component> component)
{
	bool result = false;

	SURGSIM_ASSERT(component != nullptr) << "Cannot add a nullptr as a component";

	if (m_components.find(component->getName()) == m_components.end())
	{
		component->setSceneElement(getSharedPtr());
		component->setScene(m_scene);
		result = true;
		if (isInitialized())
		{
			auto runtime = getRuntime();
			result = component->initialize(runtime);
			runtime->addComponent(component);
		}
	}
	else
	{
		SURGSIM_LOG_WARNING(Logger::getLogger("runtime")) <<
				"Component with name " << component->getName() <<
				" already exists on SceneElement " << getName() <<
				", did not add component";
	}

	if (result)
	{
		m_components[component->getName()] = component;
	}

	return result;
}

bool SceneElement::removeComponent(std::shared_ptr<Component> component)
{
	return removeComponent(component->getName());
}

bool SceneElement::removeComponent(const std::string& name)
{
	bool result = false;
	if (m_components.find(name) != m_components.end())
	{
		size_t count = m_components.erase(name);
		result = (count == 1);
	}
	return result;
}

std::shared_ptr<Component> SceneElement::getComponent(const std::string& name) const
{
	std::shared_ptr<Component> result;

	auto findResult = m_components.find(name);
	if (findResult != m_components.end())
	{
		result = findResult->second;
	}
	return result;
}

bool SceneElement::initialize()
{
	SURGSIM_ASSERT(!m_isInitialized) << "Double initialization calls on SceneElement " << m_name;
	m_isInitialized = doInitialize();

	if (m_isInitialized)
	{
		// initialize all components
		std::shared_ptr<Runtime> runtime = getRuntime();
		auto component = std::begin(m_components);
		while (m_isInitialized && component != std::end(m_components))
		{
			m_isInitialized = component->second->initialize(runtime);
			++component;
		}
	}

	return m_isInitialized;
}

std::string SceneElement::getName() const
{
	return m_name;
}

std::vector<std::shared_ptr<Component>> SceneElement::getComponents() const
{
	std::vector<std::shared_ptr<Component>> result(m_components.size());
	auto componentIt = m_components.begin();
	for (int i = 0; componentIt != m_components.end(); ++componentIt, ++i)
	{
		result[i] = componentIt->second;
	}
	return result;
}

void SceneElement::setScene(std::weak_ptr<Scene> scene)
{
	m_scene = scene;

	auto it = std::begin(m_components);
	auto endIt = std::end(m_components);

	for (;  it != endIt;  ++it)
	{
		(it->second)->setScene(scene);
	}

}

std::shared_ptr<Scene> SceneElement::getScene()
{
	return m_scene.lock();
}

void SceneElement::setRuntime(std::weak_ptr<Runtime> runtime)
{
	m_runtime = runtime;
}

std::shared_ptr<Runtime> SceneElement::getRuntime()
{
	return m_runtime.lock();
}

bool SceneElement::isInitialized() const
{
	return m_isInitialized;
}

std::shared_ptr<SceneElement> SceneElement::getSharedPtr()
{
	std::shared_ptr<SceneElement> result;
	try
	{
		result = shared_from_this();
	}
	catch (const std::exception&)
	{
		SURGSIM_FAILURE() << "SceneElement was not created as a shared_ptr.";
	}
	return result;
}

void SceneElement::setName(const std::string& name)
{
	m_name = name;
}

YAML::Node SceneElement::encode(bool standalone) const
{
	YAML::Node data(YAML::NodeType::Map);
	data["Name"] = getName();

	for (auto component = std::begin(m_components); component != std::end(m_components); ++component)
	{
		if (standalone)
		{
			data["Components"].push_back(*component->second);
		}
		else
		{
			data["Components"].push_back(component->second);
		}
	}
	YAML::Node node(YAML::NodeType::Map);
	node[getClassName()] = data;
	return node;
}

bool SceneElement::decode(const YAML::Node& node)
{
	SURGSIM_ASSERT(! isInitialized()) << "Should not call decode on a SceneElement that has already been initialized.";
	bool result = false;
	if (node.IsMap())
	{
		std::string className = node.begin()->first.as<std::string>();

		SURGSIM_ASSERT(className == getClassName()) << "Wrong type for this node, wanted <" << className << ">" <<
				"but this is a <" << getClassName() << ">.";

		YAML::Node data = node[getClassName()];

		m_name = data["Name"].as<std::string>();
		if (data["Components"].IsSequence())
		{
			for (auto nodeIt = data["Components"].begin(); nodeIt != data["Components"].end(); ++nodeIt)
			{
				addComponent(nodeIt->as<std::shared_ptr<SurgSim::Framework::Component>>());
			}
			result = true;
		}
	}
	return result;
}

std::string SceneElement::getClassName() const
{
	// SURGSIM_FAILURE() << "SceneElement is abstract, this should not be called.";
	return "SurgSim::Framework::SceneElement";
}


}; // namespace Framework
}; // namespace SurgSim




