
#pragma once

#include <list>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <set>
#include "idstatebuilder.h"

static const std::string NO_JUDGE = "No judge";

class Judgement {
public:
	Judgement(const std::string& str)  {
		if (!str.empty()) judgeStr_ = str;
	}
	Judgement() {}
	void SetJudgeStr(const std::string& str){
		judgeStr_ = str;
	}
	std::string JudgeStr() const { return judgeStr_; }
private:
	std::string judgeStr_{ NO_JUDGE };
};

class StateNode;

//The pointers need to be passed and linked, so we use shared_ptr here
using JudgeTransfer = std::pair<std::shared_ptr<Judgement>, std::shared_ptr<StateNode>>;
using JudgeList = std::multimap<std::string, JudgeTransfer>;
using TailJudge = std::unordered_set<JudgeTransfer*>;
using StateTable = std::map<std::string, std::shared_ptr<StateNode>>;
using RuleTable = std::map<std::string, std::map<char, std::string>>;
using EpsilonClosure = std::map<std::string, std::set<std::string>>;
using NewStateTerminalTable = std::map<std::string, bool>;

class StateNode {
public:
	StateNode();

	void AddNoJudgeTransfer(std::shared_ptr<StateNode> node);

	void AddJudgeTransfer(std::shared_ptr<Judgement> judge, std::shared_ptr<StateNode> node);

	JudgeTransfer* GetJudgeTransfer(const std::string& str, std::shared_ptr<StateNode> = nullptr);

	JudgeList& GetJudgeList() { return judges_; }

	TailJudge* GetTails() { return &tails_; }

	void ResetTailJudge(std::shared_ptr<Judgement> judge, std::shared_ptr<StateNode> next) ;

	void AddTail(JudgeTransfer* p);

	void AddTailSet(const TailJudge* tail) ;

	void RemoveTailSet(const TailJudge*);

	void RemoveTailNode(std::shared_ptr<StateNode> node);

	std::string StateNodeName() const { return stateName_; }

	void SetTerminal(bool b = true) { terminal_ = b; }

	bool IsTerminal() const { return terminal_; }

private:
	void _addJudge(std::shared_ptr<Judgement> judge, std::shared_ptr<StateNode> node);
	friend class QIDStateBuilder;

	JudgeList judges_{};
	TailJudge tails_{};
	int stateNum_{-1};
	std::string stateName_{};
	bool terminal_{false};
};

class QIDStateBuilder : public IDStateBuilder {
public:
	void BuildIDState(const std::string& str) override;

	void GenerateGraphvz(std::string filename) const override;

	std::shared_ptr<DFA> GenerateDFA() override;

private:
	void _recurStateBuild(std::shared_ptr<StateNode> s_prev, std::shared_ptr<StateNode> p_prev,
		CONNECTION_TYPE jtype, const char** ptr);

	std::string ExtractJudgeStr(const char** ptrp);

	void _generateGraphvz(std::ofstream& outfile) const;

	void _generateDFAGraphvz(std::shared_ptr<DFA> p) const;

	std::vector<std::pair<std::shared_ptr<Judgement>, std::shared_ptr<StateNode>>>
	_handleXToY(const std::string& judge, std::shared_ptr<StateNode>& ptr);

	void _stateSpawn();

	void _handleSpawnStates(std::tuple < std::shared_ptr<StateNode>, std::string, std::shared_ptr<StateNode>> tpl);

	void _removeJudge(StateNode* from, const std::string& judge, StateNode* to);

	void _stateTableInsert(std::shared_ptr<StateNode> node);

	std::shared_ptr<StateNode> _stateTableGet(const std::string& name);

	std::set<std::string> _stateSetTransfer(const std::set<std::string>& from, std::string judge);

	std::set<std::string> _stateSetNoJudgeTransfer(const std::set<std::string>& from);

	void _setStateTerminal();

	bool _stateSetIsTerminal(const std::set<std::string>& st);

	void _generateEpsilonClosure();

	std::shared_ptr<StateNode> rootState_;

	StateTable stateTable_;
	RuleTable ruleTable_;
	EpsilonClosure epClosure_;
	NewStateTerminalTable nstTable_;
	std::set<char> allChars_;

	const char epsilon_{'#'};
};

