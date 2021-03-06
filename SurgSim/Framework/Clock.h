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

#ifndef SURGSIM_FRAMEWORK_CLOCK_H
#define SURGSIM_FRAMEWORK_CLOCK_H

#include <boost/chrono.hpp>
#include <boost/thread.hpp>

/// \file
/// Place for a simple wrapper around boost


namespace SurgSim
{
namespace Framework
{

/// Wraps around the actual clock we are using.
typedef boost::chrono::high_resolution_clock Clock;

/// A more accurate sleep_until that accounts for scheduler errors
/// \tparam C Clock type
/// \tparam D Duration type
/// \param time The time point in absolute time to sleep until
template <class C, class D>
void sleep_until(const boost::chrono::time_point<C, D>& time)
{
	// 2ms gives good results on windows and linux
	static const boost::chrono::duration<double> schedulerError(0.002);

	boost::chrono::time_point<C, D> earlierTime = time - schedulerError;
	if (earlierTime > C::now())
	{
		boost::this_thread::sleep_until(earlierTime);
	}

	while (C::now() < time)
	{
		boost::this_thread::yield();
	}
}

}; // Framework
}; // SurgSim

#endif
