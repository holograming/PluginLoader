#include <cassert>

#include "PluginLoaderCore.hpp"
#include "PluginLoader.hpp"


namespace plugin {
namespace impl {

//////////////////////////////////////////////////////////////////////////
// MetaObject search/insert/removal/query
//////////////////////////////////////////////////////////////////////////

MetaObjectVector allMetaObjects(const FactoryMap & factories)
{
	MetaObjectVector all_meta_objs;
	for (auto & it : factories) {
		all_meta_objs.push_back(it.second);
	}
	return all_meta_objs;
}

MetaObjectVector allMetaObjects()
{
	std::unique_lock<std::recursive_mutex> lock(getPluginBaseToFactoryMapMapMutex());

	MetaObjectVector all_meta_objs;
	BaseToFactoryMapMap & factory_map_map = getGlobalPluginBaseToFactoryMapMap();
	BaseToFactoryMapMap::iterator itr;

	for (auto & it : factory_map_map) {
		MetaObjectVector objs = allMetaObjects(it.second);
		all_meta_objs.insert(all_meta_objs.end(), objs.begin(), objs.end());
	}
	return all_meta_objs;
}

MetaObjectVector filterAllMetaObjectsOwnedBy(const MetaObjectVector & to_filter, const PluginLoader * owner)
{
	MetaObjectVector filtered_objs;
	for (auto & f : to_filter) {
		if (f->isOwnedBy(owner)) {
			filtered_objs.push_back(f);
		}
	}
	return filtered_objs;
}

MetaObjectVector filterAllMetaObjectsAssociatedWithLibrary(MetaObjectVector const& to_filter, std::string const& library_path)
{
	MetaObjectVector filtered_objs;
	for (auto f : to_filter) {
		if (f->getAssociatedLibraryPath() == library_path) {
			filtered_objs.push_back(f);
		}
	}
	return filtered_objs;
}

MetaObjectVector allMetaObjectsForPluginLoader(const PluginLoader * owner)
{
	return filterAllMetaObjectsOwnedBy(allMetaObjects(), owner);
}

MetaObjectVector allMetaObjectsForLibrary(const std::string & library_path){
	return filterAllMetaObjectsAssociatedWithLibrary(allMetaObjects(), library_path);
}

MetaObjectVector allMetaObjectsForLibraryOwnedBy(const std::string & library_path, const PluginLoader * owner) {
	return filterAllMetaObjectsOwnedBy(allMetaObjectsForLibrary(library_path), owner);
}

void insertMetaObjectIntoGraveyard(AbstractMetaObjectBase* meta_obj)
{
	logDebug(
	  "plugin_loader.impl: "
	  "Inserting MetaObject (class = %s, base_class = %s, ptr = %p) into graveyard",
	  meta_obj->className().c_str(), meta_obj->baseClassName().c_str(),
	  reinterpret_cast<void *>(meta_obj));
	getMetaObjectGraveyard().push_back(meta_obj);
}

void destroyMetaObjectsForLibrary(
	const std::string & library_path, FactoryMap & factories, const PluginLoader * loader)
{
	FactoryMap::iterator factory_itr = factories.begin();
	while (factory_itr != factories.end()) {
		AbstractMetaObjectBase * meta_obj = factory_itr->second;
		if (meta_obj->getAssociatedLibraryPath() == library_path && meta_obj->isOwnedBy(loader)) {
			meta_obj->removeOwningPluginLoader(loader);
			if (!meta_obj->isOwnedByAnybody()) {
				FactoryMap::iterator factory_itr_copy = factory_itr;
				factory_itr++;
				// TODO(mikaelarguedas) fix this when branching out for melodic
				// Note: map::erase does not return iterator like vector::erase does.
				// Hence the ugliness of this code and a need for copy. Should be fixed in next C++ revision
				factories.erase(factory_itr_copy);

				// Insert into graveyard
				// We remove the metaobject from its factory map, but we don't destroy it...instead it
				// saved to a "graveyard" to the side.
				// This is due to our static global variable initialization problem that causes factories
				// to not be registered when a library is closed and then reopened.
				// This is because it's truly not closed due to the use of global symbol binding i.e.
				// calling dlopen with RTLD_GLOBAL instead of RTLD_LOCAL.
				// We require using the former as the which is required to support RTTI
				insertMetaObjectIntoGraveyard(meta_obj);
			}
			else {
				++factory_itr;
			}
		}
		else {
			++factory_itr;
		}
	}
}

void destroyMetaObjectsForLibrary(const std::string & library_path, const PluginLoader * loader)
{
	std::unique_lock<std::recursive_mutex> lock(getPluginBaseToFactoryMapMapMutex());
	logDebug(
	  "plugin_loader.impl: "
	  "Removing MetaObjects associated with library %s and class loader %p from global "
	  "plugin-to-factorymap map.\n",
	  library_path.c_str(), reinterpret_cast<const void *>(loader));

	// We have to walk through all FactoryMaps to be sure
	BaseToFactoryMapMap& factory_map_map = getGlobalPluginBaseToFactoryMapMap();
	for (auto& it : factory_map_map) {
		destroyMetaObjectsForLibrary(library_path, it.second, loader);
	}
	logDebug("%s", "plugin_loader.impl: Metaobjects removed.");
}

bool areThereAnyExistingMetaObjectsForLibrary(const std::string & library_path) {
	return !allMetaObjectsForLibrary(library_path).empty();
}

// end of MetaObject search/insert/removal/query
// ------------------------------------------------------------------------------------------------------------------------- //


//////////////////////////////////////////////////////////////////////////
// Loaded Library Vector manipulation
//////////////////////////////////////////////////////////////////////////

LibraryVector::iterator findLoadedLibrary(std::string const& library_path)
{
	LibraryVector& open_libraries = getLoadedLibraryVector();
	for (auto it = open_libraries.begin(); it != open_libraries.end(); ++it) {
		if (it->first == library_path) {
			return it;
		}
	}
	return open_libraries.end();
}

// end of Loaded Library Vector manipulation
// ------------------------------------------------------------------------------------------------------------------------- //


//////////////////////////////////////////////////////////////////////////
// Global data area
//////////////////////////////////////////////////////////////////////////

PluginLoader* getCurrentlyActivePluginLoader() {
	return getCurrentlyActivePluginLoaderReference();
}

std::string getCurrentlyLoadingLibraryName() {
	return getCurrentlyLoadingLibraryNameReference();
}


FactoryMap& getFactoryMapForBaseClass(const std::string & typeid_base_class_name)
{
	BaseToFactoryMapMap & factoryMapMap = getGlobalPluginBaseToFactoryMapMap();
	std::string base_class_name = typeid_base_class_name;
	if (factoryMapMap.find(base_class_name) == factoryMapMap.end()) {
		factoryMapMap[base_class_name] = FactoryMap();
	}

	return factoryMapMap[base_class_name];
}


bool hasANonPurePluginLibraryBeenOpened() {
	return hasANonPurePluginLibraryBeenOpenedReference();
}

void hasANonPurePluginLibraryBeenOpened(const bool hasIt) {
	hasANonPurePluginLibraryBeenOpenedReference() = hasIt;
}


void setCurrentlyActivePluginLoader(PluginLoader* const loader)
{
	PluginLoader*& loader_ref = getCurrentlyActivePluginLoaderReference();
	loader_ref = loader;
}

void setCurrentlyLoadingLibraryName(const std::string& library_name)
{
	std::string& library_name_ref = getCurrentlyLoadingLibraryNameReference();
	library_name_ref = library_name;
}


// End of Global data area
// ------------------------------------------------------------------------------------------------------------------------- //

//////////////////////////////////////////////////////////////////////////
// Implementation of Remaining Core plugin impl Functions
//////////////////////////////////////////////////////////////////////////

std::vector<std::string> getAllLibrariesUsedByPluginLoader(const PluginLoader * loader)
{
	MetaObjectVector all_loader_meta_objs = allMetaObjectsForPluginLoader(loader);
	std::vector<std::string> all_libs;
	for (auto & meta_obj : all_loader_meta_objs) {
		std::string lib_path = meta_obj->getAssociatedLibraryPath();
		if (std::find(all_libs.begin(), all_libs.end(), lib_path) == all_libs.end()) {
			all_libs.push_back(lib_path);
		}
	}
	return all_libs;
}

bool isLibraryLoaded(const std::string & library_path, PluginLoader * loader)
{
	bool is_lib_loaded_by_anyone = isLibraryLoadedByAnybody(library_path);
	size_t num_meta_objs_for_lib = allMetaObjectsForLibrary(library_path).size();
	size_t num_meta_objs_for_lib_bound_to_loader =
		allMetaObjectsForLibraryOwnedBy(library_path, loader).size();
	bool are_meta_objs_bound_to_loader =
		(0 == num_meta_objs_for_lib) ? true : (
			num_meta_objs_for_lib_bound_to_loader <= num_meta_objs_for_lib);

	return is_lib_loaded_by_anyone && are_meta_objs_bound_to_loader;
}

bool isLibraryLoadedByAnybody(const std::string & library_path)
{
	std::unique_lock<std::recursive_mutex> lock(getLoadedLibraryVectorMutex());

	LibraryVector& open_libraries = getLoadedLibraryVector();
	LibraryVector::iterator itr = findLoadedLibrary(library_path);

	if (itr != open_libraries.end()) {
		assert(itr->second->isLoaded() == true);  // Ensure Osstem actually thinks the library is loaded
		return true;
	}
	else {
		return false;
	}
}

void addPluginLoaderOwnerForAllExistingMetaObjectsForLibrary(
	const std::string & library_path, PluginLoader * loader)
{
	MetaObjectVector all_meta_objs = allMetaObjectsForLibrary(library_path);
	for (auto & meta_obj : all_meta_objs) {
		logDebug(
		  "plugin_loader.impl: "
		  "Tagging existing MetaObject %p (base = %s, derived = %s) with "
		  "class loader %p (library path = %s).",
		  reinterpret_cast<void *>(meta_obj), meta_obj->baseClassName().c_str(),
		  meta_obj->className().c_str(),
		  reinterpret_cast<void *>(loader),
		  nullptr == loader ? loader->getLibraryPath().c_str() : "NULL");
		meta_obj->addOwningPluginLoader(loader);
	}
}

void revivePreviouslyCreateMetaobjectsFromGraveyard(std::string const& library_path, PluginLoader* loader)
{
	std::unique_lock<std::recursive_mutex> b2fmm_lock(getPluginBaseToFactoryMapMapMutex());
	MetaObjectVector & graveyard = getMetaObjectGraveyard();

	for (auto & obj : graveyard) {
		if (obj->getAssociatedLibraryPath() == library_path) {
			logDebug(
			  "plugin_loader.impl: "
			  "Resurrected factory metaobject from graveyard, class = %s, base_class = %s ptr = %p..."
			  "bound to PluginLoader %p (library path = %s)",
			  obj->className().c_str(), obj->baseClassName().c_str(), reinterpret_cast<void *>(obj),
			  reinterpret_cast<void *>(loader),
			  nullptr == loader ? loader->getLibraryPath().c_str() : "NULL");

			obj->addOwningPluginLoader(loader);
			assert(obj->typeidBaseClassName() != "UNSET");
			FactoryMap & factory = getFactoryMapForBaseClass(obj->typeidBaseClassName());
			factory[obj->className()] = obj;
		}
	}
}

void purgeGraveyardOfMetaobjects(
	const std::string & library_path, PluginLoader* loader, bool delete_objs)
{
	MetaObjectVector all_meta_objs = allMetaObjects();
	// Note: Lock must happen after call to allMetaObjects as that will lock
	std::unique_lock<std::recursive_mutex> b2fmm_lock(getPluginBaseToFactoryMapMapMutex());

	MetaObjectVector & graveyard = getMetaObjectGraveyard();
	MetaObjectVector::iterator itr = graveyard.begin();

	while (itr != graveyard.end()) {
		AbstractMetaObjectBase * obj = *itr;
		if (obj->getAssociatedLibraryPath() == library_path) {
			logDebug(
			  "plugin_loader.impl: "
			  "Purging factory metaobject from graveyard, class = %s, base_class = %s ptr = %p.."
			  ".bound to PluginLoader %p (library path = %s)",
			  obj->className().c_str(), obj->baseClassName().c_str(), reinterpret_cast<void *>(obj),
			  reinterpret_cast<void *>(loader),
			  nullptr == loader ? loader->getLibraryPath().c_str() : "NULL");

			bool is_address_in_graveyard_same_as_global_factory_map =
				std::find(all_meta_objs.begin(), all_meta_objs.end(), *itr) != all_meta_objs.end();
			itr = graveyard.erase(itr);
			if (delete_objs) {
				if (is_address_in_graveyard_same_as_global_factory_map) {
					logDebug("%s",
					    "plugin_loader.impl: "
					    "Newly created metaobject factory in global factory map map has same address as "
					    "one in graveyard -- metaobject has been purged from graveyard but not deleted.");
				}
				else {
					assert(hasANonPurePluginLibraryBeenOpened() == false);
					logDebug(
					    "plugin_loader.impl: "
					    "Also destroying metaobject %p (class = %s, base_class = %s, library_path = %s) "
					    "in addition to purging it from graveyard.",
					    reinterpret_cast<void *>(obj), obj->className().c_str(), obj->baseClassName().c_str(),
					    obj->getAssociatedLibraryPath().c_str());
#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif
					delete (obj);  // Note: This is the only place where metaobjects can be destroyed
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif
				}
			}
		}
		else {
			itr++;
		}
	}
}

void loadLibrary(const std::string & library_path, PluginLoader* loader)
{
	static std::recursive_mutex loader_mutex;
	logDebug(
	  "plugin_loader.impl: "
	  "Attempting to load library %s on behalf of PluginLoader handle %p...\n",
	  library_path.c_str(), reinterpret_cast<void *>(loader));
	std::unique_lock<std::recursive_mutex> loader_lock(loader_mutex);


	// If it's already open, just update existing metaobjects to have an additional owner.
	if (isLibraryLoadedByAnybody(library_path)) {
		std::unique_lock<std::recursive_mutex> lock(getPluginBaseToFactoryMapMapMutex());
		logDebug("%s",
			"class_loader.impl: "
			"Library already in memory, but binding existing MetaObjects to loader if necesesary.\n");
		addPluginLoaderOwnerForAllExistingMetaObjectsForLibrary(library_path, loader);
		return;
	}



	SharedLibrary* library_handle = nullptr;
	{
		try {
			setCurrentlyActivePluginLoader(loader);
			setCurrentlyLoadingLibraryName(library_path);
			library_handle = new SharedLibrary(library_path);
		}
		catch (const plugin::LibraryLoadException& e)
		{
			setCurrentlyLoadingLibraryName("");
			setCurrentlyActivePluginLoader(nullptr);
			throw e;
		}

		setCurrentlyLoadingLibraryName("");
		setCurrentlyActivePluginLoader(nullptr);
	}

	assert(library_handle != nullptr);

	logDebug(
	"plugin_loader.impl: "
	"Successfully loaded library %s into memory (SharedLibrary handle = %p).",
	library_path.c_str(), reinterpret_cast<void *>(library_handle));

	// Graveyard scenario
	size_t num_lib_objs = allMetaObjectsForLibrary(library_path).size();
	if (0 == num_lib_objs) {
		logDebug(
		  "plugin_loader.impl: "
		  "Though the library %s was just loaded, it seems no factory metaobjects were registered. "
		  "Checking factory graveyard for previously loaded metaobjects...",
		  library_path.c_str());
		revivePreviouslyCreateMetaobjectsFromGraveyard(library_path, loader);
		// Note: The 'false' indicates we don't want to invoke delete on the metaobject
		purgeGraveyardOfMetaobjects(library_path, loader, false);
	}
	else {
		logDebug(
		  "plugin_loader.impl: "
		  "Library %s generated new factory metaobjects on load. "
		  "Destroying graveyarded objects from previous loads...",
		  library_path.c_str());
		purgeGraveyardOfMetaobjects(library_path, loader, true);
	}

	// Insert library into global loaded library vector
	std::unique_lock<std::recursive_mutex> llv_lock(getLoadedLibraryVectorMutex());
	LibraryVector& open_libraries = getLoadedLibraryVector();
	// Note: SharedLibrary automatically calls load() when library passed to constructor
	open_libraries.push_back(LibraryPair(library_path, library_handle));

}
	
void unloadLibrary(std::string const& library_path, PluginLoader* loader)
{
	if (hasANonPurePluginLibraryBeenOpened()) {
		logDebug(
		"plugin_loader.impl: "
		"Cannot unload %s or ANY other library as a non-pure plugin library was opened. "
		"As plugin_loader has no idea which libraries class factories were exported from, "
		"it can safely close any library without potentially unlinking symbols that are still "
		"actively being used. "
		"You must refactor your plugin libraries to be made exclusively of plugins "
		"in order for this error to stop happening.",
		library_path.c_str());
	}
	else {
		logDebug(
			"plugin_loader.impl: "
			"Unloading library %s on behalf of PluginLoader %p...",
			library_path.c_str(), reinterpret_cast<void *>(loader));
		std::unique_lock<std::recursive_mutex> lock(getLoadedLibraryVectorMutex());
		LibraryVector& open_libraries = getLoadedLibraryVector();
		LibraryVector::iterator itr = findLoadedLibrary(library_path);
		if (itr != open_libraries.end()) {
			SharedLibrary * library = itr->second;
			std::string library_path = itr->first;
			try {
				destroyMetaObjectsForLibrary(library_path, loader);

				// Remove from loaded library list as well if no more factories associated with said library
				if (!areThereAnyExistingMetaObjectsForLibrary(library_path)) {
					 logDebug(
					   "plugin_loader.impl: "
					   "There are no more MetaObjects left for %s so unloading library and "
					   "removing from loaded library vector.\n",
					   library_path.c_str());

					library->unload();
					assert(library->isLoaded() == false);
					delete (library);
					library = nullptr;
					itr = open_libraries.erase(itr);
				}
				else {
					logDebug(
					  "plugin_loader.impl: "
					  "MetaObjects still remain in memory meaning other PluginLoaders are still using library"
					  ", keeping library %s open.",
					  library_path.c_str());
				}
				return;
			}
			catch (const std::runtime_error & e) {
				delete (library);
				library = nullptr;
				throw plugin::LibraryUnloadException(
					"Could not unload library (exception = " + std::string(e.what()) + ")");
			}
		}
		throw plugin::LibraryUnloadException(
			"Attempt to unload library that plugin_loader is unaware of.");
	}
}

// End of Implementation of Remaining Core plugin impl Functions
// ------------------------------------------------------------------------------------------------------------------------- //

//////////////////////////////////////////////////////////////////////////
// Debugging
//////////////////////////////////////////////////////////////////////////

void printDebugInfoToScreen()
{
	printf("*******************************************************************************\n");
	printf("*****                 plugin_loader impl DEBUG INFORMATION                 *****\n");
	printf("*******************************************************************************\n");

	printf("OPEN LIBRARIES IN MEMORY:\n");
	printf("--------------------------------------------------------------------------------\n");
	std::unique_lock<std::recursive_mutex> lock(getLoadedLibraryVectorMutex());
	LibraryVector libs = getLoadedLibraryVector();
	for (size_t c = 0; c < libs.size(); c++) {
		printf(
			"Open library %zu = %s (Poco SharedLibrary handle = %p)\n",
			c, (libs.at(c)).first.c_str(), reinterpret_cast<void *>((libs.at(c)).second));
	}

	printf("METAOBJECTS (i.e. FACTORIES) IN MEMORY:\n");
	printf("--------------------------------------------------------------------------------\n");
	MetaObjectVector meta_objs = allMetaObjects();
	for (size_t c = 0; c < meta_objs.size(); c++) {
		AbstractMetaObjectBase * obj = meta_objs.at(c);
		printf("Metaobject %zu (ptr = %p):\n TypeId = %s\n Associated Library = %s\n",
			c,
			reinterpret_cast<void *>(obj),
			(typeid(*obj).name()),
			obj->getAssociatedLibraryPath().c_str());

		PluginLoaderVector loaders = obj->getAssociatedPluginLoaders();
		for (size_t i = 0; i < loaders.size(); i++) {
			printf(" Associated Loader %zu = %p\n", i, reinterpret_cast<void *>(loaders.at(i)));
		}
		printf("--------------------------------------------------------------------------------\n");
	}

	printf("********************************** END DEBUG **********************************\n");
	printf("*******************************************************************************\n\n");
}

// End of Debugging
// ------------------------------------------------------------------------------------------------------------------------- //

} // namespace impl
} // namespace plugin