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

#ifndef SURGSIM_FRAMEWORK_LOGGER_H
#define SURGSIM_FRAMEWORK_LOGGER_H

#include "SurgSim/Framework/LoggerManager.h"
#include "SurgSim/Framework/LogOutput.h"

#include <string>
#include <memory>

namespace SurgSim
{
namespace Framework
{

/// \addtogroup loggingAPI
/// @{

/// Logging levels.
/// Please note that most logging macros take an abbreviated version of these enumerations, without the leading
/// \c LOG_LEVEL_, i.e. one of \c DEBUG, \c INFO, \c WARNING, \c SEVERE or \c CRITICAL .
enum LogLevel
{
	/// Use at your discretion.
	LOG_LEVEL_DEBUG,
	/// Informational, notify of state changes.
	LOG_LEVEL_INFO,
	/// Something failed, but the impact of the failure is not know or minimal (e.g. purely visual).
	LOG_LEVEL_WARNING,
	/// Something failed and will impact functionality, some parts of the program will not function correctly.
	LOG_LEVEL_SEVERE,
	/// Used by assertion, after using this level the program will not be functional at all.
	LOG_LEVEL_CRITICAL
};

/// An object that can be used to control logging parameters, such as verbosity and log output destination.
class Logger
{
public:

	friend class LoggerManager;

	/// Destructor.
	~Logger()
	{
	}

	/// Uses the contained instance of LogOutput to write the log message
	/// \return true on success
	/// \param message the message to be printed
	bool writeMessage(const std::string& message)
	{
		return m_output->writeMessage(message);
	}

	/// Gets the logging threshold.
	/// Anything message with less than this level will be ignored.
	/// \return The threshold value.
	int getThreshold() const
	{
		return m_threshold;
	}

	/// Sets the logging threshold.
	/// Anything message with less than this level will be ignored.
	/// \param val The value to be used as the threshold.
	void setThreshold(int val)
	{
		m_threshold = val;
	}

	/// Gets the output object used by this logger.
	/// \return The current output object used this logger.
	std::shared_ptr<LogOutput> getOutput() const
	{
		return m_output;
	}

	/// Sets the output object used by this logger.
	/// \param val The output object to be used.
	void setOutput(std::shared_ptr<LogOutput> val)
	{
		m_output = val;
	}

	/// Gets this logger's name.
	/// \return The name.
	std::string getName( ) const
	{
		return m_name;
	}

	/// Get a logger by name from Logger Manager
	/// \return A logger with given name.
	static std::shared_ptr<Logger> getLogger(const std::string& name)
	{
		return getLoggerManager()->getLogger(name);
	}

	/// Get default logger
	/// \return Default logger
	static std::shared_ptr<Logger> getDefaultLogger()
	{
		return getLoggerManager()->getDefaultLogger();
	}

	/// Get the logger manager
	/// \return Logger Manager that manages all loggers
	static std::shared_ptr<LoggerManager> getLoggerManager();

private:
	/// Constructor.
	/// \param name The name used for this logger.
	/// \param output The LogOutput instance used to display or log the data.
	Logger(const std::string& name, std::shared_ptr<LogOutput> output);

	int m_threshold;
	std::string m_name;
	std::shared_ptr<LogOutput> m_output;
};


/// @}

}; // namespace Framework
}; // namespace SurgSim

#endif  // SURGSIM_FRAMEWORK_LOGGER_H
