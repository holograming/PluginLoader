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

#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <plugins/PluginLoader.hpp>
#include <plugins/MultiLibraryPluginLoader.hpp>

#include <plugins/PluginLoaderCore.hpp>

#include "gtest/gtest.h"

#include "base.hpp"

const std::string LIBRARY_1 = "PluginLoader_TestPlugins1.dll";  // NOLINT
const std::string LIBRARY_2 = "PluginLoader_TestPlugins2.dll";  // NOLINT

TEST(PluginLoaderTest, basicLoad) {
	try {
		plugin::PluginLoader loader1(LIBRARY_1, false);

		auto names = loader1.getAvailableClasses<Base>();

		loader1.createInstance<Base>("Cat")->saySomething();  // See if lazy load works
	}
	catch (plugin::PluginLoaderException & e) {
		FAIL() << "PluginLoaderException: " << e.what() << "\n";
	}

	SUCCEED();
}

TEST(PluginLoaderTest, correctNonLazyLoadUnload) {
	try {
		ASSERT_FALSE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
		plugin::PluginLoader loader1(LIBRARY_1, false);
		ASSERT_TRUE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
		ASSERT_TRUE(loader1.isLibraryLoaded());
		loader1.unloadLibrary();
		ASSERT_FALSE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
		ASSERT_FALSE(loader1.isLibraryLoaded());
		return;
	}
	catch (plugin::PluginLoaderException & e) {
		FAIL() << "PluginLoaderException: " << e.what() << "\n";
	}
	catch (...) {
		FAIL() << "Unhandled exception";
	}
}

TEST(PluginLoaderTest, correctLazyLoadUnload) {
	try {
		ASSERT_FALSE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
		plugin::PluginLoader loader1(LIBRARY_1, true);
		ASSERT_FALSE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
		ASSERT_FALSE(loader1.isLibraryLoaded());

		{
			std::shared_ptr<Base> obj = loader1.createInstance<Base>("Cat");
			ASSERT_TRUE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
			ASSERT_TRUE(loader1.isLibraryLoaded());
		}

		// The library will unload automatically when the only plugin object left is destroyed
		ASSERT_FALSE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
		return;
	}
	catch (plugin::PluginLoaderException & e) {
		FAIL() << "PluginLoaderException: " << e.what() << "\n";
	}
	catch (...) {
		FAIL() << "Unhandled exception";
	}
}

TEST(PluginLoaderTest, nonExistentPlugin) {
	plugin::PluginLoader loader1(LIBRARY_1, false);

	try {
		std::shared_ptr<Base> obj = loader1.createInstance<Base>("Bear");
		if (nullptr == obj) {
			FAIL() << "Null object being returned instead of exception thrown.";
		}

		obj->saySomething();
	}
	catch (const plugin::CreateClassException &) {
		SUCCEED();
		return;
	}
	catch (...) {
		FAIL() << "Unknown exception caught.\n";
	}

	FAIL() << "Did not throw exception as expected.\n";
}

TEST(PluginLoaderTest, nonExistentLibrary) {
	try {
		plugin::PluginLoader loader1("libDoesNotExist.dll", false);
	}
	catch (const plugin::LibraryLoadException &) {
		SUCCEED();
		return;
	}
	catch (...) {
		FAIL() << "Unknown exception caught.\n";
	}

	FAIL() << "Did not throw exception as expected.\n";
}

class InvalidBase
{
};

TEST(PluginLoaderTest, invalidBase) {
	try {
		plugin::PluginLoader loader1(LIBRARY_1, false);
		if (loader1.isClassAvailable<InvalidBase>("Cat")) {
			FAIL() << "Cat should not be available for InvalidBase";
		}
		else if (loader1.isClassAvailable<Base>("Cat")) {
			SUCCEED();
			return;
		}
		else {
			FAIL() << "Class not available for correct base class.";
		}
	}
	catch (const plugin::LibraryLoadException &) {
		FAIL() << "Unexpected exception";
	}
	catch (...) {
		FAIL() << "Unexpected and unknown exception caught.\n";
	}
}

