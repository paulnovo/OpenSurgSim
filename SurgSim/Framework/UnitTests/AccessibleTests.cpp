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

#include <gtest/gtest.h>
#include <memory>
#include <functional>
#include <boost/any.hpp>

#include "SurgSim/Math/Matrix.h"

class TestClass : public SurgSim::Framework::Accessible
{
public:
	TestClass() :
		normal(100),
		readWrite(100.0),
		readOnly(100),
		sharedPtr(std::make_shared<int>(4)),
		overloadedValue(200.0),
		privateProperty(100)
	{
		setGetter("normal", std::bind(&TestClass::getNormal, this));
		setSetter("normal", std::bind(&TestClass::setNormal, this, std::bind(SurgSim::Framework::convert<int>,
									  std::placeholders::_1)));

		SURGSIM_ADD_RW_PROPERTY(TestClass, double, readWrite, getReadWrite, setReadWrite);
		SURGSIM_ADD_RW_PROPERTY(TestClass, std::shared_ptr<int>, sharedPtr, getSharedPtr, setSharedPtr);

		SURGSIM_ADD_RO_PROPERTY(TestClass, int, readOnly, getReadOnly);

		SURGSIM_ADD_RW_PROPERTY(TestClass, double, privateProperty, getPrivateProperty, setPrivateProperty);

		SURGSIM_ADD_SERIALIZABLE_PROPERTY(TestClass, float, serializableProperty,
										  getSerializableProperty, setSerializableProperty);
	}

	int normal;
	double readWrite;
	int readOnly;

	std::shared_ptr<int> sharedPtr;

	float serializableProperty;

	double overloadedValue;

	int getNormal()
	{
		return normal;
	}
	void setNormal(int val)
	{
		normal = val;
	}

	double getReadWrite()
	{
		return readWrite;
	}
	void setReadWrite(double val)
	{
		readWrite = val;
	}

	std::shared_ptr<int> getSharedPtr()
	{
		return sharedPtr;
	}
	void setSharedPtr(std::shared_ptr<int> val)
	{
		sharedPtr = val;
	}

	int getReadOnly()
	{
		return readOnly;
	}

	double getPrivateProperty() const
	{
		return privateProperty;
	}
	void setPrivateProperty(double val)
	{
		privateProperty = val;
	}

	float getSerializableProperty() const
	{
		return serializableProperty;
	}
	void setSerializableProperty(float val)
	{
		serializableProperty = val;
	}

	void getOverloadedFunction(double* x) const {}
	double getOverloadedFunction() const
	{
		return overloadedValue;
	}

	void setOverloadedFunction(double x, double y) {}
	void setOverloadedFunction(const double& x)
	{
		overloadedValue = x;
	}
private:
	double privateProperty;
};

