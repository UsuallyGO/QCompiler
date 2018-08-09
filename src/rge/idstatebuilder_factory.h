
#pragma once

#include "idstatebuilder.h"
#include "factory_template.h"

FACTORY_REGISTRY_DEFINE(IDStateBuilder);

/*
class IDStateBuilderFactory
{
public:
	virtual std::unique_ptr<IDStateBuilder> CreateIDStateBuilder() = 0;
};

class IDStateBuilderFactoryRegistry{ 
public:
	using FactoryRegistry = std::map<std::string, std::unique_ptr<IDStateBuilderFactory>>;
	
	static FactoryRegistry& GetFactoryRegistry(){ 
		static FactoryRegistry registry; 
		return registry; 
	}
	
	static IDStateBuilderFactory* GetFactory(const std::string& factory_name){
		const FactoryRegistry& registry = GetFactoryRegistry();
		auto iter = registry.find(factory_name); 
	if (iter == registry.end()) return nullptr; 
	else return iter->second.get(); 
	}

	static bool FactoryRegister(const std::string& factory_name, std::unique_ptr<IDStateBuilderFactory> factory) { 
		
		FactoryRegistry& registry = GetFactoryRegistry(); 
		auto factory_ptr = GetFactory(factory_name); 
		if(factory_ptr != nullptr) return false; 
		
		auto pair = registry.insert(std::make_pair(factory_name, std::move(factory))); 
		return pair.second; 
	}
};

template<typename FactoryType>
class IDStateBuilderFactoryRegistrar {
public:
	explicit IDStateBuilderFactoryRegistrar(const std::string& name) {
		IDStateBuilderFactoryRegistry::FactoryRegister(name, std::unique_ptr<IDStateBuilderFactory>(new FactoryType));
	} 
};
*/