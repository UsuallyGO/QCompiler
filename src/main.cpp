#if 0
#include <iostream>
#include <memory>
#include <algorithm>
#include "rgeanalyzier.h"
#include "error.h"

using namespace std;

int main()
{
	unique_ptr<RgeAnalyzier> analyzier(std::move(CreateRgeAnalyzier("QRgeAnalyzier")));
	if (!analyzier || !analyzier->OpenFile("domain.rge")) {
		cout << "Error in open file" << endl;
		exit(1);
	}
	RGEDomainSpecific* domain = analyzier->RgeAnalyse();

	if(!PrintAllErrors())
		domain->DisplayRGEDomainSpecific();

	return 0;
}
#endif

#if 0
#include "syntax/grammar_generator_factory.h"
#include "syntax_specific.h"

int main(){
	std::unique_ptr<GrammarGenerator> gen = CreateGrammarGenerator("QGrammarGeneratorFactory");
	gen->OpenFile("grammar.syn");

	ContextFreeGrammar gram = gen->GrammarGenerate();
	gram.PrintGrammar();

	gram.ElimLeftRecur();

	//gram.PrintGrammar();

	//gram.GetFirstTable();
	//gram.GetFollowTable();
	
	gram.PrintGrammar();

	return 0;
}
#endif

#if 1
#include "syntax/grammar_generator_factory.h"
#include "syntax_specific.h"

int main() {
	std::unique_ptr<GrammarGenerator> gen = CreateGrammarGenerator("QGrammarGeneratorFactory");
	gen->OpenFile("grammar.syn");

	ContextFreeGrammar gram = gen->GrammarGenerate();
	//gram.PrintGrammar();

	gram.ElimLeftRecur();

	//gram.PrintGrammar();
	//gram.LeftFactoring();
	gram.GetFirstTable();
	gram.GetFollowTable();
	gram.GetSelectTable();
	gram.ConstructLL1Table();

	std::unique_ptr<SentenceReader> reader = CreateSentenceReader("QSentenceReader");
	std::vector<std::string> vec = reader->ReadFile("sentence.stn");
	std::cout << "Sentence read from file:" << std::endl;
	for (const auto& t : vec) std::cout << t << " ";
	std::cout << std::endl;

	std::unique_ptr<SyntaxTree> tree = gram.LL1Parsing(vec);
	tree->GenerateGraphvz();

	//gram.PrintGrammar();

	return 0;
}
#endif