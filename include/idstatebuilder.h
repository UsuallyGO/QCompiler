#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <map>

struct ID_Identifier {
	static const char STATE_BEGIN{ '(' };
	static const char STATE_END{ ')' };
	static const char JUDGE_BEGIN{ '[' };
	static const char JUDGE_END{ ']' };
	static const char REPEAT{ '*' };
	static const char PARALLEL{ '|' };
	static const char DASH{ '-' };
};

struct DFA{
	DFA(int len = 0) : table_(len), isTerminal_(len, false){}
	void StateSet(int from, char c, int to){
		_indexCheck(from);
		std::map<char, int>& mp = table_[from];
		mp.insert(std::make_pair(c, to));
	}
	int StateTransfer(int from, char c) const {
		_indexCheck(from);
		const std::map<char, int>& mp = table_[from];
		auto iter = mp.find(c);
		if(iter == mp.end()) return unReachable_;
		else return iter->second;
	}

	bool _indexCheck(int ind) const { return ind >= 0 && ind < table_.size(); }

	std::vector<std::map<char, int>> table_;
	std::vector<bool> isTerminal_;
	int unReachable_{-1};
};

enum class CONNECTION_TYPE {
	CONN_SERIAL, CONN_PARALLEL
};

class IDStateBuilder {
public:
	virtual void BuildIDState(const std::string& str) = 0;
	virtual void GenerateGraphvz(std::string filename) const {
		std::cout << "Cannot generate Graphvz file" << std::endl;
	}
	virtual std::shared_ptr<DFA> GenerateDFA(){
		std::cout << "Cannot generate DFA" << std::endl;
		return nullptr;
	}
};
