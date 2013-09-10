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
#include <SurgSim/Physics/UnitTests/MockCollisionRepresentation.h>
#include <memory>

#include <SurgSim/Math/Vector.h>
#include <SurgSim/Math/Quaternion.h>
#include <SurgSim/Math/RigidTransform.h>

#include <SurgSim/Physics/RigidRepresentationState.h>
#include <SurgSim/Physics/RigidShape.h>
#include <SurgSim/Physics/SphereShape.h>
#include <SurgSim/Physics/CollisionRepresentation.h>
#include <SurgSim/Physics/ContactCalculation.h>
#include <SurgSim/Physics/CollisionPair.h>

#include <SurgSim/Math/Geometry.h>
#include <SurgSim/Physics/RigidShapeCollisionRepresentation.h>

using SurgSim::Math::Vector3d;
using SurgSim::Math::Quaterniond;
using SurgSim::Math::RigidTransform3d;

namespace
{
double epsilon = 1e-10;
}

::testing::AssertionResult eigenEqual(const Vector3d& left, const Vector3d& right, double epsilon)
{
	double dist = (left - right).norm();
	if (std::abs(dist) < epsilon)
	{
		return ::testing::AssertionSuccess();
	}
	else
	{
		return ::testing::AssertionFailure() << std::endl << "Vectors not close, expected: " << left.transpose() <<
			   std::endl << " result: " << right.transpose() << std::endl;
	}
}

namespace SurgSim
{
namespace Physics
{


namespace
{
std::shared_ptr<RigidShape> sphereShape = std::make_shared<SphereShape>(1.0);
std::shared_ptr<RigidShape> doubleSidedPlaneShape = std::make_shared<DoubleSidedPlaneShape>();

std::shared_ptr<CollisionRepresentation> rep0 = std::make_shared<MockCollisionRepresentation>
	("TestSphere 1", sphereShape, Quaterniond::Identity(), Vector3d(1.0,0.0,0.0));
std::shared_ptr<CollisionRepresentation> rep1 = std::make_shared<MockCollisionRepresentation>
	("TestSphere 2", sphereShape, Quaterniond::Identity(), Vector3d(0.5,0.0,0.0));

std::shared_ptr<CollisionPair> pair01 = std::make_shared<CollisionPair>(rep0, rep1);
}

TEST(ContactCalculationTests, DefaultCalculation)
{
	DefaultContactCalculation calcShouldLog(false);
	EXPECT_NO_THROW(calcShouldLog.calculateContact(pair01));
	EXPECT_FALSE(pair01->hasContacts());

	DefaultContactCalculation calcShouldThrow(true);
	EXPECT_ANY_THROW(calcShouldThrow.calculateContact(pair01));
	EXPECT_FALSE(pair01->hasContacts());
}


void doSphereSphereTest(double r0, Vector3d p0, double r1, Vector3d p1, bool hasContacts,
						double expectedDepth = 0.0, Vector3d expectedNormal = Vector3d::UnitX(),
						Vector3d expectedPenetrationPoint0 = Vector3d::Zero(),
						Vector3d expectedPenetrationPoint1 = Vector3d::Zero())
{
	SphereSphereDcdContact calc;
	std::shared_ptr<CollisionPair> pair =
		std::make_shared<CollisionPair>(makeSphereRepresentation(nullptr, r0,Quaterniond::Identity(),p0),
										makeSphereRepresentation(nullptr, r1,Quaterniond::Identity(),p1));

	calc.calculateContact(pair);
	EXPECT_EQ(hasContacts, pair->hasContacts());
	if (pair->hasContacts())
	{
		std::shared_ptr<Contact> contact = pair->getContacts().front();
		EXPECT_TRUE(eigenEqual(expectedNormal, contact->normal, epsilon));
		EXPECT_NEAR(expectedDepth, contact->depth, epsilon);
		EXPECT_TRUE(contact->penetrationPoints.first.globalPosition.hasValue());
		EXPECT_TRUE(contact->penetrationPoints.second.globalPosition.hasValue());

		EXPECT_TRUE(eigenEqual(expectedPenetrationPoint0,
							   contact->penetrationPoints.first.globalPosition.getValue(),
							   epsilon));
		EXPECT_TRUE(eigenEqual(expectedPenetrationPoint1,
							   contact->penetrationPoints.second.globalPosition.getValue(),
							   epsilon));
	}
}


TEST(ContactCalculationTests, SphereSphereCalculation)
{
	{
		SCOPED_TRACE("No Intersection");
		doSphereSphereTest(0.1, Vector3d(0.0,0.0,0.0), 0.1, Vector3d(1.0,1.0,1.0), false);
	}

	{
		SCOPED_TRACE("Sphere-Sphere intersection at origin");
		doSphereSphereTest(0.5, Vector3d(-0.5+epsilon/2.0,0.0,0.0), 0.5, Vector3d(0.5-epsilon/2.0,0.0,0.0),
						   true, epsilon, Vector3d(-1.0,0.0,0.0), Vector3d::Zero(), Vector3d::Zero());
	}

	{
		SCOPED_TRACE("Sphere-Sphere intersection");
		doSphereSphereTest(0.5, Vector3d(0.0,0.0,0.0), 0.5, Vector3d(0.5,0.0,0.0), true, 0.5,
						   Vector3d(-1.0,0.0,0.0), Vector3d(0.5,0.0,0.0), Vector3d(0.0,0.0,0.0));
	}
}

void doSphereDoubleSidedPlaneTest(std::shared_ptr<SphereShape> sphere,
								  const Quaterniond& sphereQuat,
								  const Vector3d& sphereTrans,
								  std::shared_ptr<DoubleSidedPlaneShape> plane,
								  const Quaterniond& planeQuat,
								  const Vector3d& planeTrans,
								  bool expectedIntersect,
								  const double& expectedDepth = 0 ,
								  const Vector3d& expectedNorm = Vector3d::Zero())
{
	std::shared_ptr<CollisionRepresentation> planeRep = std::make_shared<MockCollisionRepresentation>(
		"Collision Plane",
		plane,
		planeQuat,
		planeTrans);
	std::shared_ptr<CollisionRepresentation> sphereRep = std::make_shared<MockCollisionRepresentation>(
		"Collision Sphere",
		sphere,
		sphereQuat,
		sphereTrans);

	SphereDoubleSidedPlaneDcdContact calcNormal;
	std::shared_ptr<CollisionPair> pair = std::make_shared<CollisionPair>(sphereRep, planeRep);

	// Again this replicates the way this is calculated in the contact calculation just with different
	// starting values
	Vector3d spherePenetration = sphereTrans - expectedNorm * sphere->getRadius();
	Vector3d planePenetration = sphereTrans - expectedNorm * (sphere->getRadius() - expectedDepth);

	calcNormal.calculateContact(pair);
	if (expectedIntersect)
	{
		ASSERT_TRUE(pair->hasContacts());
		std::shared_ptr<Contact> contact = pair->getContacts().front();
		EXPECT_NEAR(expectedDepth, contact->depth, 1e-10);
		EXPECT_TRUE(eigenEqual(expectedNorm, contact->normal, epsilon));
		EXPECT_TRUE(contact->penetrationPoints.first.globalPosition.hasValue());
		EXPECT_TRUE(contact->penetrationPoints.second.globalPosition.hasValue());
		EXPECT_TRUE(eigenEqual(spherePenetration,
							   contact->penetrationPoints.first.globalPosition.getValue(),
							   epsilon));
		EXPECT_TRUE(eigenEqual(planePenetration,
							   contact->penetrationPoints.second.globalPosition.getValue(),
							   epsilon));
	}
	else
	{
		EXPECT_FALSE(pair->hasContacts());
	}
}

TEST(ContactCalculationTests, SphereDoubleSidedPlaneCalculation)
{
	std::shared_ptr<DoubleSidedPlaneShape> plane = std::make_shared<DoubleSidedPlaneShape>();
	std::shared_ptr<SphereShape> sphere = std::make_shared<SphereShape>(1.0);

	{
		SCOPED_TRACE("No Intersection, no transformation");
		doSphereDoubleSidedPlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,2.0,0.0),
									 plane, Quaterniond::Identity(), Vector3d(0.0,0.5,0.0), false);
	}

