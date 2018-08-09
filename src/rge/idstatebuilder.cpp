
#include <algorithm>
#include <sstream>
#include "idstatebuilder.h"
#include "q_idstatebuilder.h"

static int StateNodeIDCounter(){
	static int counter = 0;
	return counter++;
}

StateNode::StateNode() : stateNum_(StateNodeIDCounter()){
	std::stringstream stream;
	stream << stateNum_;
	stateName_ = "StateNode" + stream.str();
}

void StateNode::AddNoJudgeTransfer(std::shared_ptr<StateNode> node) {
	std::shared_ptr<Judgement> judge(new Judgement());
	_addJudge(judge, node);
}

void StateNode::AddJudgeTransfer(std::shared_ptr<Judgement> judge, std::shared_ptr<StateNode> node) {
	_addJudge(judge, node);
}

JudgeTransfer* StateNode::GetJudgeTransfer(const std::string& str, std::shared_ptr<StateNode> node)
{
	auto iter_pair = judges_.equal_range(str);
	for(auto iter = iter_pair.first; iter != iter_pair.second; iter++){
		if(node == nullptr) break;
		else if (iter->second.second == node) return &iter->second;
	}
	return nullptr;
}

void StateNode::AddTail(JudgeTransfer* p) {
	tails_.insert(p);
}

void StateNode::AddTailSet(const TailJudge* tail) {
	for (const auto t : *tail) {
		if (tails_.find(t) == tails_.end()) tails_.insert(t);
	}
}

void StateNode::RemoveTailSet(const TailJudge* tail) {
	for(const auto t : *tail) tails_.erase(t);
}

void StateNode::ResetTailJudge(std::shared_ptr<Judgement> judge, std::shared_ptr<StateNode> next) {
	for (auto elem : tails_) {
		JudgeTransfer* p = elem;
		p->second->judges_.clear();
		if (judge == nullptr) p->second->AddNoJudgeTransfer(next);
		else p->second->AddJudgeTransfer(judge, next);
	}
}

void StateNode::RemoveTailNode(std::shared_ptr<StateNode> node){
	for(auto elem: tails_){
		JudgeTransfer* p = elem;
		if (p->second == node) {
			tails_.erase(p);
			break;
		}
	}
}

void StateNode::_addJudge(std::shared_ptr<Judgement> judge, std::shared_ptr<StateNode> node) {
	JudgeTransfer p = std::make_pair(judge, node);
	judges_.insert(make_pair(judge->JudgeStr(), p));
}

