
#pragma once

#include <set>
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

#include "sys_env.h"

struct SyntaxSemantics{
	static const char BLANK{ ' ' };
	static const char ENTER{ _ENTER_ };
	static const char NEWLINE{ _NEWLINE_ };
	static const char ITEM_BEGIN{ '<' };
	static const char ITEM_END{ '>' };
	static const char PARALLEL_CONNECT{ '|' };
	static const char EPSILON{ '#' };
	static const char FINISH{ '$' };
	static const std::string PRODUCE; //default is "->"
};

struct SyntaxNode{
	
	enum NodeType { TERMINAL, NONTERMINAL, FINISH };

	SyntaxNode(const std::string& name = "", NodeType t = NONTERMINAL) : term(name), type(t){}

	void addChild(const std::string& name, NodeType t){
		children.emplace_back(std::make_unique<SyntaxNode>(name, t));
	}
	SyntaxNode* getChild(size_t num){
		if(num >= children.size()) return nullptr;
		return children[num].get();
	}
	std::string getTerm() const {
		return term;
	}

	const std::string term;
	NodeType type; //terminal or nonterminal

	std::vector<std::unique_ptr<SyntaxNode>> children;
};

struct SyntaxTree{

	SyntaxTree() : head(new SyntaxNode(headName, SyntaxNode::NONTERMINAL)), counter(0){}
	void CountIncrease() { counter++; }
	SyntaxNode* GetHead() { return head.get(); }
	bool IsAccepted() const { return accepted; }
	void NotAccepted() { accepted = false; }
	void Accepted() { accepted = true; }

	void GenerateGraphvz(const std::string& filename = "parsing.gv");
	void _generateGraphvz(std::ofstream& outfile);

	const std::string headName{ "head" };
	std::unique_ptr<SyntaxNode> head;
	bool accepted{ false };
	int counter{ 1 }; //at least has a head
};

struct FirstTable{
	using Iterator = std::map<std::string, std::set<std::string>>::iterator;
	using Const_Iterator = std::map<std::string, std::set<std::string>>::const_iterator;

	bool Union(const std::string& term, const std::set<std::string>& st);
	bool UnionExclude(const std::string& term, const std::set<std::string>& st, 
						const std::string& exclude);
	void ChangeClear() { changed = true; }
	bool Insert(const std::string& term, const std::string& st);

	std::set<std::string> GetFirstSet(const std::string& term);
	std::set<std::string>& operator[](const std::string& term);
	std::set<std::string> GetFirstSet(const std::string& term) const;

	Iterator begin() { return table.begin(); }
	Iterator end() { return table.end(); }
	Iterator find(const std::string& term) { return table.find(term); }

	//those const functions are necessary, or you cannot get iterator in const functions
	Const_Iterator begin() const { return table.cbegin(); }
	Const_Iterator end() const { return table.cend(); }
	Const_Iterator find(const std::string& term) const { return table.find(term); }

	bool changed{ false };
	std::map<std::string, std::set<std::string>> table;
};

struct NullableTable {
	using Iterator = std::map<std::string, bool>::iterator;
	using Const_Iterator = std::map<std::string, bool>::const_iterator;

	bool SetValue(const std::string& term, bool b) {
		changed = true;
		if (table.find(term) != table.end() && table[term] == b) changed = false;
		else table[term] = b;

		return changed;
	}

	bool GetValue(const std::string& term) { return (*this)[term]; }
	bool GetValue(const std::string& term) const {
		auto iter = table.find(term); 
		return iter->second;
	}
	bool& operator[](const std::string& term) { return table[term];	}
	
	Iterator begin() { return table.begin(); }
	Iterator end() { return table.end(); }
	Iterator find(const std::string& term) { return table.find(term); }

	Const_Iterator begin() const { return table.cbegin(); }
	Const_Iterator end() const { return table.cend(); }
	Const_Iterator find(const std::string& term) const { return table.find(term); }

	bool changed{ false };
	std::map<std::string, bool> table;
};

struct ContextFreeGrammar{
	using TerminalTable = std::set<std::string>;
	using NonTerminalTable = std::set<std::string>;
	using FollowTable = FirstTable;
	/* the key of Production should be 'const', because the key in ProductionTable is const 
	 * (Once you insert the pair into ProductionTable, next time you get it, the key is const.)*/
	using Production = std::pair<const std::string, std::vector<std::string>>;
	using ProductionTable = std::multimap<std::string, std::vector<std::string >>;
	using FactorPrefix = std::set<std::vector<std::string>>;
	using SelectTable = std::map<Production*, std::set<std::string>>;

	struct _InnerNameGenerator{
		_InnerNameGenerator() {}
		_InnerNameGenerator(const _InnerNameGenerator& g) {
			counter = g.counter;
		}
		std::string GenerateName();

		int counter{ 0 };
		std::string prefix{ "_inner_" };
	};

	struct LL1Table{
		using LL1TableRow = std::map<std::string, Production*>;
		using LLTParseTable = std::map<std::string, LL1TableRow>;
		using Iterator = LLTParseTable::iterator;
		using Const_Iterator = LLTParseTable::const_iterator;

