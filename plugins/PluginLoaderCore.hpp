/*
 * Software License Agreement (BSD License)
 *
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
 *     * Neither the name of the copyright holders nor the names of its
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

#ifndef PLUGIN_IMPL_CORE_HPP_
#define PLUGIN_IMPL_CORE_HPP_

#include <mutex>
#include <cstddef>
#include <cstdio>
#include <map>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "MetaObject.hpp"
#include "Console.h"
#include "SharedLibrary.hpp"
#include "PluginMacro.hpp"
#include "Exceptions.hpp"
#include "VisibilityControl.h"


namespace plugin {

// Forward declaration
class PluginLoader;  

namespace impl {

// Typedefs
typedef std::string LibraryPath;
typedef std::string ClassName;
typedef std::string BaseClassName;
typedef std::map<ClassName, AbstractMetaObjectBase*> FactoryMap;
typedef std::map<BaseClassName, FactoryMap> BaseToFactoryMapMap; // Todo : ???? mapmap -> map
typedef std::pair<LibraryPath, SharedLibrary*> LibraryPair;
typedef std::vector<LibraryPair> LibraryVector;
typedef std::vector<AbstractMetaObjectBase*> MetaObjectVector;

///////////////////////////////////////////////////////////////////////////////////
// Debug
//////////////////////////////////////////////////////////////////////////////////
PLUGIN_LOADER_PUBLIC
void printDebugInfoToScreen();

///////////////////////////////////////////////////////////////////////////////////
// Global storage
// Singletone ?? ???? static ?????? ???????? ???????? ???? ?????? function ????
//////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Gets a handle to a global data structure that holds a map of base class names (Base class describes plugin interface) to a FactoryMap which holds the factories for the various different concrete classes that can be instantiated. Note that the Base class is NOT THE LITERAL CLASSNAME, but rather the result of typeid(Base).name() which sometimes is the literal class name (as on Windows) but is often in mangled form (as on Linux).
 * @return A reference to the global base to factory map
 * ???? ?????? base map ???? ????.
 * inline ?????? ????
 */
PLUGIN_LOADER_PUBLIC
BaseToFactoryMapMap& getGlobalPluginBaseToFactoryMapMap();

/**
 * @brief Gets a handle to a list of open libraries in the form of LibraryPairs which encode the library path+name and the handle to the underlying SharedLibrary
 * @return A reference to the global vector that tracks loaded libraries
 * ???? ?????? LibraryPair ???? ????.
 * inline ?????? ????
 */
PLUGIN_LOADER_PUBLIC
LibraryVector& getLoadedLibraryVector();

/**
 * @brief Gets the PluginLoader currently in scope which used when a library is being loaded.
 * @return A pointer to the currently active PluginLoader.
 * ???? ???????? ?????? ???? ptr ????.
 * inline ?????? ????
 */
PLUGIN_LOADER_PUBLIC
PluginLoader* getCurrentlyActivePluginLoader();

/**
 * @brief When a library is being loaded, in order for factories to know which library they are being associated with, they use this function to query which library is being loaded.
 * @return The currently set loading library name as a string
 * ???? ???????? ?????????? ???? ????.
 */
PLUGIN_LOADER_PUBLIC
std::string getCurrentlyLoadingLibraryName();

/**
 * @brief When a library is being loaded, in order for factories to know which library they are being associated with, this function is called to set the name of the library currently being loaded.
 * @param library_name - The name of library that is being loaded currently
 * ???? ???????? ?????????? ???? ????
 */
PLUGIN_LOADER_PUBLIC
void setCurrentlyLoadingLibraryName(std::string const& library_name);


/**
 * @brief Sets the PluginLoader currently in scope which used when a library is being loaded.
 * @param loader - pointer to the currently active PluginLoader.
 * ???? ???????? ?????? ???? ??
 */
PLUGIN_LOADER_PUBLIC
void setCurrentlyActivePluginLoader(PluginLoader* const loader);


/**
 * @brief Gets the PluginLoader currently in scope which used when a library is being loaded.
 * @return A pointer to the currently active PluginLoader.
 */
PLUGIN_LOADER_PUBLIC
PluginLoader* getCurrentlyActivePluginLoader();

