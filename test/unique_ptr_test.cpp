/*
 * Copyright (c) 2012, Willow Garage, Inc.
 * Copyright (c) 2016, Delft Robotics B.V.
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

#include <plugins/PluginLoader.hpp>
#include <plugins/MultiLibraryPluginLoader.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "base.hpp"

const std::string LIBRARY_1 = "PluginLoader_TestPlugins1.dll";  // NOLINT
const std::string LIBRARY_2 = "PluginLoader_TestPlugins2.dll";  // NOLINT

using plugin::PluginLoader;

TEST(PluginLoaderUniquePtrTest, basicLoad) {
  try {
    PluginLoader loader1(LIBRARY_1, false);
    loader1.createUniqueInstance<Base>("Cat")->saySomething();  // See if lazy load works
    SUCCEED();
  } catch (plugin::PluginLoaderException & e) {
    FAIL() << "PluginLoaderException: " << e.what() << "\n";
  }
}

TEST(PluginLoaderUniquePtrTest, correctLazyLoadUnload) {
  try {
    ASSERT_FALSE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
    PluginLoader loader1(LIBRARY_1, true);
    ASSERT_FALSE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
    ASSERT_FALSE(loader1.isLibraryLoaded());

    {
      PluginLoader::UniquePtr<Base> obj = loader1.createUniqueInstance<Base>("Cat");
      ASSERT_TRUE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
      ASSERT_TRUE(loader1.isLibraryLoaded());
    }

    // The library will unload automatically when the only plugin object left is destroyed
    ASSERT_FALSE(plugin::impl::isLibraryLoadedByAnybody(LIBRARY_1));
    return;
  } catch (plugin::PluginLoaderException & e) {
    FAIL() << "PluginLoaderException: " << e.what() << "\n";
  } catch (...) {
    FAIL() << "Unhandled exception";
  }
}

TEST(PluginLoaderUniquePtrTest, nonExistentPlugin) {
  PluginLoader loader1(LIBRARY_1, false);

  try {
    PluginLoader::UniquePtr<Base> obj = loader1.createUniqueInstance<Base>("Bear");
    if (nullptr == obj) {
      FAIL() << "Null object being returned instead of exception thrown.";
    }

    obj->saySomething();
  } catch (const plugin::CreateClassException &) {
    SUCCEED();
    return;
  } catch (...) {
    FAIL() << "Unknown exception caught.\n";
  }

  FAIL() << "Did not throw exception as expected.\n";
}

void wait(int seconds)
{
  std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void run(PluginLoader * loader)
{
  std::vector<std::string> classes = loader->getAvailableClasses<Base>();
  for (auto & class_ : classes) {
    loader->createUniqueInstance<Base>(class_)->saySomething();
  }
}

TEST(PluginLoaderUniquePtrTest, threadSafety) {
  PluginLoader loader1(LIBRARY_1);
  ASSERT_TRUE(loader1.isLibraryLoaded());

  // Note: Hard to test thread safety to make sure memory isn't corrupted.
  // The hope is this test is hard enough that once in a while it'll segfault
  // or something if there's some implementation error.
  try {
    std::vector<std::thread> client_threads;

    for (size_t c = 0; c < 1000; c++) {
      client_threads.emplace_back(std::bind(&run, &loader1));
    }

    for (auto & client_thread : client_threads) {
      client_thread.join();
    }

    loader1.unloadLibrary();
    ASSERT_FALSE(loader1.isLibraryLoaded());
  } catch (const plugin::PluginLoaderException &) {
    FAIL() << "Unexpected PluginLoaderException.";
  } catch (...) {
    FAIL() << "Unknown exception.";
  }
}

TEST(PluginLoaderUniquePtrTest, loadRefCountingLazy) {
  try {
    PluginLoader loader1(LIBRARY_1, true);
    ASSERT_FALSE(loader1.isLibraryLoaded());

    {
      PluginLoader::UniquePtr<Base> obj = loader1.createUniqueInstance<Base>("Dog");
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
  } catch (const plugin::PluginLoaderException &) {
    FAIL() << "Unexpected exception.\n";
  } catch (...) {
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
      loader.createUniqueInstance<Base>("Cat")->saySomething();
      loader.createUniqueInstance<Base>("Dog")->saySomething();
      loader.createUniqueInstance<Base>("Robot")->saySomething();
    }
  } catch (plugin::PluginLoaderException & e) {
    FAIL() << "PluginLoaderException: " << e.what() << "\n";
  }

  SUCCEED();
}

TEST(MultiPluginLoaderUniquePtrTest, lazyLoad) {
  testMultiPluginLoader(true);
}
TEST(MultiPluginLoaderUniquePtrTest, lazyLoadSecondTime) {
  testMultiPluginLoader(true);
}
TEST(MultiPluginLoaderUniquePtrTest, nonLazyLoad) {
  testMultiPluginLoader(false);
}
TEST(MultiPluginLoaderUniquePtrTest, noWarningOnLazyLoad) {
  try {
    PluginLoader::UniquePtr<Base> cat = nullptr, dog = nullptr, rob = nullptr;
    {
      plugin::MultiLibraryPluginLoader loader(true);
      loader.loadLibrary(LIBRARY_1);
      loader.loadLibrary(LIBRARY_2);

      cat = loader.createUniqueInstance<Base>("Cat");
      dog = loader.createUniqueInstance<Base>("Dog");
      rob = loader.createUniqueInstance<Base>("Robot");
    }
    cat->saySomething();
    dog->saySomething();
    rob->saySomething();
  } catch (plugin::PluginLoaderException & e) {
    FAIL() << "PluginLoaderException: " << e.what() << "\n";
  }

  SUCCEED();
}

// Run all the tests that were declared with TEST()
int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