	{
		SCOPED_TRACE("Intersection front, no transformation");
		doSphereDoubleSidedPlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,1.0,0.0),
									 plane,Quaterniond::Identity(), Vector3d(0.0,0.5,0.0),
									 true, 0.5, Vector3d(0.0,1.0,0.0));
	}

	{
		SCOPED_TRACE("Intersection back, no transformation");
		doSphereDoubleSidedPlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,0.0,0.0),
									 plane, Quaterniond::Identity(), Vector3d(0.0,0.5,0.0),
									 true, 0.5, Vector3d(0.0,-1.0,0.0));
	}

	{
		SCOPED_TRACE("Intersection front, sphere center on the plane, rotated plane");
		doSphereDoubleSidedPlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,0,0.0),
									 plane, SurgSim::Math::makeRotationQuaternion(M_PI_2, Vector3d(1.0,0.0,0.0)),
									 Vector3d(0.0,0.0,0.0), true, 1.0, Vector3d(0.0,0.0,1.0));
	}

	{
		SCOPED_TRACE("Intersection front, rotated Plane");
		doSphereDoubleSidedPlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,0.0,0.5),
									 plane, SurgSim::Math::makeRotationQuaternion(M_PI_2, Vector3d(1.0,0.0,0.0)),
									 Vector3d(0.0,0.0,0.0), true, 0.5, Vector3d(0.0,0.0,1.0));
	}

	{
		Vector3d planeTrans(365.321,-342.324,325.324);
		Quaterniond planeQuat =
					SurgSim::Math::makeRotationQuaternion(1.23456, Vector3d(0.234,-0.986,0.646).normalized());
		SCOPED_TRACE("Intersection front, rotated plane 2");
		doSphereDoubleSidedPlaneTest(sphere, Quaterniond::Identity(), planeQuat * (Vector3d(0.0,0.5,0.0)) + planeTrans,
								     plane, planeQuat, planeTrans, true, 0.5, planeQuat * Vector3d(0.0, 1.0, 0.0));
	}
}

TEST(ContactCalculationTests, DoubleSidedPlaneSphereShouldFail)
{
	std::shared_ptr<CollisionRepresentation> reps0 = std::make_shared<MockCollisionRepresentation>(
		"Collision Sphere 0",
		sphereShape,
		Quaterniond::Identity(),
		Vector3d(1.0,0.0,0.0));

	std::shared_ptr<CollisionRepresentation> repp0 = std::make_shared<MockCollisionRepresentation>(
		"Collision Plane 0",
		doubleSidedPlaneShape,
		Quaterniond::Identity(),
		Vector3d(0.5,0.0,0.0));

	std::shared_ptr<CollisionRepresentation> reps1 = std::make_shared<MockCollisionRepresentation>(
		"Collision Sphere 1",
		sphereShape,
		Quaterniond::Identity(),
		Vector3d(1.0,0.0,0.0));

	std::shared_ptr<CollisionRepresentation> repp1 = std::make_shared<MockCollisionRepresentation>(
		"Collision Plane 1",
		doubleSidedPlaneShape,
		Quaterniond::Identity(),
		Vector3d(0.5,0.0,0.0));

	std::shared_ptr<CollisionPair> pairpp = std::make_shared<CollisionPair>(repp0, repp1);
	std::shared_ptr<CollisionPair> pairss = std::make_shared<CollisionPair>(reps0, reps1);

	SphereDoubleSidedPlaneDcdContact contact;

	EXPECT_ANY_THROW(contact.calculateContact(pairpp));
	EXPECT_ANY_THROW(contact.calculateContact(pairss));
}

void doSpherePlaneTest(std::shared_ptr<SphereShape> sphere,
					   const Quaterniond& sphereQuat,
					   const Vector3d& sphereTrans,
					   std::shared_ptr<PlaneShape> plane,
					   const Quaterniond& planeQuat,
					   const Vector3d& planeTrans,
					   bool expectedIntersect,
					   const double& expectedDepth = 0,
					   const Vector3d& expectedNorm = Vector3d::Zero())
{
	std::shared_ptr<CollisionRepresentation> planeRep =
		std::make_shared<MockCollisionRepresentation>("Plane Shape",plane,planeQuat,planeTrans);
	std::shared_ptr<CollisionRepresentation> sphereRep =
		std::make_shared<MockCollisionRepresentation>("Sphere Shape",sphere,sphereQuat,sphereTrans);

	SpherePlaneDcdContact calcNormal;
	std::shared_ptr<CollisionPair> pair = std::make_shared<CollisionPair>(sphereRep, planeRep);

	// Again this replicates the way this is calculated in the contact calculation just with different
	// starting values
	Vector3d spherePenetration = sphereTrans - expectedNorm * sphere->getRadius();
	Vector3d planePenetration = sphereTrans - expectedNorm * (sphere->getRadius() - expectedDepth);

	calcNormal.calculateContact(pair);
	if (expectedIntersect)
	{
		ASSERT_TRUE(pair->hasContacts());
		std::shared_ptr<Contact> contact = pair->getContacts().front();
		EXPECT_NEAR(expectedDepth, contact->depth, 1e-10);
		EXPECT_TRUE(eigenEqual(expectedNorm, contact->normal, epsilon));
		EXPECT_TRUE(contact->penetrationPoints.first.globalPosition.hasValue());
		EXPECT_TRUE(contact->penetrationPoints.second.globalPosition.hasValue());
		EXPECT_TRUE(eigenEqual(spherePenetration,
							   contact->penetrationPoints.first.globalPosition.getValue(),
							   epsilon));
		EXPECT_TRUE(eigenEqual(planePenetration,
							   contact->penetrationPoints.second.globalPosition.getValue(),
							   epsilon));
	}
	else
	{
		EXPECT_FALSE(pair->hasContacts());
	}
}

