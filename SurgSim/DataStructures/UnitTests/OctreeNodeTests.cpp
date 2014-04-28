// This file is a part of the OpenSurgSim project.
// Copyright 2012-2013, SimQuest Solutions Inc.
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

/// \file
/// Tests for the OctreeNode class.

#include "gtest/gtest.h"
#include <array>
#include <memory>
#include <string>

#include "SurgSim/DataStructures/EmptyData.h"
#include "SurgSim/DataStructures/OctreeNode.h"
#include "SurgSim/Math/Vector.h"

using SurgSim::DataStructures::EmptyData;
using SurgSim::DataStructures::OctreeNode;
using SurgSim::Math::Vector3d;

struct MockData
{
	int mockInt;
	double mockDouble;
	std::string mockString;
};

typedef OctreeNode<MockData> OctreeNodeType;

namespace
{
const double epsilon = 1e-14;
}

TEST(OctreeNodeTests, CanConstruct)
{
	SurgSim::Math::Aabbd boundingBox(Vector3d::Zero(), Vector3d::Ones());

	EXPECT_NO_THROW({OctreeNodeType octree(boundingBox);});
	EXPECT_NO_THROW(std::make_shared<OctreeNodeType>(boundingBox));
}

TEST(OctreeNodeTests, InitialValues)
{
	SurgSim::Math::Aabbd expectedBoundingBox(Vector3d::Zero(), Vector3d::Ones());
	OctreeNodeType octree(expectedBoundingBox);

	EXPECT_FALSE(octree.isActive());
	EXPECT_FALSE(octree.hasChildren());
	EXPECT_TRUE(expectedBoundingBox.isApprox(octree.getBoundingBox()));

	auto children = octree.getChildren();
	for (auto child = children.cbegin(); child != children.cend(); ++child)
	{
		EXPECT_EQ(nullptr, *child);
	}
}

TEST(OctreeNodeTests, Subdivide)
{
	SurgSim::Math::Aabbd boundingBox(Vector3d::Zero(), Vector3d::Ones() * 16.0);
	OctreeNodeType octree(boundingBox);

	EXPECT_FALSE(octree.hasChildren());
	EXPECT_FALSE(octree.isActive());
	octree.subdivide();
	EXPECT_TRUE(octree.hasChildren());
	EXPECT_FALSE(octree.isActive());

	auto children = octree.getChildren();
	for (auto child = children.cbegin(); child != children.cend(); ++child)
	{
		ASSERT_NE(nullptr, *child);
		EXPECT_FALSE((*child)->isActive());
		EXPECT_FALSE((*child)->hasChildren());
	}

	std::array<SurgSim::Math::Aabbd, 8> expectedBoxes = {{
			SurgSim::Math::Aabbd(Vector3d(0.0, 0.0, 0.0), Vector3d(8.0, 8.0, 8.0)),
			SurgSim::Math::Aabbd(Vector3d(0.0, 0.0, 8.0), Vector3d(8.0, 8.0, 16.0)),
			SurgSim::Math::Aabbd(Vector3d(0.0, 8.0, 0.0), Vector3d(8.0, 16.0, 8.0)),
			SurgSim::Math::Aabbd(Vector3d(0.0, 8.0, 8.0), Vector3d(8.0, 16.0, 16.0)),
			SurgSim::Math::Aabbd(Vector3d(8.0, 0.0, 0.0), Vector3d(16.0, 8.0, 8.0)),
			SurgSim::Math::Aabbd(Vector3d(8.0, 0.0, 8.0), Vector3d(16.0, 8.0, 16.0)),
			SurgSim::Math::Aabbd(Vector3d(8.0, 8.0, 0.0), Vector3d(16.0, 16.0, 8.0)),
			SurgSim::Math::Aabbd(Vector3d(8.0, 8.0, 8.0), Vector3d(16.0, 16.0, 16.0))
		}
	};
	for (auto expectedBox = expectedBoxes.cbegin(); expectedBox != expectedBoxes.cend(); ++expectedBox)
	{
		bool boxFound = false;
		for (auto child = children.cbegin(); child != children.cend(); ++child)
		{
			if (expectedBox->isApprox((*child)->getBoundingBox()))
			{
				boxFound = true;
				break;
			}
		}
		EXPECT_TRUE(boxFound);
	}
}