void wait(int seconds)
{
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void run(plugin::PluginLoader * loader)
{
	std::vector<std::string> classes = loader->getAvailableClasses<Base>();
	for (auto & class_ : classes) {
		loader->createInstance<Base>(class_)->saySomething();
	}
}

TEST(PluginLoaderTest, threadSafety) {
	plugin::PluginLoader loader1(LIBRARY_1);
	ASSERT_TRUE(loader1.isLibraryLoaded());

	// Note: Hard to test thread safety to make sure memory isn't corrupted.
	// The hope is this test is hard enough that once in a while it'll segfault
	// or something if there's some implementation error.
	try {
		std::vector<std::thread *> client_threads;

		for (size_t c = 0; c < 1000; c++) {
			client_threads.push_back(new std::thread(std::bind(&run, &loader1)));
		}

		for (auto & client_thread : client_threads) {
			client_thread->join();
		}

		for (auto & client_thread : client_threads) {
			delete (client_thread);
		}

		loader1.unloadLibrary();
		ASSERT_FALSE(loader1.isLibraryLoaded());
	}
	catch (const plugin::PluginLoaderException &) {
		FAIL() << "Unexpected PluginLoaderException.";
	}
	catch (...) {
		FAIL() << "Unknown exception.";
	}
}

TEST(PluginLoaderTest, loadRefCountingNonLazy) {
	try {
		plugin::PluginLoader loader1(LIBRARY_1, false);
		ASSERT_TRUE(loader1.isLibraryLoaded());

		loader1.loadLibrary();
		loader1.loadLibrary();
		ASSERT_TRUE(loader1.isLibraryLoaded());

		loader1.unloadLibrary();
		ASSERT_TRUE(loader1.isLibraryLoaded());

		loader1.unloadLibrary();
		ASSERT_TRUE(loader1.isLibraryLoaded());

		loader1.unloadLibrary();
		ASSERT_FALSE(loader1.isLibraryLoaded());

		loader1.unloadLibrary();
		ASSERT_FALSE(loader1.isLibraryLoaded());

		loader1.loadLibrary();
		ASSERT_TRUE(loader1.isLibraryLoaded());

		return;
	}
	catch (const plugin::PluginLoaderException &) {
		FAIL() << "Unexpected exception.\n";
	}
	catch (...) {
		FAIL() << "Unknown exception caught.\n";
	}

	FAIL() << "Did not throw exception as expected.\n";
}

TEST(PluginLoaderTest, loadRefCountingLazy) {
	try {
		plugin::PluginLoader loader1(LIBRARY_1, true);
		ASSERT_FALSE(loader1.isLibraryLoaded());

		{
			std::shared_ptr<Base> obj = loader1.createInstance<Base>("Dog");
			ASSERT_TRUE(loader1.isLibraryLoaded());
		}

		ASSERT_FALSE(loader1.isLibraryLoaded());

		loader1.loadLibrary();
		ASSERT_TRUE(loader1.isLibraryLoaded());

		loader1.loadLibrary();
		ASSERT_TRUE(loader1.isLibraryLoaded());

		loader1.unloadLibrary();
		ASSERT_TRUE(loader1.isLibraryLoaded());

		loader1.unloadLibrary();
		ASSERT_FALSE(loader1.isLibraryLoaded());

		loader1.unloadLibrary();
		ASSERT_FALSE(loader1.isLibraryLoaded());

		loader1.loadLibrary();
		ASSERT_TRUE(loader1.isLibraryLoaded());

		return;
	}
	catch (const plugin::PluginLoaderException &) {
		FAIL() << "Unexpected exception.\n";
	}
	catch (...) {
		FAIL() << "Unknown exception caught.\n";
	}

	FAIL() << "Did not throw exception as expected.\n";
}

void testMultiPluginLoader(bool lazy)
{
	try {
		plugin::MultiLibraryPluginLoader loader(lazy);
		loader.loadLibrary(LIBRARY_1);
		loader.loadLibrary(LIBRARY_2);
		for (int i = 0; i < 2; ++i) {
			loader.createInstance<Base>("Cat")->saySomething();
			loader.createInstance<Base>("Dog")->saySomething();
			loader.createInstance<Base>("Robot")->saySomething();
		}
	}
	catch (plugin::PluginLoaderException & e) {
		FAIL() << "PluginLoaderException: " << e.what() << "\n";
	}

	SUCCEED();
}

TEST(MultiPluginLoaderTest, lazyLoad) {
	testMultiPluginLoader(true);
}
TEST(MultiPluginLoaderTest, lazyLoadSecondTime) {
	testMultiPluginLoader(true);
}
TEST(MultiPluginLoaderTest, nonLazyLoad) {
	testMultiPluginLoader(false);
}
TEST(MultiPluginLoaderTest, noWarningOnLazyLoad) {
	try {
		std::shared_ptr<Base> cat, dog, rob;
		{
			plugin::MultiLibraryPluginLoader loader(true);
			loader.loadLibrary(LIBRARY_1);
			loader.loadLibrary(LIBRARY_2);

			cat = loader.createInstance<Base>("Cat");
			dog = loader.createInstance<Base>("Dog");
			rob = loader.createInstance<Base>("Robot");
		}
		cat->saySomething();
		dog->saySomething();
		rob->saySomething();
	}
	catch (plugin::PluginLoaderException & e) {
		FAIL() << "PluginLoaderException: " << e.what() << "\n";
	}

	SUCCEED();
}

class Caaat : public Base
{
public:
	virtual void saySomething() { std::cout << "Caaat" << std::endl; }
};

// Run all the tests that were declared with TEST()
int main(int argc, char ** argv)
{


	plugin::setLogLevel(plugin::CONSOLE_LOG_DEBUG);
	//plugin::impl::registerPlugin<Caaat, Base>("Caaat", "Base");
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