TEST(ContactCalculationTests, SpherePlaneCalculation)
{
	std::shared_ptr<PlaneShape> plane = std::make_shared<PlaneShape>();
	std::shared_ptr<SphereShape> sphere = std::make_shared<SphereShape>(1.0);

	{
		SCOPED_TRACE("No Intersection, no transformation");
		doSpherePlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,2.0,0.0),
						  plane, Quaterniond::Identity(), Vector3d(0.0,0.5,0.0), false);
	}

	{
		SCOPED_TRACE("Intersection front, no transformation");
		doSpherePlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,1.0,0.0),
						  plane,Quaterniond::Identity(), Vector3d(0.0,0.5,0.0),
						  true, 0.5, Vector3d(0.0,1.0,0.0));
	}

	{
		SCOPED_TRACE("Intersection back, no transformation");
		doSpherePlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,0.0,0.0),
						  plane, Quaterniond::Identity(), Vector3d(0.0,0.5,0.0),
						  true, 1.5, Vector3d(0.0,1.0,0.0));
	}

	{
		SCOPED_TRACE("Intersection front, sphere center on the plane, rotated plane");
		doSpherePlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,0,0.0),
						  plane, SurgSim::Math::makeRotationQuaternion(M_PI_2, Vector3d(1.0,0.0,0.0)),
						  Vector3d(0.0,0.0,0.0), true, 1.0, Vector3d(0.0,0.0,1.0));
	}

	{
		SCOPED_TRACE("Intersection front, rotated Plane");
		doSpherePlaneTest(sphere, Quaterniond::Identity(), Vector3d(0.0,0.0,0.5),
						  plane, SurgSim::Math::makeRotationQuaternion(M_PI_2, Vector3d(1.0,0.0,0.0)),
						  Vector3d(0.0,0.0,0.0), true, 0.5, Vector3d(0.0,0.0,1.0));
	}
}

::testing::AssertionResult isContactPresentInList(std::shared_ptr<Contact> expected,
                                                  const std::list<std::shared_ptr<Contact>>& contactsList)
{
    using SurgSim::Math::Geometry::ScalarEpsilon;

    bool contactPresent = false;
    for (auto it = contactsList.begin(); it != contactsList.end() && !contactPresent; ++it)
    {
        // Compare the normals.
        contactPresent = eigenEqual(expected->normal, it->get()->normal, ScalarEpsilon);
        // Compare the global position of first object.
        contactPresent &= eigenEqual(expected->penetrationPoints.first.globalPosition.getValue(),
                                     it->get()->penetrationPoints.first.globalPosition.getValue(), ScalarEpsilon);
        // Compare the global position of second object.
        contactPresent &= eigenEqual(expected->penetrationPoints.second.globalPosition.getValue(),
                                     it->get()->penetrationPoints.second.globalPosition.getValue(),
                                     ScalarEpsilon);
        // Compare the depth.
        contactPresent &= std::abs(expected->depth - it->get()->depth) <= ScalarEpsilon;
    }

    if (contactPresent)
    {
        return ::testing::AssertionSuccess();
    }
    else
    {
        return ::testing::AssertionFailure() << "Expected contact not found in calculated contacts list:\n" <<
               "Normal: " << expected->normal << "\n" <<
               "First objects' contact point: " << expected->penetrationPoints.first.globalPosition.getValue()
               << "\n" <<
               "Second objects' contact point: " << expected->penetrationPoints.second.globalPosition.getValue()
               << "\n" <<
               "Depth of penetration: " << expected->depth << "\n";
    }
}

void contactsInfoEqualityTest(const std::list<std::shared_ptr<Contact>>& expectedContacts,
                              const std::list<std::shared_ptr<Contact>>& calculatedContacts)
{
    SCOPED_TRACE("Comparing the contact info.");

    EXPECT_EQ(expectedContacts.size(), calculatedContacts.size());

    for (auto it = expectedContacts.begin(); it != expectedContacts.end(); ++it)
    {
        EXPECT_TRUE(isContactPresentInList(*it, calculatedContacts));
    }
}

Vector3d calculateBoxVertex(const int i, const double* size,
                            const Quaterniond& quat,
                            const Vector3d& trans)
{
    static const double multiplier[8][3] = {{-0.5, -0.5, -0.5}, {-0.5, -0.5, 0.5}, {-0.5, 0.5, 0.5}, {-0.5, 0.5, -0.5},
        {0.5, -0.5, -0.5}, {0.5, -0.5, 0.5}, {0.5, 0.5, 0.5}, {0.5, 0.5, -0.5}
    };

    return (quat * Vector3d(size[0] * multiplier[i][0], size[1] * multiplier[i][1], size[2] * multiplier[i][2])) +
           trans;
}

void generateBoxPlaneContact(std::list<std::shared_ptr<Contact>>& expectedContacts,
							 const int expectedNumberOfContacts, const int* expectedBoxIndicesInContacts,
							 const double* size,
							 const Vector3d& boxTrans, const Quaterniond& boxQuat,
							 const Vector3d& planeNormal, const double planeD,
							 const Vector3d& planeTrans, const Quaterniond& planeQuat)
{
    Vector3d vertex;
    Vector3d planeNormalGlobal = planeQuat * planeNormal;
    Vector3d pointOnPlane = planeTrans + (planeNormalGlobal * planeD);
    double depth = 0.0;
    Vector3d collisionNormal = planeNormalGlobal;
    for (int i = 0; i < expectedNumberOfContacts; ++i)
    {
        vertex = calculateBoxVertex(expectedBoxIndicesInContacts[i], size, boxQuat, boxTrans);
        std::pair<Location, Location> penetrationPoint;
        penetrationPoint.first.globalPosition.setValue(vertex);
        depth = planeNormalGlobal.dot(vertex - pointOnPlane);
        penetrationPoint.second.globalPosition.setValue(vertex - planeNormalGlobal * depth);
        expectedContacts.push_back(std::make_shared<Contact>(depth, Vector3d::Zero(),
                                                             collisionNormal, penetrationPoint));
    }
}

