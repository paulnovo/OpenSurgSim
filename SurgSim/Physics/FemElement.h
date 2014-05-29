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

#ifndef SURGSIM_PHYSICS_FEMELEMENT_H
#define SURGSIM_PHYSICS_FEMELEMENT_H

#include <vector>

#include "SurgSim/Math/Matrix.h"
#include "SurgSim/Math/Vector.h"

namespace SurgSim
{

namespace Math
{
class OdeState;
};

namespace Physics
{

/// Base class for all Fem Element (1D, 2D, 3D)
/// It handles the node ids to which it is connected and requires all derived classes to compute the element
/// mass matrix and the force vector along with the derivatives (the stiffness and damping matrices).
/// A extra method also exist to compute all of them at once for performance purposes.
/// It holds on to the actual computed values (m_f, m_M, m_D, m_K) as its size is not predefined from outside
/// and would requires intensive (de)allocation or a temporary variable anyway.
/// It contains the linear elasticity parameter (Young modulus and Poisson ratio) as well as mass density
class FemElement
{
public:
	/// Constructor
	FemElement();

	/// Virtual destructor
	virtual ~FemElement();

	/// Initialize the FemElement once everything has been set
	/// \param state The state to initialize the FemElement with
	virtual void initialize(const SurgSim::Math::OdeState& state);

	/// Gets the number of degree of freedom per node
	/// \return The number of dof per node
	unsigned int getNumDofPerNode() const;

	/// Gets the number of nodes connected by this element
	/// \return The number of nodes
	unsigned int getNumNodes() const;

	/// Gets the elementNodeId-th node id
	/// \return The requested node id
	unsigned int getNodeId(unsigned int elementNodeId) const;

	/// Gets the node ids for this element
	/// \return A vector containing the node ids on which the element is defined
	const std::vector<unsigned int>& getNodeIds() const;

	/// Sets the Young modulus (in N.m-2)
	/// \param E The Young modulus
	void setYoungModulus(double E);
	/// Gets the Young modulus (in N.m-2)
	/// \return The Young modulus
	double getYoungModulus() const;

	/// Sets the Poisson ratio (unitless)
	/// \param nu The Poisson ratio
	void setPoissonRatio(double nu);
	/// Gets the Poisson ratio (unitless)
	/// \return The Poisson ratio
	double getPoissonRatio() const;

	/// Sets the mass density (in Kg.m-3)
	/// \param rho The mass density
	void setMassDensity(double rho);
	/// Gets the mass density (in Kg.m-3)
	/// \return The mass density
	double getMassDensity() const;

	/// Gets the element mass based on the input state (in Kg)
	/// \param state The state to compute the mass with
	/// \return The mass of this element (in Kg)
	double getMass(const SurgSim::Math::OdeState& state) const;

	/// Gets the element volume based on the input state (in m-3)
	/// \param state The state to compute the volume with
	/// \return The volume of this element (in m-3)
	virtual double getVolume(const SurgSim::Math::OdeState& state) const = 0;

	/// Adds the element force (computed for a given state) to a complete system force vector F (assembly)
	/// \param state The state to compute the force with
	/// \param[in,out] F The complete system force vector to add the element force into
	/// \param scale A factor to scale the added force with
	/// \note The element force is of size (getNumDofPerNode() x getNumNodes())
	/// \note This method supposes that the incoming state contains information with the same number of dof
	/// \note per node as getNumDofPerNode()
	virtual void addForce(const SurgSim::Math::OdeState& state, SurgSim::Math::Vector* F, double scale = 1.0) = 0;

	/// Adds the element mass matrix M (computed for a given state) to a complete system mass matrix M (assembly)
	/// \param state The state to compute the mass matrix with
	/// \param[in,out] M The complete system mass matrix to add the element mass-matrix into
	/// \param scale A factor to scale the added mass matrix with
	/// \note The element mass matrix is square of size getNumDofPerNode() x getNumNodes()
	/// \note This method supposes that the incoming state contains information with the same number of
	/// \note dof per node as getNumDofPerNode()
	virtual void addMass(const SurgSim::Math::OdeState& state, SurgSim::Math::Matrix* M, double scale = 1.0) = 0;

	/// Adds the element damping matrix D (= -df/dv) (comuted for a given state)
	/// to a complete system damping matrix D (assembly)
	/// \param state The state to compute the damping matrix with
	/// \param[in,out] D The complete system damping matrix to add the element damping matrix into
	/// \param scale A factor to scale the added damping matrix with
	/// \note The element damping matrix is square of size getNumDofPerNode() x getNumNodes()
	/// \note This method supposes that the incoming state contains information with the same number of
	/// \note dof per node as getNumDofPerNode()
	virtual void addDamping(const SurgSim::Math::OdeState& state, SurgSim::Math::Matrix* D,
		double scale = 1.0) = 0;

