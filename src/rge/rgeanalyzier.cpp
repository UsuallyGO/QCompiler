
#include <memory>
#include "rgeanalyzier.h"
#include "rgespecific.h"
#include "rge/rgeanalyzier_factory.h"

std::unique_ptr<RgeAnalyzier> CreateRgeAnalyzier(const std::string& analyzier_name) {
	RgeAnalyzierFactory* factory = RgeAnalyzierFactoryRegistry::GetFactory(analyzier_name);
	if (factory == nullptr) return nullptr;
	else return factory->CreateRgeAnalyzier();
}