void doBoxPlaneTest(std::shared_ptr<BoxShape> box,
					const Quaterniond& boxQuat,
					const Vector3d& boxTrans,
					std::shared_ptr<PlaneShape> plane,
					const Quaterniond& planeQuat,
					const Vector3d& planeTrans,
					const int expectedNumberOfContacts,
					const int* expectedBoxIndicesInContacts)
{
    std::shared_ptr<CollisionRepresentation> boxRep = std::make_shared<MockCollisionRepresentation>(
		"Collision Box 0",
		box,
		boxQuat,
		boxTrans);
    std::shared_ptr<CollisionRepresentation> planeRep = std::make_shared<MockCollisionRepresentation>(
		"Collision Plane 0",
		plane,
		planeQuat,
		planeTrans);

    // First calculate the expected contact info.
    std::list<std::shared_ptr<Contact>> expectedContacts;
    if (expectedNumberOfContacts > 0)
    {
        double boxSize[] = {box->getSizeX(), box->getSizeY(), box->getSizeZ()};
        generateBoxPlaneContact(expectedContacts, expectedNumberOfContacts, expectedBoxIndicesInContacts,
								boxSize, boxTrans, boxQuat, plane->getNormal(), plane->getD(), planeTrans,
								planeQuat);
    }

    // Perform collision detection.
    BoxPlaneDcdContact calcContact;
    std::shared_ptr<CollisionPair> pair = std::make_shared<CollisionPair>(boxRep, planeRep);
    calcContact.calculateContact(pair);

    // Compare the contact info.
    contactsInfoEqualityTest(expectedContacts, pair->getContacts());
}

TEST(ContactCalculationTests, BoxPlaneCalculation)
{
    std::shared_ptr<BoxShape> box = std::make_shared<BoxShape>(1.0, 1.0, 1.0);
    std::shared_ptr<PlaneShape> plane = std::make_shared<PlaneShape>();
    SurgSim::Math::Quaterniond boxQuat;
    SurgSim::Math::Vector3d boxTrans;
    SurgSim::Math::Quaterniond planeQuat;
    SurgSim::Math::Vector3d planeTrans;
	SurgSim::Math::Quaterniond globalQuat;

    {
        SCOPED_TRACE("No intersection, box in front of rotated plane");
        boxQuat = SurgSim::Math::makeRotationQuaternion(0.5674, Vector3d(0.4332,0.927, 0.13557).normalized());
        boxTrans = Vector3d(3.4535,10.0,350.0);
        planeQuat = SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = Vector3d::Zero();
        int expectedNumberOfContacts = 0;
        int expectedBoxIndicesInContacts[] = {0};
        doBoxPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
					   expectedBoxIndicesInContacts);
    }

    {
        SCOPED_TRACE("Intersection in front of plane, four contacts, rotated plane");
        boxQuat = SurgSim::Math::makeRotationQuaternion(1.233469, Vector3d(0.91834,0.39687,0.8271).normalized());
        boxTrans = Vector3d(0.5,10.0,350.0);
        planeQuat = boxQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + boxQuat * Vector3d(-0.5,0.0,0.0);
        int expectedNumberOfContacts = 4;
        int expectedBoxIndicesInContacts[] = {0, 1, 2, 3};
        doBoxPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
					   expectedBoxIndicesInContacts);
    }

    {
        SCOPED_TRACE("Intersection in front of plane, two contacts, rotated plane");
		globalQuat = SurgSim::Math::makeRotationQuaternion(0.8753, Vector3d(0.235345,0.6754,0.4567).normalized());
        boxQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_4, Vector3d(0.0,0.0,1.0));
        boxTrans = Vector3d(std::sqrt(0.5),230.0,540.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(-std::sqrt(0.5),0.0,0.0);
        int expectedNumberOfContacts = 2;
        int expectedBoxIndicesInContacts[] = {0, 1};
        doBoxPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
					   expectedBoxIndicesInContacts);
    }

    {
        SCOPED_TRACE("Intersection in front of plane, one contact, rotated plane");
        globalQuat = SurgSim::Math::makeRotationQuaternion(-0.3257, Vector3d(-0.4575,-0.8563,0.63457).normalized());
        double angle = -35.264389682754654315377000330019*(M_PI/180.0);
        boxQuat = globalQuat * Quaterniond(SurgSim::Math::makeRotationMatrix(angle, Vector3d(0.0,1.0,0.0)) *
										   SurgSim::Math::makeRotationMatrix(-M_PI_4, Vector3d(0.0,0.0,1.0)));
        boxTrans = Vector3d(std::sqrt(0.75),0.0,0.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(-std::sqrt(0.75),0.0,0.0);
        int expectedNumberOfContacts = 1;
        int expectedBoxIndicesInContacts[] = {1};
        doBoxPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
					   expectedBoxIndicesInContacts);
    }

	{
        SCOPED_TRACE("Intersection inside of plane, one contact, rotated plane");
        globalQuat = SurgSim::Math::makeRotationQuaternion(0.3465, Vector3d(54.4575,76.8563,43.63457).normalized());
        double angle = -35.264389682754654315377000330019*(M_PI/180.0);
        boxQuat = globalQuat * Quaterniond(SurgSim::Math::makeRotationMatrix(angle, Vector3d(0.0,1.0,0.0)) *
										   SurgSim::Math::makeRotationMatrix(-M_PI_4, Vector3d(0.0,0.0,1.0)));
        boxTrans = Vector3d(std::sqrt(0.73),0.0,0.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(-std::sqrt(0.75),0.0,0.0);
        int expectedNumberOfContacts = 1;
        int expectedBoxIndicesInContacts[] = {1};
        doBoxPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
					   expectedBoxIndicesInContacts);
    }

	{
        SCOPED_TRACE("Intersection in front of plane, two contacts, rotated plane");
		globalQuat = SurgSim::Math::makeRotationQuaternion(-0.8753, Vector3d(-1.235345,1.6754,1.4567).normalized());
        boxQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_4, Vector3d(0.0,0.0,1.0));
        boxTrans = Vector3d(std::sqrt(0.45),230.0,540.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(-std::sqrt(0.5),0.0,0.0);
        int expectedNumberOfContacts = 2;
        int expectedBoxIndicesInContacts[] = {0, 1};
        doBoxPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
					   expectedBoxIndicesInContacts);
    }

	{
        SCOPED_TRACE("Intersection inside of plane, four contacts, rotated plane");
        boxQuat = SurgSim::Math::makeRotationQuaternion(.99763, Vector3d(0.19834,0.93687,0.2871).normalized());
        boxTrans = Vector3d(0.23,10.0,350.0);
        planeQuat = boxQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + boxQuat * Vector3d(-0.5,0.0,0.0);
        int expectedNumberOfContacts = 4;
        int expectedBoxIndicesInContacts[] = {0, 1, 2, 3};
        doBoxPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
					   expectedBoxIndicesInContacts);
    }

	{
        SCOPED_TRACE("Intersection inside of plane - case 1, eight contacts, rotated plane");
		globalQuat = SurgSim::Math::makeRotationQuaternion(-0.8753, Vector3d(-1.235345,1.6754,1.4567).normalized());
        boxQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_4, Vector3d(0.0,0.0,1.0));
        boxTrans = Vector3d(0.435,230.0,540.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(9.43523,0.0,0.0);
        int expectedNumberOfContacts = 8;
        int expectedBoxIndicesInContacts[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
        doBoxPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
					   expectedBoxIndicesInContacts);
    }

	{
        SCOPED_TRACE("Intersection inside of plane - case 2, eight contacts, rotated plane");
		globalQuat = SurgSim::Math::makeRotationQuaternion(1.4576, Vector3d(23.45,-98.24,42.46).normalized());
        double angle = -35.264389682754654315377000330019*(M_PI/180.0);
        boxQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(angle, Vector3d(0.0,0.0,1.0)) *
                  SurgSim::Math::makeRotationQuaternion(-M_PI_4, Vector3d(0.0,1.0,0.0));
        boxTrans = Vector3d(0.34,0.0,0.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(5.345,0.0,0.0);
        int expectedNumberOfContacts = 8;
        int expectedBoxIndicesInContacts[] = {0, 1, 2, 3, 4, 5, 6, 7};
        doBoxPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
					   expectedBoxIndicesInContacts);
    }
}