/**
 * @brief This function extracts a reference to the FactoryMap for appropriate base class out of the global plugin base to factory map. This function should be used by functions in this namespace that need to access the various factories so as to make sure the right key is generated to index into the global map.
 * @return A reference to the FactoryMap contained within the global Base-to-FactoryMap map.
 */
PLUGIN_LOADER_PUBLIC
FactoryMap& getFactoryMapForBaseClass(const std::string & typeid_base_class_name);

/**
 * @brief Same as above but uses a type parameter instead of string for more safety if info is available.
 * @return A reference to the FactoryMap contained within the global Base-to-FactoryMap map.
 * Base ?????? class name ????.
 */
template<typename Base>
FactoryMap& getFactoryMapForBaseClass() {
	return getFactoryMapForBaseClass(typeid(Base).name());
}

/**
 * @brief To provide thread safety, all exposed plugin functions can only be run serially by multiple threads. This is implemented by using critical sections enforced by a single mutex which is locked and released with the following two functions
 * @return A reference to the global mutex
 * ???? mutex ???? ????
 * inline ?????? ????
 */
PLUGIN_LOADER_PUBLIC
std::recursive_mutex& getLoadedLibraryVectorMutex();

PLUGIN_LOADER_PUBLIC
std::recursive_mutex& getPluginBaseToFactoryMapMapMutex();

/**
 * @brief Indicates if a library containing more than just plugins has been opened by the running process
 * @return True if a non-pure plugin library has been opened, otherwise false
 */
PLUGIN_LOADER_PUBLIC
bool hasANonPurePluginLibraryBeenOpened();

/**
 * @brief Sets a flag indicating if a library containing more than just plugins has been opened by the running process
 * @param hasIt - The flag
 * 
 */
PLUGIN_LOADER_PUBLIC
void hasANonPurePluginLibraryBeenOpened(const bool hasIt);

// -- End of Global storage area
// -------------------------------------------------------------------------  //


///////////////////////////////////////////////////////////////////////////////////
// Plugin Functions
//////////////////////////////////////////////////////////////////////////////////

/**
 * @brief This function is called by the plugin_loader_REGISTER_CLASS macro in plugin_register_macro.h to register factories.
 * Classes that use that macro will cause this function to be invoked when the library is loaded. The function will create a MetaObject (i.e. factory) 
	for the corresponding Derived class and insert it into the appropriate FactoryMap in the global Base-to-FactoryMap map. 
	Note that the passed class_name is the literal class name and not the mangled version.

 * @param Derived - parameteric type indicating concrete type of plugin
 * @param Base - parameteric type indicating base type of plugin
 * @param class_name - the literal name of the class being registered (NOT MANGLED)
 */
template<typename Derived, typename Base>
void registerPlugin(std::string const& class_name, std::string const& base_class_name)
{
	// Note: This function will be automatically invoked when a dlopen() call
	// opens a library. Normally it will happen within the scope of loadLibrary(),
	// but that may not be guaranteed.
	
	char temp[512];
	sprintf_s(temp, 512, "plugin.impl: "
		"Registering plugin factory for class = %s, PluginLoader* = %p and library name %s.",
		class_name.c_str(), getCurrentlyActivePluginLoader(),
		getCurrentlyLoadingLibraryName().c_str());
	std::string sss(temp);
 	logDebug("%s", sss.c_str());

	if (nullptr == getCurrentlyActivePluginLoader()) {
		logDebug("%s",
			"plugin_loader.impl: ALERT!!! "
			"A library containing plugins has been opened through a means other than through the "
			"plugin_loader or pluginlib package. "
			"This can happen if you build plugin libraries that contain more than just plugins "
			"(i.e. normal code your app links against). "
			"This inherently will trigger a dlopen() prior to main() and cause problems as plugin_loader "
			"is not aware of plugin factories that autoregister under the hood. "
			"The plugin_loader package can compensate, but you may run into namespace collision problems "
			"(e.g. if you have the same plugin class in two different libraries and you load them both "
			"at the same time). "
			"The biggest problem is that library can now no longer be safely unloaded as the "
			"PluginLoader does not know when non-plugin code is still in use. "
			"In fact, no PluginLoader instance in your application will be unable to unload any library "
			"once a non-pure one has been opened. "
			"Please refactor your code to isolate plugins into their own libraries.");
		hasANonPurePluginLibraryBeenOpened(true);
	}

	// Create factory
	AbstractMetaObjectBase* new_factory = new MetaObject<Derived, Base>(class_name, base_class_name);
	new_factory->addOwningPluginLoader(getCurrentlyActivePluginLoader());
	new_factory->setAssociatedLibraryPath(getCurrentlyLoadingLibraryName());


	// Add it to global factory map map
	getPluginBaseToFactoryMapMapMutex().lock();
	FactoryMap& factoryMap = getFactoryMapForBaseClass<Base>();
	if (factoryMap.find(class_name) != factoryMap.end()) {
		logWarn(
		  "plugin_loader.impl: SEVERE WARNING!!! "
		  "A namespace collision has occured with plugin factory for class %s. "
		  "New factory will OVERWRITE existing one. "
		  "This situation occurs when libraries containing plugins are directly linked against an "
		  "executable (the one running right now generating this message). "
		  "Please separate plugins out into their own library or just don't link against the library "
		  "and use either plugin_loader::PluginLoader/MultiLibraryPluginLoader to open.",
		  class_name.c_str());
	}
	factoryMap[class_name] = new_factory;
	getPluginBaseToFactoryMapMapMutex().unlock();

	logDebug(
	  "plugin_loader.impl: "
	  "Registration of %s complete (Metaobject Address = %p)",
	  class_name.c_str(), reinterpret_cast<void *>(new_factory));
}

