
#include <iostream>
#include <fstream>
#include <memory>
#include <stack>
#include <algorithm>
#include "rgespecific.h"
#include "error.h"
#include "rgeanalyzier.h"
#include "rgeanalyzier_factory.h"
#include "idstatebuilder.h"
#include "idstatebuilder_factory.h"
#include "q_idstatebuilder.h"
#include "factory_template.h"
#include "utility/file_reader.h"
#include "utility/file_reader_factory.h"
#include "utility/q_file_reader.h"
#include "utility/utility_internal.h"

class QRgeAnalyzier final : public RgeAnalyzier {
public:
	QRgeAnalyzier() : fileReader_(nullptr), domain_(new RGEDomainSpecific), idbuilder_(nullptr){
		FileReaderFactory *filereader_factory = FileReaderFactoryRegistry::GetFactory("QFileReader");
		FileReader *reader = filereader_factory->CreateFileReader().release();
		QFileReader *qreader = dynamic_cast<QFileReader*>(reader);
		fileReader_.reset(qreader);
		
		IDStateBuilderFactory *factory = IDStateBuilderFactoryRegistry::GetFactory("QIDStateBuilderFactory");
		idbuilder_.reset(factory->CreateIDStateBuilder().release());
	}

	bool OpenFile(const std::string& filepath);
	RGEDomainSpecific* RgeAnalyse();

	//Should be removed ?
	void show_file() { 	while (!fileReader_->IsFileEnd())	std::cout << fileReader_->NextChar(); }

private:
	/* There are also comments in regular expression file, this has nothing to do with the
	 * comments in user domain specific language.
	 * Single line comment: begins with '#'
	 * Multi-line comment: begins with '!'
	 */
	void _consumeComment();

	void _consumeBlank() { fileReader_->NextChar(); }
	void _consumeEnter() { fileReader_->NextChar(); }

	bool _domainExtract(RGEDefinition_Type type);
	bool _checkID(const std::string& substr);

	bool _extractContents(std::string& substr, RGEDefinition_Type type);
	bool _setDomainContent(std::string& tmp, RGEDefinition_Type type);

	void _errorSkip(char c) {
		GENERATE_ERRMSG_PUSH(ILLEGAL_IDENTITY, fileReader_);
		while (!fileReader_->IsFileEnd() && fileReader_->NextChar() != c);
	}

	std::unique_ptr<QFileReader> fileReader_;
	std::unique_ptr<RGEDomainSpecific> domain_;
	std::unique_ptr<IDStateBuilder> idbuilder_;
};

bool QRgeAnalyzier::OpenFile(const std::string& filepath){
	return fileReader_->OpenFile(filepath);
}

RGEDomainSpecific* QRgeAnalyzier::RgeAnalyse() {
	fileReader_->ReadToBuffer();

	while (!fileReader_->IsFileEnd())
	{
		if (fileReader_->CurrentChar() == RgeularSemantics::BLANK) {
			_consumeBlank();
		}
		else if (fileReader_->CurrentChar() == RgeularSemantics::ENTER || 
				 fileReader_->CurrentChar() == RgeularSemantics::NEWLINE) {
			_consumeEnter();
		}
		else if (fileReader_->CurrentChar() == RgeularSemantics::LINE_COM
			|| fileReader_->CurrentChar() == RgeularSemantics::MUL_COM_BEGIN) {
			_consumeComment();
		}
		else if (fileReader_->CurrentChar() == RgeularSemantics::KEYWORDS[0]) {
			if (!_domainExtract(RGEDefinition_Type::KEYWORDS))
				std::cout << "Error in keywords" << std::endl;
			else
				std::cout << "Keywords OK" << std::endl;
		}
		else if (fileReader_->CurrentChar() == RgeularSemantics::SYMBOLS[0]) {
			if (!_domainExtract(RGEDefinition_Type::SYMBOLS))
				std::cout << "Error in symbols" << std::endl;
			else
				std::cout << "symbols OK" << std::endl;
		}
		else if (fileReader_->CurrentChar() == RgeularSemantics::OPERATORS[0]) {
			if (!_domainExtract(RGEDefinition_Type::OPERATORS))
				std::cout << "Error in operators" << std::endl;
			else
				std::cout << "operators OK" << std::endl;
		}
		else if (fileReader_->CurrentChar() == RgeularSemantics::DATATYPES[0]) {
			if (!_domainExtract(RGEDefinition_Type::DATATYPE))
				std::cout << "Error in datatypes" << std::endl;
			else
				std::cout << "datatypes OK" << std::endl;
		}
		else if (fileReader_->CurrentChar() == RgeularSemantics::ID[0]) {
			if (!_domainExtract(RGEDefinition_Type::ID))
				std::cout << "Error in ID " << std::endl;
			else
				std::cout << "ID OK" << std::endl;
		}
		else //error skip
			std::cout << fileReader_->NextChar();
	}
	return domain_.get();
}

