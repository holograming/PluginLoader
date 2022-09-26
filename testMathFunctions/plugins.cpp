#include <plugins/PluginLoader.hpp>
#include "base.hpp"

class PlusOperation : public Base
{
public:
  virtual double mathFunctions(const double param1, const double param2) override { return param1 + param2; }
};

class SubstractOperation : public Base
{
public:
  virtual double mathFunctions(const double param1, const double param2) override { return param1 - param2; }
};

class MultiplyOperation : public Base
{
public:
  virtual double mathFunctions(const double param1, const double param2) override { return param1 * param2; }
};

class DivideOperation : public Base
{
public:
  virtual double mathFunctions(const double param1, const double param2) override { 
		if(param2 == 0.) return 0.; 
		return param1 / param2; 

	}
};

PLUGIN_LOADER_REGISTER_CLASS(PlusOperation, Base)
PLUGIN_LOADER_REGISTER_CLASS(SubstractOperation, Base)
PLUGIN_LOADER_REGISTER_CLASS(MultiplyOperation, Base)
PLUGIN_LOADER_REGISTER_CLASS(DivideOperation, Base)