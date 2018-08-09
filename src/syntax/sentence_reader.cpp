
#include <memory>
#include "syntax_specific.h"
#include "syntax/sentence_reader_factory.h"

std::unique_ptr<SentenceReader> CreateSentenceReader(const std::string& name){
	SentenceReaderFactory* factory = SentenceReaderFactoryRegistry::GetFactory(name);
	if (factory == nullptr) return nullptr;
	else return factory->CreateSentenceReader();
}