int countOctreeLevels(std::shared_ptr<OctreeNodeType> node)
{
	if (node->hasChildren())
	{
		auto children = node->getChildren();
		int maxLevels = 0;
		for (auto child = children.cbegin(); child != children.cend(); ++child)
		{
			if ((*child)->isActive())
			{
				int levels = countOctreeLevels(*child);
				if (levels > maxLevels)
				{
					maxLevels = levels;
				}
			}
		}
		return maxLevels + 1;
	}
	return 1;
}

TEST(OctreeNodeTests, AddNodes)
{
	SurgSim::Math::Aabbd boundingBox(Vector3d::Ones() * (-8.0), Vector3d::Ones() * 8.0);
	std::shared_ptr<OctreeNodeType> octree = std::make_shared<OctreeNodeType>(boundingBox);

	const int levels = 5;
	MockData data = {1, 3.14, "string"};

	EXPECT_FALSE(octree->hasChildren());
	EXPECT_FALSE(octree->isActive());

	EXPECT_TRUE(octree->addData(Vector3d(1.0, 1.0, 1.0), data, levels));
	EXPECT_TRUE(octree->addData(Vector3d(-4.0, 5.0, -7.0), data, levels));

	EXPECT_TRUE(octree->hasChildren());
	EXPECT_TRUE(octree->isActive());
	EXPECT_EQ(5, countOctreeLevels(octree));

	int numActive = 0;
	auto children = octree->getChildren();
	for (auto child = children.cbegin(); child != children.cend(); ++child)
	{
		if ((*child)->isActive())
		{
			numActive++;
		}
	}
	EXPECT_EQ(2, numActive);
}

TEST(OctreeNodeTests, Data)
{
	SurgSim::Math::Aabbd boundingBox(Vector3d::Ones() * (-8.0), Vector3d::Ones() * 8.0);
	OctreeNodeType octree(boundingBox);

	const int levels = 1;
	MockData expectedData = {1, 3.14, "string"};

	EXPECT_FALSE(octree.hasChildren());
	EXPECT_FALSE(octree.isActive());
	EXPECT_TRUE(octree.addData(Vector3d(1.0, 1.0, 1.0), expectedData, levels));
	EXPECT_FALSE(octree.hasChildren());
	EXPECT_TRUE(octree.isActive());

	EXPECT_EQ(expectedData.mockInt, octree.data.mockInt);
	EXPECT_NEAR(expectedData.mockDouble, octree.data.mockDouble, epsilon);
	EXPECT_EQ(expectedData.mockString, octree.data.mockString);
}

TEST(OctreeNodeTests, OctreePath)
{
	SurgSim::Math::Aabbd boundingBox(Vector3d::Ones() * (-8.0), Vector3d::Ones() * 8.0);
	std::shared_ptr<OctreeNodeType> octree = std::make_shared<OctreeNodeType>(boundingBox);

	SurgSim::DataStructures::OctreePath path;
	EXPECT_NO_THROW(octree->getNode(path));
	EXPECT_EQ(octree, octree->getNode(path));

	octree->subdivide();
	path.push_back(3);
	EXPECT_NO_THROW(octree->getNode(path));
	EXPECT_NE(nullptr, octree->getNode(path));

	path.push_back(1);
	EXPECT_THROW(octree->getNode(path), SurgSim::Framework::AssertionFailure);
}