	/// Adds the element stiffness matrix K (= -df/dx) (computed for a given state)
	/// to a complete system stiffness matrix K (assembly)
	/// \param state The state to compute the stiffness matrix with
	/// \param[in,out] K The complete system stiffness matrix to add the element stiffness matrix into
	/// \param scale A factor to scale the added stiffness matrix with
	/// \note The element stiffness matrix is square of size getNumDofPerNode() x getNumNodes()
	/// \note This method supposes that the incoming state contains information with the same number of
	/// \note dof per node as getNumDofPerNode()
	virtual void addStiffness(const SurgSim::Math::OdeState& state, SurgSim::Math::Matrix* K,
		double scale = 1.0) = 0;

	/// Adds the element force vector, mass, stiffness and damping matrices (computed for a given state)
	/// into a complete system data structure F, M, D, K (assembly)
	/// \param state The state to compute everything with
	/// \param[in,out] F The complete system force vector to add the element force into
	/// \param[in,out] M The complete system mass matrix to add the element mass matrix into
	/// \param[in,out] D The complete system damping matrix to add the element damping matrix into
	/// \param[in,out] K The complete system stiffness matrix to add the element stiffness matrix into
	/// \note This method supposes that the incoming state contains information with the same number of dof
	/// \note per node as getNumDofPerNode()
	virtual void addFMDK(const SurgSim::Math::OdeState& state,
		SurgSim::Math::Vector* F,
		SurgSim::Math::Matrix* M,
		SurgSim::Math::Matrix* D,
		SurgSim::Math::Matrix* K) = 0;

	/// Adds the element matrix-vector contribution F += (alphaM.M + alphaD.D + alphaK.K).x (computed for a given state)
	/// into a complete system data structure F (assembly)
	/// \param state The state to compute everything with
	/// \param alphaM The scaling factor for the mass contribution
	/// \param alphaD The scaling factor for the damping contribution
	/// \param alphaK The scaling factor for the stiffness contribution
	/// \param x A complete system vector to be used as the vector in the matrix-vector multiplication
	/// \param[in,out] F The complete system force vector to add the element matrix-vector contribution into
	/// \note This method supposes that the incoming state contains information with the same number of dof
	/// \note per node as getNumDofPerNode()
	virtual void addMatVec(const SurgSim::Math::OdeState& state, double alphaM, double alphaD, double alphaK,
		const SurgSim::Math::Vector& x, SurgSim::Math::Vector* F) = 0;

	/// Update the element based on a given state
	/// \param state The state to compute the update from
	/// \return True if the update is successful, False otherwise, in which case the element behavior
	/// becomes undefined. The representation should get deactivated/reset in this case.
	virtual bool update(const SurgSim::Math::OdeState& state);

	/// Determines whether a given natural coordinate is valid
	/// \param naturalCoordinate Coordinate to check
	/// \return True if valid
	bool isValidCoordinate(const SurgSim::Math::Vector& naturalCoordinate) const;

	/// Computes a given natural coordinate in cartesian coordinates
	/// \param state The state at which to transform coordinates
	/// \param naturalCoordinate The coordinates to transform
	/// \return The resultant cartesian coordinates
	virtual SurgSim::Math::Vector computeCartesianCoordinate(
		const SurgSim::Math::OdeState& state,
		const SurgSim::Math::Vector& naturalCoordinate) const = 0;

	/// Computes a natural coordinate given a global coordinate
	/// \param state The state at which to transform coordinates
	/// \param cartesianCoordinate The coordinates to transform
	/// \return The resultant natural coordinates
	virtual SurgSim::Math::Vector computeNaturalCoordinate(
		const SurgSim::Math::OdeState& state,
		const SurgSim::Math::Vector& cartesianCoordinate) const = 0;

protected:
	/// Sets the number of degrees of freedom per node
	/// \param numDofPerNode The number of dof per node
	/// \note Protected to be accessible only to derived classes which should be the only
	/// \note ones able to set this parameter
	void setNumDofPerNode(unsigned int numDofPerNode);

	/// Number of degree of freedom per node for this element
	unsigned int m_numDofPerNode;

	/// Node ids connected by this element
	std::vector<unsigned int> m_nodeIds;

	/// Mass density (in Kg.m-3)
	double m_rho;

	/// Young modulus (in N.m-2)
	double m_E;

	/// Poisson ratio (unitless)
	double m_nu;
};

} // namespace Physics

} // namespace SurgSim

#endif // SURGSIM_PHYSICS_FEMELEMENT_H