namespace SurgSim
{
namespace Framework
{

TEST(AccessibleTest, GetterTest)
{
	TestClass t;
	t.normal = 5;

	int receiver = -1;

	EXPECT_EQ(5, boost::any_cast<int>(t.getValue("normal")));
	EXPECT_EQ(5, t.getValue<int>("normal"));
	EXPECT_TRUE(t.getValue<int>("normal", &receiver));
	EXPECT_EQ(5, receiver);

	/// Response to fetching value that does not exist
	EXPECT_ANY_THROW(t.getValue("xxx"));
	EXPECT_ANY_THROW(t.getValue<int>("xxx"));
	receiver = -1;
	EXPECT_FALSE(t.getValue<int>("xxx", &receiver));
	EXPECT_EQ(-1, receiver);

	/// Response to trying to fetch an type that can't be converted
	EXPECT_ANY_THROW(t.getValue<TestClass>("normal"));
	EXPECT_FALSE(t.getValue<TestClass>("normal", &t));

}

TEST(AccessibleTest, SetterTest)
{
	TestClass t;
	t.normal = 0;

	t.setValue("normal", 4);
	EXPECT_EQ(4, t.getNormal());
	EXPECT_ANY_THROW(t.setValue("xxxx", 666.66));
}

TEST(AccessibleTest, TransferTest)
{
	TestClass a, b;
	a.normal = 100;
	b.normal = 0;

	b.setValue("normal", a.getValue("normal"));

	EXPECT_EQ(a.normal, b.normal);
}

TEST(AccessibleTest, ReadWriteMacroTest)
{
	TestClass a;
	a.readWrite = 100.0;

	EXPECT_EQ(a.readWrite, boost::any_cast<double>(a.getValue("readWrite")));
	a.setValue("readWrite", 50.0);
	EXPECT_EQ(50.0, a.readWrite);
}

TEST(AccessibleTest, ReadOnlyMacroTest)
{
	TestClass a;
	a.readOnly = 200;

	EXPECT_EQ(a.readOnly, boost::any_cast<int>(a.getValue("readOnly")));

	EXPECT_ANY_THROW(a.setValue("readOnly", 100));
}

TEST(AccessibleTest, TemplateFunction)
{
	TestClass a;
	a.normal = 10;
	a.readWrite = 100.0;

	// Parameter Deduction
	int aDotNormal = 123;
	double aDotReadWrite = 456;
	EXPECT_TRUE(a.getValue("normal", &aDotNormal));
	EXPECT_EQ(10, aDotNormal);
	EXPECT_TRUE(a.getValue("readWrite", &aDotReadWrite));
	EXPECT_EQ(100.0, aDotReadWrite);

	EXPECT_FALSE(a.getValue("xxxx", &aDotNormal));

	double* noValue = nullptr;

	EXPECT_FALSE(a.getValue("normal", noValue));
}

TEST(AccessibleTest, Privates)
{
	TestClass a;

	EXPECT_EQ(a.getPrivateProperty(), boost::any_cast<double>(a.getValue("privateProperty")));
	EXPECT_NO_THROW(a.setValue("privateProperty", 123.456));
	EXPECT_NEAR(123.456, boost::any_cast<double>(a.getValue("privateProperty")), 1e10);
	EXPECT_NEAR(a.getPrivateProperty(), boost::any_cast<double>(a.getValue("privateProperty")), 1e10);
}

TEST(AccessibleTest, SharedPointer)
{
	TestClass a;
	std::shared_ptr<int> x = std::make_shared<int>(5);
	std::shared_ptr<int> y;

	y = boost::any_cast<std::shared_ptr<int>>(a.getValue("sharedPtr"));
	EXPECT_EQ(4, *y);

	a.setValue("sharedPtr", x);
	y = boost::any_cast<std::shared_ptr<int>>(a.getValue("sharedPtr"));
	EXPECT_EQ(5, *y);
}

TEST(AccessibleTest, ConvertDoubleToFloat)
{
	// Values don't matter only care for them to be filled
	SurgSim::Math::Matrix44d sourceDouble;
	sourceDouble <<
				 1.0 / 2.0, 1.0 / 3.0, 1.0 / 4.0, 1.0 / 5.0,
					 1.0 / 6.0, 1.0 / 7.0, 1.0 / 8.0, 1.0 / 9.0,
					 1.0 / 10.0, 1.0 / 11.0, 1.0 / 12.0, 1.0 / 13.0,
					 1.0 / 14.0, 1.0 / 15.0, 1.0 / 16.0, 1.0 / 17.0;

	SurgSim::Math::Matrix44f sourceFloat;
	sourceFloat <<
				1.0f / 2.0f, 1.0f / 3.0f, 1.0f / 4.0f, 1.0f / 5.0f,
					 1.0f / 6.0f, 1.0f / 7.0f, 1.0f / 8.0f, 1.0f / 9.0f,
					 1.0f / 10.0f, 1.0f / 11.0f, 1.0f / 12.0f, 1.0f / 13.0f,
					 1.0f / 14.0f, 1.0f / 15.0f, 1.0f / 16.0f, 1.0f / 17.0f;

	SurgSim::Math::Matrix44f target;

	ASSERT_NO_THROW({convert<SurgSim::Math::Matrix44f>(sourceDouble);});
	target = convert<SurgSim::Math::Matrix44f>(sourceDouble);
	SurgSim::Math::Matrix44f doubleToFloat = sourceDouble.cast<float>();
	EXPECT_TRUE(target.isApprox(doubleToFloat));

	ASSERT_NO_THROW({convert<SurgSim::Math::Matrix44f>(sourceFloat);});
	target = convert<SurgSim::Math::Matrix44f>(sourceFloat);
	EXPECT_TRUE(target.isApprox(sourceFloat));
}

TEST(AccessibleTests, Serialize)
{
	TestClass a;
	a.serializableProperty = 100;

	YAML::Node node = a.encode();

	EXPECT_TRUE(node.IsMap());
	EXPECT_EQ(100, node["serializableProperty"].as<int>());

	node["serializableProperty"] = 50;
	EXPECT_NO_THROW(a.decode(node));
	EXPECT_EQ(50, a.serializableProperty);
}

class MultipleValuesClass : public Accessible
{
public:
	MultipleValuesClass() : a("invalid"), b("invalid"), c("invalid")
	{
		SURGSIM_ADD_SERIALIZABLE_PROPERTY(MultipleValuesClass, std::string, a, getA, setA);
		SURGSIM_ADD_SERIALIZABLE_PROPERTY(MultipleValuesClass, std::string, b, getB, setB);
		SURGSIM_ADD_SERIALIZABLE_PROPERTY(MultipleValuesClass, std::string, c, getC, setC);
	}

	std::string a;
	std::string getA() const
	{
		return a;
	}
	void setA(std::string val)
	{
		a = val;
	}

	std::string b;
	std::string getB() const
	{
		return b;
	}
	void setB(std::string val)
	{
		b = val;
	}

	std::string c;
	std::string getC() const
	{
		return c;
	}
	void setC(std::string val)
	{
		c = val;
	}
};

TEST(AccessibleTests, MultipleValues)
{
	YAML::Node newValues;
	newValues["xxx"] = "invalid";
	newValues["a"] = "a";
	newValues["b"] = "b";

	MultipleValuesClass test;
	test.decode(newValues);

	EXPECT_EQ(test.a, "a");
	EXPECT_EQ(test.b, "b");
	EXPECT_EQ(test.c, "invalid");

	YAML::Node encodedValues = test.encode();

	EXPECT_EQ("a", encodedValues["a"].as<std::string>());
	EXPECT_EQ("b", encodedValues["b"].as<std::string>());
	EXPECT_EQ("invalid", encodedValues["c"].as<std::string>());

}



}; // namespace Framework
}; // namespace SurgSim
