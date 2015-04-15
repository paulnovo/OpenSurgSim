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

/// \file
/// Tests for the LinearSparseSolveAndInverse.cpp functions.

#include <gtest/gtest.h>

#include "SurgSim/Math/LinearSparseSolveAndInverse.h"

namespace SurgSim
{

namespace Math
{

class LinearSparseSolveAndInverseTests : public ::testing::Test
{
private:
	size_t size;

	void initializeVector(Vector* v)
	{
		v->resize(size);
		for (size_t row = 0; row < size; row++)
		{
			(*v)(row) = fmod(-4.1 * row * row + 3.46, 5.0);
		}
	}

	void initializeSparseMatrix(SparseMatrix* m)
	{
		m->resize(static_cast<int>(size), static_cast<int>(size));
		for (size_t row = 0; row < size; row++)
		{
			for (size_t col = 0; col < size; col++)
			{
				(*m).insert(static_cast<int>(row), static_cast<int>(col)) =
					fmod((10.3 * cos(static_cast<double>(row * col)) + 3.24), 10.0);
			}
		}
		m->makeCompressed();
	}

	void initializeDenseMatrix(Matrix* m)
	{
		m->resize(size, size);
		for (size_t row = 0; row < size; row++)
		{
			for (size_t col = 0; col < size; col++)
			{
				(*m)(row, col) = fmod((10.3 * cos(static_cast<double>(row * col)) + 3.24), 10.0);
			}
		}
	}

	void setupTest()
	{
		initializeVector(&b);
		initializeDenseMatrix(&denseMatrix);
		expectedInverse = denseMatrix.inverse();
		expectedX = expectedInverse * b;
	}

public:

	void setupSparseMatrixTest()
	{
		size = 18;
		inverseMatrix.resize(size, size);
		initializeSparseMatrix(&matrix);
		setupTest();
	}

	SparseMatrix matrix;
	Matrix denseMatrix, inverseMatrix, expectedInverse;
	Vector b;
	Vector x, expectedX;
};

TEST_F(LinearSparseSolveAndInverseTests, SparseLUInitializationTests)
{
	SparseMatrix nonSquare(9, 18);
	nonSquare.setZero();

	LinearSparseSolveAndInverseLU solveAndInverse;
	EXPECT_ANY_THROW(solveAndInverse(nonSquare, b, &x, &inverseMatrix));
	EXPECT_ANY_THROW(solveAndInverse.setMatrix(nonSquare));
};

TEST_F(LinearSparseSolveAndInverseTests, SparseLUMatrixOperatorTests)
{
	setupSparseMatrixTest();

	LinearSparseSolveAndInverseLU solveAndInverse;
	solveAndInverse(matrix, b, &x, &inverseMatrix);

	EXPECT_TRUE(x.isApprox(expectedX));
	EXPECT_TRUE(inverseMatrix.isApprox(expectedInverse));
};

TEST_F(LinearSparseSolveAndInverseTests, SparseLUMatrixComponentsTest)
{
	setupSparseMatrixTest();

	LinearSparseSolveAndInverseLU solveAndInverse;
	solveAndInverse.setMatrix(matrix);
	x = solveAndInverse.solve(b);
	inverseMatrix = solveAndInverse.getInverse();

	EXPECT_TRUE(x.isApprox(expectedX));
	EXPECT_TRUE(inverseMatrix.isApprox(expectedInverse));
};


}; // namespace Math

}; // namespace SurgSim