TEST(OctreeNodeTests, CopyConstructor)
{
	struct Data1
	{
		std::string name;
	};
	struct Data2
	{
		double value;
	};

	SurgSim::Math::Aabbd boundingBox(Vector3d::Zero(), 2 * Vector3d::Ones());
	std::shared_ptr<OctreeNode<Data1>> octree1 = std::make_shared<OctreeNode<Data1>>(boundingBox);
	Data1 dataRoot = {"root"};
	octree1->addData(Vector3d(1.0, 1.0, 1.0), dataRoot, 1);
	Data1 dataChild = {"child"};
	octree1->addData(Vector3d(0.5, 0.5, 0.5), dataChild, 2);

	{
		SCOPED_TRACE("Copying with different Data Type");
		std::shared_ptr<OctreeNode<Data2>> octree2 = std::make_shared<OctreeNode<Data2>>(*octree1);
		ASSERT_NE(nullptr, octree2);
		EXPECT_TRUE(octree1->getBoundingBox().isApprox(octree2->getBoundingBox()));
		EXPECT_EQ(octree1->hasChildren(), octree2->hasChildren());
		EXPECT_EQ(octree1->isActive(), octree2->isActive());
		for (size_t i = 0; i < 8; i++)
		{
			if (octree1->getChild(i) == nullptr)
			{
				EXPECT_EQ(nullptr, octree2->getChild(i));
			}
			else
			{
				ASSERT_NE(nullptr, octree2->getChild(i));
				EXPECT_TRUE(octree1->getChild(i)->getBoundingBox().isApprox(octree2->getChild(i)->getBoundingBox()));
				EXPECT_EQ(octree1->getChild(i)->hasChildren(), octree2->getChild(i)->hasChildren());
				EXPECT_EQ(octree1->getChild(i)->isActive(), octree2->getChild(i)->isActive());
			}
		}
	}

	{
		SCOPED_TRACE("Copying with same Data Type");
		std::shared_ptr<OctreeNode<Data1>> octree2 = std::make_shared<OctreeNode<Data1>>(*octree1);
		ASSERT_NE(nullptr, octree2);
		EXPECT_TRUE(octree1->getBoundingBox().isApprox(octree2->getBoundingBox()));
		EXPECT_EQ(octree1->hasChildren(), octree2->hasChildren());
		EXPECT_EQ(octree1->isActive(), octree2->isActive());
		EXPECT_EQ(octree1->data.name, octree2->data.name);
		for (size_t i = 0; i < 8; i++)
		{
			if (octree1->getChild(i) == nullptr)
			{
				EXPECT_EQ(nullptr, octree2->getChild(i));
			}
			else
			{
				ASSERT_NE(nullptr, octree2->getChild(i));
				EXPECT_TRUE(octree1->getChild(i)->getBoundingBox().isApprox(octree2->getChild(i)->getBoundingBox()));
				EXPECT_EQ(octree1->getChild(i)->hasChildren(), octree2->getChild(i)->hasChildren());
				EXPECT_EQ(octree1->getChild(i)->isActive(), octree2->getChild(i)->isActive());
				EXPECT_EQ(octree1->getChild(i)->data.name, octree2->getChild(i)->data.name);
			}
		}
	}
}

TEST(OctreeNodeTests, EmptyData)
{
	SurgSim::Math::Aabbd boundingBox(Vector3d::Zero(), Vector3d::Ones());

	EXPECT_NO_THROW({OctreeNode<EmptyData> octree(boundingBox);});
	EXPECT_NO_THROW(std::make_shared<OctreeNode<EmptyData>>(boundingBox));
}


TEST(OctreeNodeTests, LoadOctree)
{
	std::shared_ptr<OctreeNode<SurgSim::DataStructures::EmptyData>> octree;
	EXPECT_NO_THROW(octree = SurgSim::DataStructures::loadOctree("OctreeShapeData/staple.vox"));

	ASSERT_TRUE(nullptr != octree);
	auto boundingBox = octree->getBoundingBox();

	SurgSim::Math::Vector3d boundingMin(-0.00207699998282, -0.00532899983227, -0.000403999991249);
	SurgSim::Math::Vector3d boundingMax(0.01392300001718, 0.01067100016773, 0.015596000008751);
	EXPECT_TRUE(boundingMin.isApprox(boundingBox.min()));
	EXPECT_TRUE(boundingMax.isApprox(boundingBox.max()));

	EXPECT_TRUE(octree->isActive());
	EXPECT_TRUE(octree->hasChildren());

	EXPECT_TRUE(octree->getChild(0)->isActive());
	EXPECT_TRUE(octree->getChild(0)->hasChildren());

	EXPECT_TRUE(octree->getChild(0)->getChild(2)->isActive());
	EXPECT_TRUE(octree->getChild(0)->getChild(2)->hasChildren());

	EXPECT_TRUE(octree->getChild(0)->getChild(2)->getChild(2)->isActive());
}