void generateBoxDoubleSidedPlaneContact(std::list<std::shared_ptr<Contact>>& expectedContacts,
										const int expectedNumberOfContacts, const int* expectedBoxIndicesInContacts,
										const double* size,
										const Vector3d& boxTrans, const Quaterniond& boxQuat,
										const Vector3d& planeNormal, const double planeD,
										const Vector3d& planeTrans, const Quaterniond& planeQuat,
										const bool collisionNormalIsPlaneNormal)
{
    Vector3d vertex;
    Vector3d planeNormalGlobal = planeQuat * planeNormal;
    Vector3d pointOnPlane = planeTrans + (planeNormalGlobal * planeD);
    double depth = 0.0;
    Vector3d collisionNormal = planeNormalGlobal * (collisionNormalIsPlaneNormal ? 1.0 : -1.0);
    for (int i = 0; i < expectedNumberOfContacts; ++i)
    {
        vertex = calculateBoxVertex(expectedBoxIndicesInContacts[i], size, boxQuat, boxTrans);
        std::pair<Location, Location> penetrationPoint;
        penetrationPoint.first.globalPosition.setValue(vertex);
        depth = planeNormalGlobal.dot(vertex - pointOnPlane);
        penetrationPoint.second.globalPosition.setValue(vertex - planeNormalGlobal * depth);
        expectedContacts.push_back(std::make_shared<Contact>(std::abs(depth), Vector3d::Zero(),
                                                             collisionNormal, penetrationPoint));
    }
}

void doBoxDoubleSidedPlaneTest(std::shared_ptr<BoxShape> box,
							   const Quaterniond& boxQuat,
							   const Vector3d& boxTrans,
							   std::shared_ptr<DoubleSidedPlaneShape> plane,
							   const Quaterniond& planeQuat,
							   const Vector3d& planeTrans,
							   const int expectedNumberOfContacts,
							   const int* expectedBoxIndicesInContacts,
							   const bool collisionNormalIsPlaneNormal)
{
    std::shared_ptr<CollisionRepresentation> boxRep = std::make_shared<MockCollisionRepresentation>(
		"Collision Box 0",
		box,
		boxQuat,
		boxTrans);
    std::shared_ptr<CollisionRepresentation> planeRep = std::make_shared<MockCollisionRepresentation>(
		"Collision Plane 0",
		plane,
		planeQuat,
		planeTrans);

    // First calculate the expected contact info.
    std::list<std::shared_ptr<Contact>> expectedContacts;
    if (expectedNumberOfContacts > 0)
    {
        double boxSize[] = {box->getSizeX(), box->getSizeY(), box->getSizeZ()};
        generateBoxDoubleSidedPlaneContact(expectedContacts, expectedNumberOfContacts, expectedBoxIndicesInContacts,
										   boxSize, boxTrans, boxQuat, plane->getNormal(), plane->getD(), planeTrans,
										   planeQuat, collisionNormalIsPlaneNormal);
    }

    // Perform collision detection.
    BoxDoubleSidedPlaneDcdContact calcContact;
    std::shared_ptr<CollisionPair> pair = std::make_shared<CollisionPair>(boxRep, planeRep);
    calcContact.calculateContact(pair);

    // Compare the contact info.
    contactsInfoEqualityTest(expectedContacts, pair->getContacts());
}

