

#include "syntax/grammar_generator_factory.h"
#include "syntax_specific.h"

int main(){
	std::unique_ptr<GrammarGenerator> gen = CreateGrammarGenerator("QGrammarGeneratorFactory");

	return 0;
}
