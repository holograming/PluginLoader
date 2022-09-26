/*
 * Copyright (c) 2012, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string>


#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <plugins/PluginLoader.hpp>

#include "gtest/gtest.h"

#include "base.hpp"

const std::string LIBRARY_1 = "PluginLoader_TestPluginsMathFunctions.dll";  // NOLINT

const double PARAM1 = 1.0;
const double PARAM2 = 2.0;

TEST(PluginLoaderTest, PlusOperation) {
	try {
		plugin::PluginLoader loader1(LIBRARY_1, false);

		if (loader1.isClassAvailable<Base>("PlusOperation")) {
			std::cout << PARAM1 << " + " << PARAM2 << std::endl;
			ASSERT_TRUE((loader1.createInstance<Base>("PlusOperation")->mathFunctions(PARAM1, PARAM2) == 3.0));
		}
		else {
			FAIL() << "Class not available for correct base class.";
		}
	}
	catch (plugin::PluginLoaderException & e) {
		FAIL() << "PluginLoaderException: " << e.what() << "\n";
	}

	SUCCEED();
}

TEST(PluginLoaderTest, SubstractOperation) {
	try {
		plugin::PluginLoader loader1(LIBRARY_1, false);

		if (loader1.isClassAvailable<Base>("SubstractOperation")) {
			std::cout << PARAM1 << " - " << PARAM2 << std::endl;
			ASSERT_TRUE((loader1.createInstance<Base>("SubstractOperation")->mathFunctions(PARAM1, PARAM2) == -1.0));
		}
		else {
			FAIL() << "Class not available for correct base class.";
		}
	}
	catch (plugin::PluginLoaderException & e) {
		FAIL() << "PluginLoaderException: " << e.what() << "\n";
	}

	SUCCEED();
}

TEST(PluginLoaderTest, MultiplyOperation) {
	try {
		plugin::PluginLoader loader1(LIBRARY_1, false);

		if (loader1.isClassAvailable<Base>("MultiplyOperation")) {
			std::cout << PARAM1 << " x " << PARAM2 << std::endl;
			ASSERT_FALSE((loader1.createInstance<Base>("MultiplyOperation")->mathFunctions(PARAM1, PARAM2) == 1.0));
		}
		else {
			FAIL() << "Class not available for correct base class.";
		}
	}
	catch (plugin::PluginLoaderException & e) {
		FAIL() << "PluginLoaderException: " << e.what() << "\n";
	}

	SUCCEED();
}

TEST(PluginLoaderTest, DivideOperation) {
	try {
		plugin::PluginLoader loader1(LIBRARY_1, false);

		if (loader1.isClassAvailable<Base>("DivideOperation")) {
			std::cout << PARAM1 << " / " << PARAM2 << std::endl;
			ASSERT_TRUE((loader1.createInstance<Base>("DivideOperation")->mathFunctions(PARAM1, PARAM2) == 0.5));
		}
		else {
			FAIL() << "Class not available for correct base class.";
		}
	}
	catch (plugin::PluginLoaderException & e) {
		FAIL() << "PluginLoaderException: " << e.what() << "\n";
	}

	SUCCEED();
}

// Run all the tests that were declared with TEST()
int main(int argc, char ** argv)
{
	//plugin::setLogLevel(plugin::CONSOLE_LOG_DEBUG);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}