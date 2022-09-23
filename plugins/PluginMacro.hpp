
#ifndef PLUGIN_MACRO_HPP_
#define PLUGIN_MACRO_HPP_

#include <string>
#include "PluginLoaderCore.hpp"

#define PLUGIN_LOADER_REGISTER_CLASS_INTERNAL(Derived, Base, UniqueID) \
  namespace \
  { \
  struct ProxyExec ## UniqueID \
  { \
    typedef  Derived _derived; \
    typedef  Base _base; \
    ProxyExec ## UniqueID() \
    { \
      plugin::impl::registerPlugin<_derived, _base>(#Derived, #Base); \
    } \
  }; \
  static ProxyExec ## UniqueID g_register_plugin_ ## UniqueID; \
  }  // namespace

#define PLUGIN_LOADER_REGISTER_CLASS_INTERNAL_HOP1(Derived, Base, UniqueID) \
  PLUGIN_LOADER_REGISTER_CLASS_INTERNAL(Derived, Base, UniqueID)

#define PLUGIN_LOADER_REGISTER_CLASS(Derived, Base) \
  PLUGIN_LOADER_REGISTER_CLASS_INTERNAL_HOP1(Derived, Base, __COUNTER__)


#endif // PLUGIN_MACRO_HPP_