
#include <cassert>
#include "syntax_specific.h"
#include "factory_template.h"
#include "syntax/grammar_generator_factory.h"
#include "utility/file_reader.h"
#include "utility/file_reader_factory.h"
#include "utility/q_file_reader.h"
#include "utility/utility_internal.h"
#include "factory_template.h"

class QGrammarGenerator final : public GrammarGenerator{
public:	
	QGrammarGenerator() : fileReader_(nullptr){
		FileReaderFactory* factory = FileReaderFactoryRegistry::GetFactory("QFileReader");
		FileReader* reader = factory->CreateFileReader().release();
		QFileReader* qreader = dynamic_cast<QFileReader*>(reader);
		fileReader_.reset(qreader);
	}
	virtual bool OpenFile(const std::string& file) override{
		return fileReader_->OpenFile(file);
	}
	ContextFreeGrammar GrammarGenerate() override;

private:
	void _parseSentence(std::string sentence, std::vector<std::string>& contents,
		std::vector<bool>& isTerm);
	std::unique_ptr<QFileReader> fileReader_;
};

/* GrammarGenerate() function reads grammar definition file and generate ContextFreeGrammar object.
 * Almost cannot find out any errors in *.syn file, but can skip duplicate spaces in files.
 * Nonterminals should be surrounded by angle brackets('<>'), continuous terminals should be separated
 * by blank.
 * For example: <A>-><B>c d|<C>num
 * In this production, terminal 'c' follows nonterminal 'B', then terminal 'd' follows terminal 'c'. 
 * 'c' and 'd' are sperated by space, so they two different terminals. But for 'num', this is terminal
 * that called "num".
 * Continuous productions for same nonterminal can omit the left part of the production. For example, 
 * <A>-><F>
 *      <T>
 * The left part of the second production has been omitted. It just as the same as <A>-><T>
 *
 * The nonterminal of the first production will be seen as the start symbol, but we will add an other
 * internal start symbol which noted as "_start_". So, you cannot use "_start_" in your *.syn as 
 * terminal or nonterminal.
 */
ContextFreeGrammar QGrammarGenerator::GrammarGenerate(){
	ContextFreeGrammar grammar;
	std::string left;
	std::string start_symbol;
	std::string str_epsilon = ContextFreeGrammar::Epsilon;
	//check file here...

	fileReader_->ReadToBuffer();
	while(!fileReader_->IsFileEnd()){
		std::string str_line = fileReader_->ReadLine();
		if (str_line.empty()) continue;
		TrimmedPrefix(str_line, SyntaxSemantics::BLANK, SyntaxSemantics::BLANK);
		auto pos = str_line.find(SyntaxSemantics::PRODUCE);
		if(pos != std::string::npos) {//this is a part of the last produce
			auto pos2 = str_line.find(SyntaxSemantics::ITEM_END);
			left = str_line.substr(1, pos2 - 1); //skip '<' and '>'
			str_line = str_line.substr(pos + SyntaxSemantics::PRODUCE.length());
			if (start_symbol.empty()) start_symbol = left;
		}

		TrimmedPostfix(str_line, SyntaxSemantics::BLANK, SyntaxSemantics::BLANK);
		const char* ptr = str_line.c_str();
		while(*ptr != '\0'){
			size_t beg, len;
			
			beg = ptr - str_line.c_str();
			pos = str_line.find_first_of(SyntaxSemantics::PARALLEL_CONNECT, beg);

			if(pos == std::string::npos) len = str_line.length() - beg;
			else len = pos - beg, ptr++;//ptr++ to skip the PARALLEL_CONNECT

			std::string sentence = str_line.substr(beg, len);
			ptr += len;

			std::vector<std::string> contents;
			std::vector<bool> isTerm;
			_parseSentence(sentence, contents, isTerm);

			//contents cannot be empty here
			for (int i = 0; i < contents.size(); i++) {
				if(contents[i] == str_epsilon) continue; //epsilon is neither terminal nor nonterminal
				else if (isTerm[i]) grammar.terminals.insert(contents[i]);
				else grammar.nonTerminals.insert(contents[i]);
			}
			grammar.productions.insert(std::make_pair(left, contents));
			if(contents.size() == 1 && contents[0] == str_epsilon) grammar.nullable[left] = true;
		}
		grammar.nonTerminals.insert(left);
	}

	if(grammar.addStartSymbol){
		std::vector<std::string> contents(1, start_symbol);
		grammar.productions.insert(std::make_pair(grammar.startSymbol, contents));
		grammar.nonTerminals.insert(grammar.startSymbol);
	}
	else grammar.startSymbol = start_symbol;

	for(const auto& t : grammar.terminals)
		grammar.nullable[t] = false;
	for(const auto& nt : grammar.nonTerminals)
		if(grammar.nullable.find(nt) == grammar.nullable.end()) grammar.nullable[nt] = false;
	grammar.nullable[CharToString(SyntaxSemantics::EPSILON)] = true;
	
	return grammar;
}

void QGrammarGenerator::_parseSentence(std::string sentence, std::vector<std::string>& contents, 
						std::vector<bool>& isTerm){
	const char* ptr = sentence.c_str();

	while(*ptr != '\0'){ //move char* is faster than use string itself
		if(*ptr == SyntaxSemantics::BLANK) { ptr++; continue; }

		std::string item;
		bool b_term = false;
		if(*ptr == SyntaxSemantics::ITEM_BEGIN){ //nonterminals are in '<>'
			auto pos = sentence.find_first_of(SyntaxSemantics::ITEM_END, ptr - sentence.c_str());
			assert(pos != std::string::npos);
			size_t beg_pos = ptr - sentence.c_str() + 1;
			item = sentence.substr(beg_pos, pos - beg_pos);
			b_term = false;
			ptr = sentence.c_str() + pos + 1;
		}
		else {
			while(*ptr && *ptr != SyntaxSemantics::ITEM_BEGIN &&
				*ptr != SyntaxSemantics::BLANK) //continuous terminals are identified by space
				item += *ptr++;
			b_term = true; //*ptr is terminal
		}
	
		contents.push_back(item);
		isTerm.push_back(b_term);
	}
}

class QGrammarGeneratorFactory : public GrammarGeneratorFactory{
	std::unique_ptr<GrammarGenerator> CreateGrammarGenerator(){
		std::unique_ptr<GrammarGenerator> ptr(new QGrammarGenerator);
		return ptr;
	}
};

FACTORY_REGISTRAR_DEFINE("QGrammarGeneratorFactory", GrammarGenerator, QGrammarGeneratorFactory);