void QRgeAnalyzier::_consumeComment() {
	bool done = false;
	char com = fileReader_->NextChar(); //no matter what kind of comment, we can skip the comment character
	do {
		if (fileReader_->IsFileEnd()) {
			//Dealing multiple comment missing '!' but reaches the end
			if (com == RgeularSemantics::MUL_COM_BEGIN) GENERATE_ERRMSG_PUSH(INCOMPLETE_COMMENT, fileReader_);
			done = true;
		}
		else if (fileReader_->CurrentChar() == RgeularSemantics::ENTER ||
			     fileReader_->CurrentChar() == RgeularSemantics::NEWLINE) {
			if (com == RgeularSemantics::LINE_COM) done = true;//do not eat this '\n', because it does not belong to the comment
			else fileReader_->NextChar();
		}
		else {
			if (fileReader_->CurrentChar() == RgeularSemantics::MUL_COM_END
				&& com == RgeularSemantics::MUL_COM_BEGIN) {
				done = true; //eat tail '!', else: '!' appears in line comment
			}
			fileReader_->NextChar();
		}
	} while (!done);
}

bool QRgeAnalyzier::_domainExtract(RGEDefinition_Type type) {
	//Check the definition type first, for example, "keywords" or "symbols"
	std::string substr = fileReader_->TrimmedSubstrBeforeChar(RgeularSemantics::ASSIGN_OPERATOR);
	if (substr != RGEDefinition_Type_String(type)) {
		_errorSkip(RgeularSemantics::ENTER);
		return false;
	}

	//assign operator should closely follow the definition type
	substr = fileReader_->TrimmedSubstrBeforeChar(RgeularSemantics::DEFINITION_BEGIN);
	if (substr != std::string(1, RgeularSemantics::ASSIGN_OPERATOR)) {
		_errorSkip(RgeularSemantics::ENTER);
		return false;
	}
	//donot need to check _fileEnd() here, because there is at least one char of RegularSemantics::DEFINITION_BEGIN
	fileReader_->NextChar();

	substr = fileReader_->TrimmedSubstrBeforeChar(RgeularSemantics::DEFINITION_END);
	if (fileReader_->IsFileEnd()) { //reach the file end and still cannot find DEFINITION_END
		GENERATE_ERRMSG_PUSH(INCOMPLETE_DEFINITION, fileReader_);
		return false;
	}
	else {
		fileReader_->NextChar(); //skip DEFINITION_END
		if (type == RGEDefinition_Type::ID) {
			if (_checkID(substr)) {
				_setDomainContent(substr, type);
				idbuilder_->BuildIDState(substr);
				idbuilder_->GenerateGraphvz("graph.gv");
				return true;
			}
			else {
				GENERATE_ERRMSG_PUSH(ILLEGAL_ID_DEFINITION, fileReader_);
				return false;
			}
		}
		else return _extractContents(substr, type);
	}
}

bool QRgeAnalyzier::_extractContents(std::string& substr, RGEDefinition_Type type)
{
	//split the substr with RegularSemantics::DEFINITION_INTERVAL
	bool done = false;
	int plus_extra = 1;
	do {
		size_t index = substr.find_first_of(RgeularSemantics::DEFINITION_INTERVAL);
		if (index == std::string::npos) //no DEFINITION_INTERVAL, this is the last content
			index = substr.length(), plus_extra = 0, done = true;
		std::string tmp = substr.substr(0, index);
		substr = substr.substr(index + plus_extra); //plus 1 to skip the interval
		TrimmedPrefix(tmp);
		TrimmedPostfix(tmp);
		if (CharExistInString(tmp, RgeularSemantics::BLANK) ||
            CharExistInString(tmp, RgeularSemantics::ENTER) ||
            CharExistInString(tmp, RgeularSemantics::NEWLINE)) {
			GENERATE_ERRMSG_PUSH(ILLEGAL_DEFINITION, fileReader_);
			return false;
		}
		if (!_setDomainContent(tmp, type)) {
			GENERATE_ERRMSG_PUSH(MULTIPLE_DEFINITION, fileReader_);
			return false;
		}
	} while (!done);
	return true;
}

bool QRgeAnalyzier::_setDomainContent(std::string& tmp, RGEDefinition_Type type)
{
	bool res = false;
	switch (type) {
	case RGEDefinition_Type::KEYWORDS:
		res = domain_->KeywordsInsert(tmp);
		break;
	case RGEDefinition_Type::OPERATORS:
		res = domain_->OperatorsInsert(tmp);
		break;
	case RGEDefinition_Type::SYMBOLS:
		res = domain_->SymbolsInsert(tmp);
		break;
	case RGEDefinition_Type::DATATYPE:
		res = domain_->DatatypesInsert(tmp);
		break;
	case RGEDefinition_Type::ID:
		domain_->IDSet(tmp);
		break;
	}
	return res;
}

bool QRgeAnalyzier::_checkID(const std::string& substr) {
	// do more here...
	return true;
}

class QRgeAnalyzierFactory final : public RgeAnalyzierFactory {
public:
	std::unique_ptr<RgeAnalyzier> CreateRgeAnalyzier() override {
		std::unique_ptr<RgeAnalyzier> analyzier(new QRgeAnalyzier);
		return analyzier;
	}
};

FACTORY_REGISTRAR_DEFINE("QRgeAnalyzier", RgeAnalyzier, QRgeAnalyzierFactory);