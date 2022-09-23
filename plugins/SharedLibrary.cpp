#include "SharedLibrary.hpp"
#include <Windows.h>

#include "Exceptions.hpp"
#include "PluginLoaderCore.hpp"

namespace plugin {

void SharedLibrary::load(const std::string& path)
{
	std::unique_lock<std::mutex> lock(_mutex);

	if (_handle) {
		throw plugin::LibraryLoadException("Library already loaded: " + path);
	}
	_handle = ::LoadLibraryA(path.c_str());

	if (!_handle)
	{
		auto const error_id = GetLastError();

		const char* err = std::system_category().message(error_id).c_str();
		throw plugin::LibraryLoadException(
			"Could not load library: " + (err ? std::string(err) : path));
	}
	_path = path;
}


void SharedLibrary::unload()
{
	std::unique_lock<std::mutex> lock(_mutex);

	if (_handle)
	{
		::FreeLibrary((HMODULE)_handle);
		_handle = 0;
	}
}


bool SharedLibrary::isLoaded() const
{
	return _handle != 0;
}


void* SharedLibrary::findSymbol(const std::string& name)
{
	std::unique_lock<std::mutex> lock(_mutex);
	if (_handle) {
		return ::GetProcAddress((HMODULE)_handle, name.c_str());
	}
	return nullptr;
}


const std::string& SharedLibrary::getPath() const
{
	return _path;
}

std::string SharedLibrary::prefix()
{
	return "lib";
}

std::string SharedLibrary::suffix()
{
#if defined(_DEBUG)
	return "d.dll";
#else
	return ".dll";
#endif

}

} // namespace plugin