		Production* Parse(const std::string& nonTerm, const std::string& termi) const {
			auto iter = table.find(nonTerm);
			if (iter == table.end()) return nullptr;
			const LL1TableRow& row = iter->second;
			auto iter2 = row.find(termi);
			if(iter2 == row.end()) return nullptr;
			else return iter2->second;
		}
		Production* Parse(const std::string& nonTerm, const std::string& termi) {
			const LL1TableRow& row = table[nonTerm];
			auto iter = row.find(termi);
			if (iter == row.end()) return nullptr;
			else return iter->second;
		}
		
		bool Insert(const std::string& nonTerm, const std::string& termi, Production* p){
			bool inserted = false;
		
			LL1TableRow& row = table[nonTerm];
			if(row.find(termi) == row.end())
				inserted = true, row.insert(std::make_pair(termi, p));
			return inserted;
		}

		Iterator begin() { return table.begin(); }
		Iterator end() { return table.end(); }

		Const_Iterator begin() const { return table.cbegin(); }
		Const_Iterator end() const { return table.cend(); }

		LLTParseTable table;
	};

	std::string startSymbol{ "_start_" };
	NonTerminalTable nonTerminals;
	TerminalTable terminals;
	ProductionTable productions;

	FirstTable first;
	FollowTable follow;
	SelectTable select;
	NullableTable nullable;
	const static std::string Epsilon;
	const static std::string Finish;
	bool addStartSymbol{ false };
	LL1Table ll1table;

	_InnerNameGenerator nameGenerator;

	ContextFreeGrammar() {}
	ContextFreeGrammar(const ContextFreeGrammar& grammar) {
		startSymbol = grammar.startSymbol;
		nonTerminals = grammar.nonTerminals;
		terminals = grammar.terminals;
		productions = grammar.productions;

		first = grammar.first;
		follow = grammar.follow;
		nullable = grammar.nullable;

		addStartSymbol = grammar.addStartSymbol;
		nameGenerator = grammar.nameGenerator;
		ll1table = grammar.ll1table;
	}

	void GetFirstTable();
	void GetFollowTable();
	void GetSelectTable();

	/* The difference between Eliminate left recursion and left factoring is that: the former one works for
	 * productions that the	first term of the right part is the same as left part(should be nonterminals), 
	 * the latter one works for multi-productions that their right part start with same substring(the first
	 * term should NOT be the same as the left part, or it is a left recursion).
	 *
	 * So, you should eliminate left recursion first and then do left factoring.
	 *
	 * !!! You should know about this: although you eliminate the left recursion and extract the left factor,
	 * it cannot guarantee that your grammar is no longer ambiguous. You'd better try your best to eliminate
	 * the ambiguity when you design your grammar.
	 */
	void ElimLeftRecur();

	/* Left factoring can only deter the parsing decision until enough of the input has been seen that we can 
	 * make the right choice.
	 */
	void LeftFactoring();

	bool ConstructLL1Table();

	std::unique_ptr<SyntaxTree> LL1Parsing(const std::vector<std::string>& sentence) const;

	void PrintGrammar() const;

	std::set<std::string> _getSentenceFirst(const std::vector<std::string>& p) const;

	bool _elimImmediateLeftRecur(const std::string& term);
	void _leftSubstitue(const std::string& A, const std::string& S);

	FactorPrefix _getLeftFactor(const std::string& term) const ;
	int _findLongestLeftFactor(const std::vector<std::string>& vec_i, 
						const std::vector<std::string>& vec_j) const;
	void _leftFactoring(const std::string& term, const FactorPrefix& prefix);

	bool _allNullable(const std::vector<std::string>& Y, int beg, int end);
	bool _isTerminal(const std::string& term) const;
	bool _isNullable(const std::string& term) const;

	bool _nontermIsUsing(const std::string& term) const;

	void _printFirst() const { 
		std::cout << "First table:" << std::endl;
		_printFFTable(first); 
	}
	void _printFollow() const { 
		std::cout << "Follow table:" << std::endl;
		_printFFTable(follow); 
	};
	void _printSelect() const {
		std::cout << "Select table:" << std::endl;
		for(const auto& s : select){
			Production* p = s.first;
			std::cout << p->first<< "->";
			for(const auto& t : p->second)
				std::cout << t <<" ";
			std::cout << ":";
			const std::set<std::string>& st = s.second;
			for(const auto& nt : st)
				std::cout << nt << " ";
			std::cout << std::endl;
		}
	}
	void _printLL1Table() const;
	void _printProduction(const Production* p) const{
		std::cout << p->first << "-> ";
		for(const auto& s : p->second)
			std::cout << s << " ";
		std::cout << std::endl;
	}
	void _printFFTable(const FirstTable& ) const;
};

class GrammarGenerator{
public:
	GrammarGenerator(){}
	virtual ~GrammarGenerator(){}

	virtual bool OpenFile(const std::string& ) = 0;

	virtual ContextFreeGrammar GrammarGenerate() = 0;
};

class SentenceReader{
public:
	SentenceReader(){}
	virtual ~SentenceReader(){}

	virtual std::vector<std::string> ReadFile(const std::string& ) = 0;
};

std::unique_ptr<GrammarGenerator> CreateGrammarGenerator(const std::string& name);

std::unique_ptr<SentenceReader> CreateSentenceReader(const std::string& name);
