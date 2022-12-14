/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Ryan Luna, Ioan Sucan */

#ifndef PLUGIN_CONSOLE_CONSOLE_
#define PLUGIN_CONSOLE_CONSOLE_

#include "VisibilityControl.h"
#include <string>

namespace plugin
{

/** \file console.h
	\defgroup logging Logging Macros
	\{

	\def CONSOLE_logError(fmt, ...)
	\brief Log a formatted error string.
	\remarks This macro takes the same arguments as <a href="http://www.cplusplus.com/reference/clibrary/cstdio/printf">printf</a>.

	\def CONSOLE_logWarn(fmt, ...)
	\brief Log a formatted warning string.
	\remarks This macro takes the same arguments as <a href="http://www.cplusplus.com/reference/clibrary/cstdio/printf">printf</a>.

	\def CONSOLE_logInform(fmt, ...)
	\brief Log a formatted information string.
	\remarks This macro takes the same arguments as <a href="http://www.cplusplus.com/reference/clibrary/cstdio/printf">printf</a>.

	\def CONSOLE_logDebug(fmt, ...)
	\brief Log a formatted debugging string.
	\remarks This macro takes the same arguments as <a href="http://www.cplusplus.com/reference/clibrary/cstdio/printf">printf</a>.

	\}
*/

/** 
\brief Message namespace. This contains classes needed to
	output error messages (or logging) from within the library.
	Message logging can be performed with \ref logging "logging macros" 
*/

/** \brief The set of priorities for message logging */
enum LogLevel
{
	CONSOLE_LOG_DEBUG = 0,
	CONSOLE_LOG_INFO,
	CONSOLE_LOG_WARN,
	CONSOLE_LOG_ERROR,
	CONSOLE_LOG_NONE
};

/** 
\brief Generic class to handle output from a piece of code.
		In order to handle output from the library in different
		ways, an implementation of this class needs to be
		provided. This instance can be set with the useOutputHandler function.
*/
class OutputHandler
{
public:
	OutputHandler() = default;
	virtual ~OutputHandler() = default;

	/** \brief log a message to the output handler with the given text
	and logging level from a specific file and line number */
	virtual void log(const std::string &text, LogLevel level, const char *filename, int line) = 0;
};

/** \brief Default implementation of OutputHandler. This sends
	the information to the console. */
class OutputHandlerSTD : public OutputHandler
{
public:
	OutputHandlerSTD() = default;
	virtual void log(const std::string &text, LogLevel level, const char *filename, int line);
};

/** \brief Implementation of OutputHandler that saves messages in a file. */
class  OutputHandlerFile : public OutputHandler
{
public:
	/** \brief The name of the file in which to save the message data */
	OutputHandlerFile(const char *filename);
	virtual ~OutputHandlerFile(void);
	virtual void log(const std::string &text, LogLevel level, const char *filename, int line);
private:

	/** \brief The file to save to */
	FILE *file_;
};

/** \brief This function instructs ompl that no messages should be outputted. Equivalent to useOutputHandler(NULL) */
PLUGIN_LOADER_PUBLIC
void noOutputHandler(void);

/** \brief Restore the output handler that was previously in use (if any) */
PLUGIN_LOADER_PUBLIC
void restorePreviousOutputHandler(void);

/** \brief Specify the instance of the OutputHandler to use. By default, this is OutputHandlerSTD */
PLUGIN_LOADER_PUBLIC
void useOutputHandler(OutputHandler *oh);

/** \brief Get the instance of the OutputHandler currently used. This is NULL in case there is no output handler. */
PLUGIN_LOADER_PUBLIC
OutputHandler* getOutputHandler(void);

/** \brief Set the minimum level of logging data to output.  Messages
	with lower logging levels will not be recorded. */
PLUGIN_LOADER_PUBLIC
void setLogLevel(LogLevel level);

/** \brief Retrieve the current level of logging data.  Messages
	with lower logging levels will not be recorded. */
PLUGIN_LOADER_PUBLIC
LogLevel getLogLevel(void);

/** \brief Root level logging function.  This should not be invoked directly,
	but rather used via a \ref logging "logging macro".  Formats the message
	string given the arguments and forwards the string to the output handler */
PLUGIN_LOADER_PUBLIC
void log(const char *file, int line, LogLevel level, const char* m, ...);


template<typename... Args> inline
void logError(const char* fmt, Args... args) {
	plugin::log(__FILE__, __LINE__, plugin::CONSOLE_LOG_ERROR, fmt, args...);
}

template<typename... Args> inline
void logWarn(const char* fmt, Args... args) {
	plugin::log(__FILE__, __LINE__, plugin::CONSOLE_LOG_WARN, fmt, args...);
}

template<typename... Args> inline
void logInform(const char* fmt, Args... args) {
	plugin::log(__FILE__, __LINE__, plugin::CONSOLE_LOG_INFO, fmt, args...);
}

template<typename... Args> inline
void logDebug(const char* fmt, Args... args) {
	plugin::log(__FILE__, __LINE__, plugin::CONSOLE_LOG_DEBUG, fmt, args...);
}

} // namespace plugin

#endif // PLUGIN_CONSOLE_CONSOLE_