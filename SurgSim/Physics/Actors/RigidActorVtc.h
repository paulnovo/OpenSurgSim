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

#ifndef SURGSIM_PHYSICS_RIGIDACTORVTC_H
#define SURGSIM_PHYSICS_RIGIDACTORVTC_H

#include <SurgSim/Physics/Actors/RigidActorBase.h>
#include <SurgSim/Physics/Actors/RigidActorState.h>
#include <SurgSim/Physics/Actors/RigidActorParameters.h>

#include <SurgSim/Physics/Actors/RigidVtcParameters.h>

#include <SurgSim/Math/Vector.h>
#include <SurgSim/Math/Matrix.h>
#include <SurgSim/Math/RigidTransform.h>

namespace SurgSim 
{

namespace Physics
{

/// The RigidActorVtc class defines a rigid body actor associated with a
/// Virtual tool coupler (i.e. god-object or proxy)
/// Note that the rigid actor is velocity-based, therefore its degrees of
/// freedom are the linear and angular velocities: 6 Dof
/// \note The Rigid actor is driven by the Vtc through setPose(...)
class RigidActorVtc : public RigidActorBase
{
public:
	/// Constructor
	/// \param name The rigid actor's name
	explicit RigidActorVtc(const std::string& name);

	/// Destructor
	virtual ~RigidActorVtc();

	/// Set the initial state of the rigid actor
	/// \param state The initial state (pose + lin/ang velocities)
	void setInitialState(const RigidActorState& state)
	{
		m_initialState = state;
		m_currentState = state;
		m_previousState = state;

		updateGlobalInertiaMatrices(m_currentState);
	};

	/// Set the initial parameters of the rigid actor
	/// \param parameters The initial parameters
	void setInitialParameters(const RigidActorParameters& parameters)
	{
		m_initialParameters = parameters;
		m_currentParameters = parameters;

		updateGlobalInertiaMatrices(m_currentState);
	};

	/// Set the current parameters of the rigid actor
	/// \param parameters The current parameters
	void setCurrentParameters(const RigidActorParameters& parameters)
	{
		m_currentParameters = parameters;

		updateGlobalInertiaMatrices(m_currentState);
	};

	/// Get the initial state of the rigid actor
	/// \return The initial state (pose + lin/ang velocities)
	const RigidActorState& getInitialState() const
	{
		return m_initialState;
	};

	/// Get the initial parameters of the rigid actor
	/// \return The initial parameters of the rigid actor
	const RigidActorParameters& getInitialParameters() const
	{
		return m_initialParameters;
	};

	/// Get the current state of the rigid actor
	/// \return The current state (pose + lin/ang velocities)
	const RigidActorState& getCurrentState() const
	{
		return m_currentState;
	};

	/// Get the previous state of the rigid actor
	/// \return The previous state (pose + lin/ang velocities)
	const RigidActorState& getPreviousState() const
	{
		return m_previousState;
	};

	/// Get the current parameters of the rigid actor
	/// \return The current parameters of the rigid actor
	const RigidActorParameters& getCurrentParameters() const
	{
		return m_currentParameters;
	};

	/// Set the initial Vtc proxy state
	/// \param state The initial Vtc state (pose + lin/ang velocities)
	void setInitialVtcState(const RigidActorState& state)
	{
		m_initialVtcState = state;
		m_currentVtcState = state;
		m_previousVtcState = state;
	};

	/// Set the initial Vtc parameters
	/// \param parameters The initial Vtc parameters
	void setInitialVtcParameters(const RigidVtcParameters& parameters)
	{
		m_initialVtcParameters = parameters;
		m_currentVtcParameters = parameters;
	};

	/// Set the current Vtc parameters
	/// \param parameters The current Vtc parameters
	void setCurrentVtcParameters(const RigidVtcParameters& parameters)
	{
		m_currentVtcParameters = parameters;
	};

	/// Get the initial Vtc state
	/// \return The initial Vtc state (pose + lin/ang velocities)
	const RigidActorState& getInitialVtcState() const
	{
		return m_initialVtcState;
	};

	/// Get the initial Vtc parameters
	/// \return The initial Vtc parameters
	const RigidVtcParameters& getInitialVtcParameters() const
	{
		return m_initialVtcParameters;
	};

	/// Get the current Vtc state
	/// \return The current Vtc state (pose + lin/ang velocities)
	const RigidActorState& getCurrentVtcState() const
	{
		return m_currentVtcState;
	};

