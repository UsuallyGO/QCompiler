
#pragma once

#include <memory>
#include <map>
#include <string>

#define FACTORY_REGISTRY_DEFINE(Product) \
class Product##Factory\
{\
public:\
	virtual std::unique_ptr<Product> Create##Product() = 0; \
};\
\
class Product##FactoryRegistry{ \
public:\
	using FactoryRegistry = std::map<std::string, std::unique_ptr<Product##Factory>>;\
	\
	static FactoryRegistry& GetFactoryRegistry(){ \
		static FactoryRegistry registry; \
		return registry; \
	}\
	\
	static Product##Factory* GetFactory(const std::string& factory_name){\
		const FactoryRegistry& registry = GetFactoryRegistry();\
		auto iter = registry.find(factory_name); \
	if (iter == registry.end()) return nullptr; \
	else return iter->second.get(); \
}\
\
	static bool FactoryRegister(const std::string& factory_name, std::unique_ptr<Product##Factory> factory) { \
		\
		FactoryRegistry& registry = GetFactoryRegistry(); \
		auto factory_ptr = GetFactory(factory_name); \
		if(factory_ptr != nullptr) return false; \
		\
		auto pair = registry.insert(std::make_pair(factory_name, std::move(factory))); \
		return pair.second; \
	}\
};\
\
template<typename FactoryType>\
class Product##FactoryRegistrar{ \
public: \
	explicit Product##FactoryRegistrar(const std::string& name){ \
		Product##FactoryRegistry::FactoryRegister(name, std::unique_ptr<Product##Factory>(new FactoryType)); \
	} \
}
		
#define FACTORY_REGISTRAR_DEFINE(name, Product, Factory) static Product##FactoryRegistrar<Factory>\
									__##Factory##__COUNTER__##__obj(name)


