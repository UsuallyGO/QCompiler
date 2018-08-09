
QCompiler is a simple program that can generate a compiler, the lexical rules and grammar rules are specified by user.

Any questions can contact me: qinliok@126.com

Under file/ directory, there are some samples.

domain.rge defines the lexical rules. Postfix 'rge' stands for regular expression, because we use regular expressions
to express the lexical rules of ID. Other lexical units are relatively simple, we can enumerate them.

grammar.syn defines the grammar of the programming language. Postfix 'syn' stands for syntax. Grammar is built by 
terminals and nonterminals，the nonterminals should be wrapped by angle brackets.

sentence.stn gives some sample sentences, if the grammar has been correctly parsed, the sentences can deduce to a 
syntax tree.

Some result will be displayed as graph, use Dot tools to generate visual files from *.gv.