	/// Get the previous Vtc state
	/// \return The previous Vtc state (pose + lin/ang velocities)
	const RigidActorState& getPreviousVtcState() const
	{
		return m_previousVtcState;
	};

	/// Get the current Vtc parameters
	/// \return The current Vtc parameters
	const RigidVtcParameters& getCurrentVtcParameters() const
	{
		return m_currentVtcParameters;
	};

	/// Set the initial pose of the rigid actor
	/// \param transformation The initial pose (translation + rotation)
	/// \note This sets the current/previous poses as well
	void setInitialPose(const SurgSim::Math::RigidTransform3d& pose)
	{
		m_initialState.setPose(pose);
		m_currentState.setPose(pose);
		m_previousState.setPose(pose);

		updateGlobalInertiaMatrices(m_currentState);
	};

	/// Get the initial pose of the rigid actor
	/// \return The initial pose (translation + rotation)
	const SurgSim::Math::RigidTransform3d& getInitialPose() const
	{
		return m_initialState.getPose();
	};

	/// Set the current pose of the rigid actor
	/// \param pose The current pose (translation + rotation)
	/// \note This is done through the Vtc proxy !
	/// \note We let the end-user drive the Vtc, not the virtual
	/// \note rigid actor directly
	void setPose(const SurgSim::Math::RigidTransform3d& pose)
	{
		m_currentVtcState.setPose(pose);
	};

	/// Get the current pose of the rigid actor
	/// \return The current pose (translation + rotation)
	/// \note The end-user set the pose of the Vtc but retrieve
	/// \note information from the virtual rigid actor
	const SurgSim::Math::RigidTransform3d& getPose() const
	{
		return m_currentState.getPose();
	};

	/// Preprocessing done before the update call
	/// \param dt The time step (in seconds)
	void beforeUpdate(double dt);

	/// Update the actor state to the current time step
	/// \param dt The time step (in seconds)
	void update(double dt);

	/// Postprocessing done after the update call
	/// \param dt The time step (in seconds)
	void afterUpdate(double dt);

	/// Reset the rigid actor state to its initial state
	/// \note This reset the rigid actor state but not the Vtc state
	/// \note The Vtc is controlled externally via setPose()
	void resetState()
	{
		RigidActorBase::resetState();

		m_currentState  = m_initialState;
		m_previousState = m_initialState;

		updateGlobalInertiaMatrices(m_currentState);
	};

	/// Reset the rigid actor parameters to their initial values
	void resetParameters()
	{
		RigidActorBase::resetParameters();

		m_currentParameters = m_initialParameters;

		updateGlobalInertiaMatrices(m_currentState);
	};

	/// Reset the Vtc parameters to their initial values
	void resetVtcParameters()
	{
		m_currentVtcParameters = m_initialVtcParameters;
	};

protected:
	/// Inertia matrices in global coordinates
	SurgSim::Math::Matrix33d m_globalInertia;

	/// Inverse of inertia matrix in global coordinates
	SurgSim::Math::Matrix33d m_invGlobalInertia;

	/// Current force applied on the rigid actor (in N)
	SurgSim::Math::Vector3d m_force;

	/// Current torque applied on the rigid actor (in N.m)
	SurgSim::Math::Vector3d m_torque;

	/// Compliance matrix (size of the number of Dof = 6)
	Eigen::Matrix<double, 6,6, Eigen::DontAlign | Eigen::RowMajor> m_C;

private:
	/// Compute compliance matrix (internal data structure)
	/// \param dt The time step in use
	void computeComplianceMatrix(double dt);

	/// Update global inertia matrices (internal data structure)
	/// \param state The state of the rigid actor to use for the update
	void updateGlobalInertiaMatrices(const RigidActorState& state);

	/// Initial rigid actor state (useful for reset)
	RigidActorState m_initialState;

	/// Previous rigid actor state
	RigidActorState m_previousState;

	/// Current rigid actor state
	RigidActorState m_currentState;

	/// Initial physical parameters
	RigidActorParameters m_initialParameters;

	/// Current physical parameters
	RigidActorParameters m_currentParameters;

	/// Initial Vtc state (useful for reset)
	RigidActorState m_initialVtcState;

	/// Previous Vtc state
	RigidActorState m_previousVtcState;

	/// Current Vtc state
	RigidActorState m_currentVtcState;

	/// Initial Vtc parameters
	RigidVtcParameters m_initialVtcParameters;

	/// Current Vtc parameters
	RigidVtcParameters m_currentVtcParameters;
};

}; /// Physics

}; /// SurgSim

#endif /// SURGSIM_PHYSICS_RIGIDACTORVTC_H