TEST(ContactCalculationTests, BoxDoubleSidedPlaneCalculation)
{
    std::shared_ptr<BoxShape> box = std::make_shared<BoxShape>(1.0, 1.0, 1.0);
    std::shared_ptr<DoubleSidedPlaneShape> plane = std::make_shared<DoubleSidedPlaneShape>();
    SurgSim::Math::Quaterniond boxQuat;
    SurgSim::Math::Vector3d boxTrans;
    SurgSim::Math::Quaterniond planeQuat;
    SurgSim::Math::Vector3d planeTrans;
	SurgSim::Math::Quaterniond globalQuat;

    {
        SCOPED_TRACE("No intersection, box in front of rotated plane");
        boxQuat = SurgSim::Math::makeRotationQuaternion(0.5674, Vector3d(0.4332,0.927, 0.13557).normalized());
        boxTrans = Vector3d(2.5,10.0,350.0);
        planeQuat = SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = Vector3d::Zero();
        int expectedNumberOfContacts = 0;
        int expectedBoxIndicesInContacts[] = {0};
        bool collisionNormalIsPlaneNormal = true;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

    {
        SCOPED_TRACE("Intersection in front of plane, four contacts, rotated plane");
        boxQuat = SurgSim::Math::makeRotationQuaternion(1.233469, Vector3d(0.91834,0.39687,0.8271).normalized());
        boxTrans = Vector3d(0.5,10.0,350.0);
        planeQuat = boxQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + boxQuat * Vector3d(-0.5,0.0,0.0);
        int expectedNumberOfContacts = 4;
        int expectedBoxIndicesInContacts[] = {0, 1, 2, 3};
        bool collisionNormalIsPlaneNormal = true;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

	{
        SCOPED_TRACE("Intersection inside of plane, four contacts, rotated plane");
        boxQuat = SurgSim::Math::makeRotationQuaternion(1.233469, Vector3d(0.91834,0.39687,0.8271).normalized());
        boxTrans = Vector3d(0.5,10.0,350.0);
        planeQuat = boxQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + boxQuat * Vector3d(-0.4,0.0,0.0);
        int expectedNumberOfContacts = 4;
        int expectedBoxIndicesInContacts[] = {0, 1, 2, 3};
        bool collisionNormalIsPlaneNormal = true;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

    {
        SCOPED_TRACE("Intersection in front of plane, two contacts, rotated plane");
		globalQuat = SurgSim::Math::makeRotationQuaternion(0.8753, Vector3d(0.235345,0.6754,0.4567).normalized());
        boxQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_4, Vector3d(0.0,0.0,1.0));
        boxTrans = Vector3d(std::sqrt(0.5),230.0,540.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(-std::sqrt(0.5),0.0,0.0);
        int expectedNumberOfContacts = 2;
        int expectedBoxIndicesInContacts[] = {0, 1};
        bool collisionNormalIsPlaneNormal = true;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

	{
        SCOPED_TRACE("Intersection inside of plane, two contacts, rotated plane");
		globalQuat = SurgSim::Math::makeRotationQuaternion(0.8753, Vector3d(0.235345,0.6754,0.4567).normalized());
        boxQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_4, Vector3d(0.0,0.0,1.0));
        boxTrans = Vector3d(std::sqrt(0.5),230.0,540.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(-std::sqrt(0.45),0.0,0.0);
        int expectedNumberOfContacts = 2;
        int expectedBoxIndicesInContacts[] = {0, 1};
        bool collisionNormalIsPlaneNormal = true;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

    {
        SCOPED_TRACE("Intersection in front of plane, one contact, rotated plane");
        globalQuat = SurgSim::Math::makeRotationQuaternion(-0.3257, Vector3d(-0.4575,-0.8563,0.63457).normalized());
        double angle = -35.264389682754654315377000330019*(M_PI/180.0);
        boxQuat = globalQuat * Quaterniond(SurgSim::Math::makeRotationMatrix(angle, Vector3d(0.0,1.0,0.0)) *
										   SurgSim::Math::makeRotationMatrix(-M_PI_4, Vector3d(0.0,0.0,1.0)));
        boxTrans = Vector3d(std::sqrt(0.75),0.0,0.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(-std::sqrt(0.75),0.0,0.0);
        int expectedNumberOfContacts = 1;
        int expectedBoxIndicesInContacts[] = {1};
        bool collisionNormalIsPlaneNormal = true;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

	{
        SCOPED_TRACE("Intersection inside of plane, one contact, rotated plane");
        globalQuat = SurgSim::Math::makeRotationQuaternion(-0.3257, Vector3d(-0.4575,-0.8563,0.63457).normalized());
        double angle = -35.264389682754654315377000330019*(M_PI/180.0);
        boxQuat = globalQuat * Quaterniond(SurgSim::Math::makeRotationMatrix(angle, Vector3d(0.0,1.0,0.0)) *
										   SurgSim::Math::makeRotationMatrix(-M_PI_4, Vector3d(0.0,0.0,1.0)));
        boxTrans = Vector3d(std::sqrt(0.75),0.0,0.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(-std::sqrt(0.74),0.0,0.0);
        int expectedNumberOfContacts = 1;
        int expectedBoxIndicesInContacts[] = {1};
        bool collisionNormalIsPlaneNormal = true;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

    {
        SCOPED_TRACE("No intersection, box behind rotated plane");
        boxQuat = SurgSim::Math::makeRotationQuaternion(0.3252, Vector3d(0.5434,0.634,0.13435).normalized());
        boxTrans = Vector3d(-45.5,10.0,350.0);
        planeQuat = SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = Vector3d::Zero();
        int expectedNumberOfContacts = 0;
        int expectedBoxIndicesInContacts[] = {0};
        bool collisionNormalIsPlaneNormal = false;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

    {
        SCOPED_TRACE("Intersection behind plane, four contacts, rotated plane");
        boxQuat = SurgSim::Math::makeRotationQuaternion(0.1436, Vector3d(0.8441,0.3579,0.2168).normalized());
        boxTrans = Vector3d(-0.5,0.0,0.0);
        planeQuat = boxQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + boxQuat * Vector3d(0.5,0.0,0.0);
        int expectedNumberOfContacts = 4;
        int expectedBoxIndicesInContacts[] = {4, 5, 6, 7};
        bool collisionNormalIsPlaneNormal = false;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

    {
        SCOPED_TRACE("Intersection behind plane, two contacts, rotated plane");
		globalQuat = SurgSim::Math::makeRotationQuaternion(-0.2356, Vector3d(0.4542,-0.2356,0.1187).normalized());
        boxQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_4, Vector3d(0.0,0.0,1.0));
        boxTrans = Vector3d(-std::sqrt(0.5),0.0,0.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(std::sqrt(0.5),0.0,0.0);
        int expectedNumberOfContacts = 2;
        int expectedBoxIndicesInContacts[] = {6, 7};
        bool collisionNormalIsPlaneNormal = false;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }

    {
        SCOPED_TRACE("Intersection behind plane, one contact, rotated plane");
		globalQuat = SurgSim::Math::makeRotationQuaternion(1.4576, Vector3d(23.45,-98.24,42.46).normalized());
        double angle = -35.264389682754654315377000330019*(M_PI/180.0);
        boxQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(angle, Vector3d(0.0,0.0,1.0)) *
                  SurgSim::Math::makeRotationQuaternion(-M_PI_4, Vector3d(0.0,1.0,0.0));
        boxTrans = Vector3d(-std::sqrt(0.75),0.0,0.0);
        planeQuat = globalQuat * SurgSim::Math::makeRotationQuaternion(-M_PI_2, Vector3d(0.0,0.0,1.0));
        planeTrans = boxTrans + globalQuat * Vector3d(std::sqrt(0.75),0.0,0.0);
        int expectedNumberOfContacts = 1;
        int expectedBoxIndicesInContacts[] = {7};
        bool collisionNormalIsPlaneNormal = false;
        doBoxDoubleSidedPlaneTest(box, boxQuat, boxTrans, plane, planeQuat, planeTrans, expectedNumberOfContacts,
								  expectedBoxIndicesInContacts, collisionNormalIsPlaneNormal);
    }
}

void doBoxSphereTest(std::shared_ptr<BoxShape> box,
					 const Quaterniond& boxQuat,
					 const Vector3d& boxTrans,
					 std::shared_ptr<SphereShape> sphere,
					 const Quaterniond& sphereQuat,
					 const Vector3d& sphereTrans,
					 bool hasContacts = false,
					 double expectedDepth = 0.0,
					 Vector3d expectedNormal = Vector3d::UnitX(),
					 Vector3d expectedPenetrationPoint0 = Vector3d::Zero(),
					 Vector3d expectedPenetrationPoint1 = Vector3d::Zero())
{
	using SurgSim::Math::Geometry::DistanceEpsilon;
	using SurgSim::Math::Geometry::ScalarEpsilon;

    std::shared_ptr<CollisionRepresentation> boxRep = std::make_shared<MockCollisionRepresentation>(
		"Collision Box 0",
		box,
		boxQuat,
		boxTrans);
    std::shared_ptr<CollisionRepresentation> sphereRep = std::make_shared<MockCollisionRepresentation>(
		"Collision Sphere 0",
		sphere,
		sphereQuat,
		sphereTrans);

    // Perform collision detection.
    BoxSphereDcdContact calcContact;
    std::shared_ptr<CollisionPair> pair = std::make_shared<CollisionPair>(boxRep, sphereRep);
    calcContact.calculateContact(pair);

	// Compare contact info.
	EXPECT_EQ(hasContacts, pair->hasContacts());
	if (pair->hasContacts())
	{
		std::shared_ptr<Contact> contact = pair->getContacts().front();
		EXPECT_TRUE(eigenEqual(expectedNormal, contact->normal, ScalarEpsilon));
		EXPECT_NEAR(expectedDepth, contact->depth, DistanceEpsilon);
		EXPECT_TRUE(contact->penetrationPoints.first.globalPosition.hasValue());
		EXPECT_TRUE(contact->penetrationPoints.second.globalPosition.hasValue());

		EXPECT_TRUE(eigenEqual(expectedPenetrationPoint0,
							   contact->penetrationPoints.first.globalPosition.getValue(),
							   ScalarEpsilon));
		EXPECT_TRUE(eigenEqual(expectedPenetrationPoint1,
							   contact->penetrationPoints.second.globalPosition.getValue(),
							   ScalarEpsilon));
	}
}

TEST(ContactCalculationTests, BoxSphereCalculation)
{
	std::shared_ptr<BoxShape> box = std::make_shared<BoxShape>(1.0, 1.0, 1.0);
    std::shared_ptr<SphereShape> sphere = std::make_shared<SphereShape>(1.0);
    SurgSim::Math::Quaterniond boxQuat;
    SurgSim::Math::Vector3d boxTrans;
    SurgSim::Math::Quaterniond sphereQuat;
    SurgSim::Math::Vector3d sphereTrans;
	SurgSim::Math::Quaterniond globalQuat;
	SurgSim::Math::Vector3d globalTrans;

	{
		SCOPED_TRACE("No Intersection");
		boxQuat.setIdentity();
		boxTrans = Vector3d(100.0,0.0,0.0);
		sphereQuat.setIdentity();
		sphereTrans = Vector3d(0.0,0.0,0.0);
		doBoxSphereTest(box, boxQuat, boxTrans, sphere, sphereQuat, sphereTrans, false);
	}

	{
		SCOPED_TRACE("Intersection on top face");
		boxQuat.setIdentity();
		boxTrans = Vector3d(0.0,0.0,0.0);
		sphereQuat.setIdentity();
		sphereTrans = Vector3d(0.0,1.0,0.0);
		globalQuat = SurgSim::Math::makeRotationQuaternion(0.35465, Vector3d(0.3454, 0.78567, 0.234346).normalized());
		globalTrans = Vector3d(24.6,-32.67,87.53);
		boxQuat = globalQuat * boxQuat;
		boxTrans = globalQuat * boxTrans + globalTrans;
		sphereQuat = globalQuat * sphereQuat;
		sphereTrans = globalQuat * sphereTrans + globalTrans;
		doBoxSphereTest(box, boxQuat, boxTrans, sphere, sphereQuat, sphereTrans, true,
						0.5, // depth
						boxQuat * Vector3d(0.0,1.0,0.0), // normal
						boxQuat * Vector3d(0.0,0.5,0.0) + boxTrans, // box penetration point
						sphereQuat * Vector3d(0.0,-1.0,0.0) + sphereTrans); // sphere penetration point
	}

	{
		SCOPED_TRACE("Intersection on bottom face");
		boxQuat.setIdentity();
		boxTrans = Vector3d(0.0,0.0,0.0);
		sphereQuat.setIdentity();
		sphereTrans = Vector3d(0.3345,-1.2,0.1234);
		globalQuat =
			SurgSim::Math::makeRotationQuaternion(-0.35465, Vector3d(18.3454, -27.78567, 23.234346).normalized());
		globalTrans = Vector3d(234.6,326.67,987.53);
		boxQuat = globalQuat * boxQuat;
		boxTrans = globalQuat * boxTrans + globalTrans;
		sphereQuat = globalQuat * sphereQuat;
		sphereTrans = globalQuat * sphereTrans + globalTrans;
		doBoxSphereTest(box, boxQuat, boxTrans, sphere, sphereQuat, sphereTrans, true,
						0.3, // depth
						boxQuat * Vector3d(0.0,-1.0,0.0), // normal
						boxQuat * Vector3d(0.3345,-0.5,0.1234) + boxTrans, // box penetration point
						sphereQuat * Vector3d(0.0,1.0,0.0) + sphereTrans); // sphere penetration point
	}

	{
		SCOPED_TRACE("Intersection on right face");
		boxQuat.setIdentity();
		boxTrans = Vector3d(23.545,3.4321,5.3421);
		sphereQuat.setIdentity();
		sphereTrans = boxTrans + Vector3d(1.2324,-0.2354,0.412);
		globalQuat = SurgSim::Math::makeRotationQuaternion(1.285, Vector3d(23.446, 13.786, 32.254).normalized());
		globalTrans = Vector3d(-249.6,532.67,977.53);
		boxQuat = globalQuat * boxQuat;
		boxTrans = globalQuat * boxTrans + globalTrans;
		sphereQuat = globalQuat * sphereQuat;
		sphereTrans = globalQuat * sphereTrans + globalTrans;
		doBoxSphereTest(box, boxQuat, boxTrans, sphere, sphereQuat, sphereTrans, true,
						0.2676, // depth
						boxQuat * Vector3d(1.0,0.0,0.0), // normal
						boxQuat * Vector3d(0.5,-0.2354,0.412) + boxTrans, // box penetration point
						sphereQuat * Vector3d(-1.0,0.0,0.0) + sphereTrans); // sphere penetration point
	}

	{
		SCOPED_TRACE("Intersection on left face");
		boxQuat.setIdentity();
		boxTrans = Vector3d(876.324,6754.23,7343.76);
		sphereQuat.setIdentity();
		sphereTrans = boxTrans + Vector3d(-1.1223,0.2354,-0.412);
		globalQuat = SurgSim::Math::makeRotationQuaternion(0.276, Vector3d(0.945, 1.532, 0.896).normalized());
		globalTrans = Vector3d(-24.6,32.67,97.53);
		boxQuat = globalQuat * boxQuat;
		boxTrans = globalQuat * boxTrans + globalTrans;
		sphereQuat = globalQuat * sphereQuat;
		sphereTrans = globalQuat * sphereTrans + globalTrans;
		doBoxSphereTest(box, boxQuat, boxTrans, sphere, sphereQuat, sphereTrans, true,
						0.3777, // depth
						boxQuat * Vector3d(-1.0,0.0,0.0), // normal
						boxQuat * Vector3d(-0.5,0.2354,-0.412) + boxTrans, // box penetration point
						sphereQuat * Vector3d(1.0,0.0,0.0) + sphereTrans); // sphere penetration point
	}

	{
		SCOPED_TRACE("Intersection on front face");
		boxQuat.setIdentity();
		boxTrans = Vector3d(0.3252,-0.64564,0.12345);
		sphereQuat.setIdentity();
		sphereTrans = boxTrans + Vector3d(0.1564,-0.2987,-0.8986);
		globalQuat = SurgSim::Math::makeRotationQuaternion(-1.32, Vector3d(235.67, 215.567, 146.345).normalized());
		globalTrans = Vector3d(224.6,132.67,27.53);
		boxQuat = globalQuat * boxQuat;
		boxTrans = globalQuat * boxTrans + globalTrans;
		sphereQuat = globalQuat * sphereQuat;
		sphereTrans = globalQuat * sphereTrans + globalTrans;
		doBoxSphereTest(box, boxQuat, boxTrans, sphere, sphereQuat, sphereTrans, true,
						0.6014, // depth
						boxQuat * Vector3d(0.0,0.0,-1.0), // normal
						boxQuat * Vector3d(0.1564,-0.2987,-0.5) + boxTrans, // box penetration point
						sphereQuat * Vector3d(0.0,0.0,1.0) + sphereTrans); // sphere penetration point
	}

	{
		SCOPED_TRACE("Intersection on back face");
		boxQuat.setIdentity();
		boxTrans = Vector3d(24.345,-865.325,46.345);
		sphereQuat.setIdentity();
		sphereTrans = boxTrans + Vector3d(-0.2564,-0.4987,0.7986);
		globalQuat = SurgSim::Math::makeRotationQuaternion(1.2, Vector3d(25.67, -25.567, 16.345).normalized());
		globalTrans = Vector3d(24.6,3243.67,9762.53);
		boxQuat = globalQuat * boxQuat;
		boxTrans = globalQuat * boxTrans + globalTrans;
		sphereQuat = globalQuat * sphereQuat;
		sphereTrans = globalQuat * sphereTrans + globalTrans;
		doBoxSphereTest(box, boxQuat, boxTrans, sphere, sphereQuat, sphereTrans, true,
						0.7014, // depth
						boxQuat * Vector3d(0.0,0.0,1.0), // normal
						boxQuat * Vector3d(-0.2564,-0.4987,0.5) + boxTrans, // box penetration point
						sphereQuat * Vector3d(0.0,0.0,-1.0) + sphereTrans); // sphere penetration point
	}
}

void doCapsuleSphereTest(double capsuleHeight, double capsuleRadius,
						 const Vector3d& capsulePosition, const Quaterniond& capsuleQuat,
						 double sphereRadius, const Vector3d& spherePosition, const Quaterniond& sphereQuat,
						 bool hasContacts, double depth,
						 const Vector3d& sphereProjection = Vector3d::Zero(),
						 const Vector3d& expectedNorm = Vector3d::Zero())
{
	std::shared_ptr<CollisionPair> pair = std::make_shared<CollisionPair>(
		makeCapsuleRepresentation(nullptr, capsuleHeight, capsuleRadius, capsuleQuat, capsulePosition),
		makeSphereRepresentation(nullptr, sphereRadius, sphereQuat, spherePosition));

	CapsuleSphereDcdContact calc;
	calc.calculateContact(pair);
	EXPECT_EQ(hasContacts, pair->hasContacts());

	if (pair->hasContacts())
	{
		std::shared_ptr<Contact> contact(pair->getContacts().front());

		EXPECT_TRUE(eigenEqual(expectedNorm, contact->normal, epsilon));
		EXPECT_NEAR(depth, contact->depth, epsilon);
		EXPECT_TRUE(contact->penetrationPoints.first.globalPosition.hasValue());
		EXPECT_TRUE(contact->penetrationPoints.second.globalPosition.hasValue());

		Vector3d penetrationPoint0(sphereProjection - expectedNorm * capsuleRadius);
		Vector3d penetrationPoint1(spherePosition + expectedNorm * sphereRadius);
		EXPECT_TRUE(eigenEqual(penetrationPoint0, contact->penetrationPoints.first.globalPosition.getValue(), epsilon));
		EXPECT_TRUE(eigenEqual(penetrationPoint1, contact->penetrationPoints.second.globalPosition.getValue(),epsilon));
	}
}

TEST(ContactCalculationTests, CapsuleSphereCalculation)
{
	{
		SCOPED_TRACE("No Intersection");
		doCapsuleSphereTest(0.2, 0.1, Vector3d::Zero(), Quaterniond::Identity(),
							0.1, Vector3d(1.0, 1.0, 1.0), Quaterniond::Identity(), false, 0.0);
	}

	{
		SCOPED_TRACE("Capsule along Y-axis, intersection with cylindrical part of the capsule");
		doCapsuleSphereTest(0.8, 0.5, Vector3d::Zero(), Quaterniond::Identity(),
							0.3, Vector3d(0.7, 0, 0), Quaterniond::Identity(), true, 0.1,
							Vector3d::Zero(), Vector3d(-1.0, 0.0, 0.0));
	}

	{
		SCOPED_TRACE("Capsule along X-axis, intersection with hemispherical part of the capsule");
		doCapsuleSphereTest(0.1, 0.2, Vector3d::Zero(),
							SurgSim::Math::makeRotationQuaternion(M_PI_2, Vector3d(0.0, 0.0, 1.0)),
							0.1, Vector3d(-0.2, 0.0, 0.0),
							Quaterniond::Identity(), true, 0.15,
							Vector3d(-0.05, 0.0, 0.0), Vector3d(1.0, 0.0, 0.0));
	}

	{
		SCOPED_TRACE("Intersection, capsule roated along Z-axis clockwise 45 degrees");
		Vector3d sphereCenter(2.0, 0.0, 0.0);
		Vector3d sphereProjection(1.0, 1.0, 0.0);
		Vector3d expectedNormal = (sphereProjection - sphereCenter).normalized();

		doCapsuleSphereTest(2 * M_SQRT2, M_SQRT2, Vector3d::Zero(),
							SurgSim::Math::makeRotationQuaternion(-M_PI_4, Vector3d(0.0, 0.0, 1.0)),
							1.0, sphereCenter,
							Quaterniond::Identity(), true, 1,
							sphereProjection, expectedNormal);

	}
}


}; // Physics
}; // SurgSim