/**
 * @brief This function creates an instance of a plugin class given the derived name of the class and returns a pointer of the Base class type.
 * @param derived_class_name - The name of the derived class (unmangled)
 * @param loader - The PluginLoader whose scope we are within
 * @return A pointer to newly created plugin, note caller is responsible for object destruction
 */
template<typename Base>
Base* createInstance(const std::string& derived_class_name, PluginLoader* loader)
{
	AbstractMetaObject<Base>* factory = nullptr;

	getPluginBaseToFactoryMapMapMutex().lock();
	FactoryMap & factoryMap = getFactoryMapForBaseClass<Base>();
	if (factoryMap.find(derived_class_name) != factoryMap.end()) {
		factory = dynamic_cast<AbstractMetaObject<Base> *>(factoryMap[derived_class_name]);
	}
	else {
		logError(
		  "plugin_loader.impl: No metaobject exists for class type %s.", derived_class_name.c_str());
	}
	getPluginBaseToFactoryMapMapMutex().unlock();

	Base * obj = nullptr;
	if (factory != nullptr && factory->isOwnedBy(loader)) {
		obj = factory->create();
	}

	if (nullptr == obj) {  // Was never created
		if (factory && factory->isOwnedBy(nullptr)) {
			logDebug("%s",
			    "plugin_loader.impl: ALERT!!! "
			    "A metaobject (i.e. factory) exists for desired class, but has no owner. "
			    "This implies that the library containing the class was dlopen()ed by means other than "
			    "through the plugin_loader interface. "
			    "This can happen if you build plugin libraries that contain more than just plugins "
			    "(i.e. normal code your app links against) -- that intrinsically will trigger a dlopen() "
			    "prior to main(). "
			    "You should isolate your plugins into their own library, otherwise it will not be "
			    "possible to shutdown the library!");

			obj = factory->create();
		}
		else {
			throw plugin::CreateClassException(
				"Could not create instance of type " + derived_class_name);
		}
	}

	logDebug(
	    "plugin_loader.impl: Created instance of type %s and object pointer = %p",
	    (typeid(obj).name()), reinterpret_cast<void *>(obj));

	return obj;
}


/**
 * @brief This function returns all the available plugin_loader in the plugin system that are derived from Base and within scope of the passed PluginLoader.
 * @param loader - The pointer to the PluginLoader whose scope we are within,
 * @return A vector of strings where each string is a plugin we can create
 */
