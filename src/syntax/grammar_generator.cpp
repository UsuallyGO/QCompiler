
#include <memory>
#include "syntax_specific.h"
#include "syntax/grammar_generator_factory.h"

std::unique_ptr<GrammarGenerator> CreateGrammarGenerator(const std::string& name){
	GrammarGeneratorFactory* factory = GrammarGeneratorFactoryRegistry::GetFactory(name);
	if (factory == nullptr) return nullptr;
	else return factory->CreateGrammarGenerator();
}

