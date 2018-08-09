
#include <memory>
#include <vector>
#include "syntax_specific.h"
#include "factory_template.h"
#include "syntax/sentence_reader_factory.h"
#include "utility/file_reader.h"
#include "utility/file_reader_factory.h"
#include "utility/q_file_reader.h"
#include "utility/utility_internal.h"

class QSentenceReader : public SentenceReader{
public:
	QSentenceReader() : fileReader_(nullptr){
		FileReaderFactory* factory = FileReaderFactoryRegistry::GetFactory("QFileReader");
		FileReader* reader = factory->CreateFileReader().release();
		QFileReader* qreader = dynamic_cast<QFileReader*>(reader);
		fileReader_.reset(qreader);
	}
	
	std::vector<std::string> ReadFile(const std::string& file) override;

private:
	std::unique_ptr<QFileReader> fileReader_;
};

std::vector<std::string> QSentenceReader::ReadFile(const std::string& file){
	std::vector<std::string> vec;
	
	bool b = fileReader_->OpenFile(file);
	if(!b) return vec; //generate error message here	

	std::string sentence;

	do {
		sentence = fileReader_->ReadLine();
		TrimmedPrefix(sentence);
		TrimmedPostfix(sentence);
	} while (sentence.empty());

	const char* p = sentence.c_str();
	while(*p){
		const char* cur = p;
		
		while(*cur && *cur != UTILITY_BLANK)
			cur++;

		vec.emplace_back(p, cur - p);

		while(*cur && *cur == UTILITY_BLANK)
			cur++;
		
		p = cur;
	}
	return vec;
}

class QSentenceReaderFactory : public SentenceReaderFactory{
	std::unique_ptr<SentenceReader> CreateSentenceReader(){
		std::unique_ptr<SentenceReader> reader(new QSentenceReader);
		return reader;
	}
};

FACTORY_REGISTRAR_DEFINE("QSentenceReader", SentenceReader, QSentenceReaderFactory);