template<typename Base>
std::vector<std::string> getAvailableClasses(PluginLoader * loader)
{
	std::unique_lock<std::recursive_mutex> lock(getPluginBaseToFactoryMapMapMutex());

	FactoryMap & factory_map = getFactoryMapForBaseClass<Base>();
	std::vector<std::string> classes;
	std::vector<std::string> classes_with_no_owner;

	for (auto & it : factory_map) {
		AbstractMetaObjectBase * factory = it.second;
		if (factory->isOwnedBy(loader)) {
			classes.push_back(it.first);
		}
		else if (factory->isOwnedBy(nullptr)) {
			classes_with_no_owner.push_back(it.first);
		}
	}

	// Added classes not associated with a class loader (Which can happen through
	// an unexpected dlopen() to the library)
	classes.insert(classes.end(), classes_with_no_owner.begin(), classes_with_no_owner.end());
	return classes;
}

/**
 * @brief This function returns the names of all libraries in use by a given class loader.
 * @param loader - The PluginLoader whose scope we are within
 * @return A vector of strings where each string is the path+name of each library that are within a PluginLoader's visible scope
 */
PLUGIN_LOADER_PUBLIC
std::vector<std::string> getAllLibrariesUsedByPluginLoader(const PluginLoader * loader);

/**
 * @brief Indicates if passed library loaded within scope of a PluginLoader. The library maybe loaded in memory, but to the class loader it may not be.
 * @param library_path - The name of the library we wish to check is open
 * @param loader - The pointer to the PluginLoader whose scope we are within
 * @return true if the library is loaded within loader's scope, else false
 */
PLUGIN_LOADER_PUBLIC
bool isLibraryLoaded(const std::string & library_path, PluginLoader * loader);

/**
 * @brief Indicates if passed library has been loaded by ANY PluginLoader
 * @param library_path - The name of the library we wish to check is open
 * @return true if the library is loaded in memory, otherwise false
 */
PLUGIN_LOADER_PUBLIC
bool isLibraryLoadedByAnybody(const std::string & library_path);

/**
 * @brief Loads a library into memory if it has not already been done so. Attempting to load an already loaded library has no effect.
 * @param library_path - The name of the library to open
 * @param loader - The pointer to the PluginLoader whose scope we are within
 */
PLUGIN_LOADER_PUBLIC
void loadLibrary(const std::string & library_path, PluginLoader* loader);

/**
 * @brief Unloads a library if it loaded in memory and cleans up its corresponding class factories. If it is not loaded, the function has no effect
 * @param library_path - The name of the library to open
 * @param loader - The pointer to the PluginLoader whose scope we are within
 */
PLUGIN_LOADER_PUBLIC
void unloadLibrary(const std::string & library_path, PluginLoader* loader);


////////////////////////////////////////////////////////////////////////// 
// inline 
// class ?????? ?????? ?????? ?????????? ???? ?? ?? x
// singletone ???????? ?????? ???????? ???????? ???? ???????? ???? ?? ????.
//////////////////////////////////////////////////////////////////////////
PLUGIN_LOADER_PUBLIC inline
std::recursive_mutex& getLoadedLibraryVectorMutex()
{
	static std::recursive_mutex m;
	return m;
}

PLUGIN_LOADER_PUBLIC inline
std::recursive_mutex& getPluginBaseToFactoryMapMapMutex()
{
	static std::recursive_mutex m;
	return m;
}
 
PLUGIN_LOADER_PUBLIC inline
BaseToFactoryMapMap& getGlobalPluginBaseToFactoryMapMap()
{
	static BaseToFactoryMapMap instance;
	return instance;
}

PLUGIN_LOADER_PUBLIC inline
MetaObjectVector& getMetaObjectGraveyard()
{
	static MetaObjectVector instance;
	return instance;
}

PLUGIN_LOADER_PUBLIC inline
LibraryVector& getLoadedLibraryVector()
{
	static LibraryVector instance;
	return instance;
}

PLUGIN_LOADER_PUBLIC inline
std::string& getCurrentlyLoadingLibraryNameReference()
{
	static std::string library_name;
	return library_name;
}

PLUGIN_LOADER_PUBLIC inline
bool& hasANonPurePluginLibraryBeenOpenedReference()
{
	static bool hasANonPurePluginLibraryBeenOpenedReference = false;
	return hasANonPurePluginLibraryBeenOpenedReference;
}

PLUGIN_LOADER_PUBLIC inline
PluginLoader*& getCurrentlyActivePluginLoaderReference()
{
	static PluginLoader* loader = nullptr;
	return loader;
}

} // namespace impl
} // namespace plugin

#endif // PLUGIN_IMPL_CORE_HPP_