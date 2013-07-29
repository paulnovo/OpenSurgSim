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


#include <gtest/gtest.h>
#include <SurgSim/Physics/UnitTests/RepresentationUtilities.h>

#include <SurgSim/Math/Vector.h>
#include <SurgSim/Math/Quaternion.h>
#include <SurgSim/Math/RigidTransform.h>

#include <SurgSim/Physics/RigidRepresentationState.h>
#include <SurgSim/Physics/RigidShapeCollisionRepresentation.h>
#include <SurgSim/Physics/RigidShape.h>
#include <SurgSim/Physics/SphereShape.h>
#include <SurgSim/Physics/CollisionRepresentation.h>
#include <SurgSim/Physics/ContactCalculation.h>
#include <SurgSim/Physics/CollisionPair.h>

using SurgSim::Math::Vector3d;
using SurgSim::Math::Quaterniond;
using SurgSim::Math::RigidTransform3d;


namespace SurgSim
{
namespace Physics
{


TEST(CollisionPairTests, InitTest)
{
	// Default Constructor, needs to work for ReuseFrepresentationy
	EXPECT_NO_THROW({CollisionPair pair;});

	std::shared_ptr<CollisionRepresentation> rep0 = makeSphereRepresentation(nullptr,1.0);
	std::shared_ptr<CollisionRepresentation> rep1 = makeSphereRepresentation(nullptr,2.0);

	EXPECT_ANY_THROW({CollisionPair pair(rep0, rep0);});
	EXPECT_ANY_THROW({CollisionPair pair(nullptr, rep0);});
	EXPECT_ANY_THROW({CollisionPair pair(nullptr, nullptr);});
	EXPECT_ANY_THROW({CollisionPair pair(rep0, nullptr);});

	ASSERT_NO_THROW({CollisionPair pair(rep0, rep1);});
	CollisionPair pair(rep0,rep1);

	EXPECT_EQ(rep0, pair.getFirst());
	EXPECT_EQ(rep1, pair.getSecond());
	EXPECT_FALSE(pair.hasContacts());
	EXPECT_FALSE(pair.isSwapped());

	std::pair<Location,Location> penetrationPoints;
	pair.addContact(1.0, Vector3d(1.0,0.0,0.0),penetrationPoints);
	EXPECT_TRUE(pair.hasContacts());
}

TEST(CollisionPairTests, SwapTest)
{
	std::shared_ptr<CollisionRepresentation> rep0 = makeSphereRepresentation(nullptr, 1.0);
	std::shared_ptr<CollisionRepresentation> rep1 = makeSphereRepresentation(nullptr, 2.0);

	CollisionPair pair(rep0,rep1);
	EXPECT_FALSE(pair.isSwapped());
	EXPECT_EQ(rep0.get(),pair.getRepresentations().first.get());
	EXPECT_EQ(rep1.get(),pair.getRepresentations().second.get());
	pair.swapRepresentations();
	EXPECT_TRUE(pair.isSwapped());
	pair.swapRepresentations();
	EXPECT_FALSE(pair.isSwapped());

	std::pair<Location,Location> penetrationPoints;

	pair.addContact(1.0, Vector3d(1.0,0.0,0.0),penetrationPoints);
	EXPECT_TRUE(pair.hasContacts());

	EXPECT_ANY_THROW(pair.swapRepresentations());
}

TEST(CollisionPairTests, setRepresentationsTest)
{
	std::shared_ptr<CollisionRepresentation> rep0 = makeSphereRepresentation(nullptr, 1.0);
	std::shared_ptr<CollisionRepresentation> rep1 = makeSphereRepresentation(nullptr, 2.0);
	std::shared_ptr<CollisionRepresentation> repA = makeSphereRepresentation(nullptr, 99.0);
	std::shared_ptr<CollisionRepresentation> repB = makeSphereRepresentation(nullptr, 100.0);

	CollisionPair pair(repA,repB);
	EXPECT_FALSE(pair.isSwapped());
	pair.swapRepresentations();

	pair.setRepresentations(rep0,rep1);

	EXPECT_EQ(rep0.get(), pair.getRepresentations().first.get());
	EXPECT_EQ(rep1.get(), pair.getRepresentations().second.get());
	EXPECT_FALSE(pair.isSwapped());
}


}; // namespace Physics
}; // namespace SurgSim
