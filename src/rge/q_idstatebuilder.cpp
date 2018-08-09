
#include <string>
#include <set>
#include <map>
#include <cassert>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "idstatebuilder.h"
#include "q_idstatebuilder.h"
#include "idstatebuilder_factory.h"
#include "utility/utility_internal.h"

const char STATE_CONNECT = '_';

static std::string _transferToString(StateNode* from, const std::string& judge, StateNode* to){
	std::stringstream ss;
	std::string res;
	std::string tmp;

	ss << from;
	ss >> res;

	res += judge;

	ss.clear();
	ss << to;
	ss >> tmp;
	res += tmp;
	return res;
}

void QIDStateBuilder::_stateTableInsert(std::shared_ptr<StateNode> node){
	stateTable_.insert(std::make_pair(node->StateNodeName(), node));
}

std::shared_ptr<StateNode> QIDStateBuilder::_stateTableGet(const std::string & name){
	auto iter = stateTable_.find(name);
	if(iter == stateTable_.end()) return nullptr;
	else return iter->second;
}

bool QIDStateBuilder::_stateSetIsTerminal(const std::set<std::string>& st){
	for(auto iter = st.begin(); iter != st.end(); iter++){
		std::shared_ptr<StateNode> node = _stateTableGet(*iter);
		if(node->IsTerminal()) return true;
	}
	return false;
}

void QIDStateBuilder::_setStateTerminal(){
	for(auto iter = stateTable_.begin(); iter != stateTable_.end(); iter++){
		std::shared_ptr<StateNode> node = iter->second;
		if(node->GetJudgeList().size() == 0) node->SetTerminal();
	}
}

std::string QIDStateBuilder::ExtractJudgeStr(const char** ptrp) {
	const char* ptr = *ptrp;
	if (*ptr == ID_Identifier::JUDGE_BEGIN) {
		ptr++;
		std::string tmp;
		while(*ptr) {
			if (*ptr == ID_Identifier::STATE_BEGIN || *ptr == ID_Identifier::STATE_END
				|| *ptr == ID_Identifier::REPEAT || *ptr == ID_Identifier::PARALLEL
				|| *ptr == ID_Identifier::JUDGE_BEGIN) //error
				break;
			else if (*ptr == ID_Identifier::JUDGE_END) {
				ptr++;
				*ptrp += ptr - *ptrp;
				return tmp;
			}
			else tmp += *ptr++;
		}
		return std::string();
	}
	else {
		std::string tmp;
		while (*ptr) {
			if (*ptr == ID_Identifier::JUDGE_BEGIN || *ptr == ID_Identifier::JUDGE_END
				|| *ptr == ID_Identifier::REPEAT || *ptr == ID_Identifier::PARALLEL
				|| *ptr == ID_Identifier::STATE_BEGIN || *ptr == ID_Identifier::STATE_END)
				break;
			else tmp += *ptr++;
		}
		*ptrp += ptr - *ptrp;
		return tmp;
	}
}

/* This function is used to parse the ID definition regular expression and build the
 * ID recognize rules(or to say, first to build the NFA, then to build the DFA). 
 * Those rules are very important for language lexical analysis. This function should
 * be called only when QRgeAnalyzier::_checkID returns true.
 *
 * For an ID regular expression, it is composed by several connected parts. There two
 * kinds of connections: serial connection and parallel connection. And for each part,
 * there are three ways to express: (xxx), [xx-xx] and xxx.
 *
 * Parenthesis expression can be embedded, for example ((xx)(yy)) is legal. But bracket
 * expression cannot be embedded. Thus, parenthesis needs to be recursively parsed.
 *
 * Serial connection has a higher priority than parallel connection, for example, 
 * [aaa]|[bbb][ccc] shoud be parsed as [aaa]|([bbb][ccc]).
 *
 * In building, we need two pointers, one points to the head of parallel part, the other 
 * one points to the direct previous serail part.
 *
 * ([abc][123][edf][0123][ghi]|[4-8])
 * ^          ^          ^
 * |          |          |
 * p_prev     s_prev     current
 *
 * Just as above, p_prev points to the '(', s_prev points to the [edf] and now we are
 * going to handle [ghi].
 *
 * Parallel connections is a little more easy. For serial connection, we need to deal 
 * with three difference cases:
 * 1. ([] or ((): s_prev and p_prev both points to the first '(', cur_node points to []
 * or (). In this situation, we transfer from s_prev(or p_prev, they are the same node)
 * to cur_node and add cur_node's tail set to p_prev(or s_prev).
 * 
 * 2. [][] or [](): s_prev points to the first []. In this case, we transfer from s_prev
 * to cur_node and upgrade cur_node or cur_node's tail set to p_prev's tail set.
 * 
 * 3. ()[] or ()(): s_prev points to the first (). In this case, we transfer from s_prev
 * to cur_node. For ()[], we need to set ()'s tail points to [], then add the transfer 
 * which from tail to [] to p_prev's tail set. Pay attention that, what we need to do is
 * NOT add ()'s tail to p_prev's tail set, but add the tranfser from ()'s tail to [].
 *
 * Notice about this: until we meet PARALLEL indicator(such as '|'), we set connection
 * type to PARALLEL, or we always set connection type to SERIAL.
 */
void QIDStateBuilder::_recurStateBuild(std::shared_ptr<StateNode> s_prev, std::shared_ptr<StateNode> p_prev, 
					CONNECTION_TYPE jtype, const char** ptr) {
	 while(**ptr) {
		if (**ptr == ID_Identifier::STATE_BEGIN) {
			std::shared_ptr<StateNode> cur_node(new StateNode);//no chance to leak memory, donot need to use make_shared
			_stateTableInsert(cur_node);
			*ptr += 1;
			_recurStateBuild(cur_node, cur_node, CONNECTION_TYPE::CONN_SERIAL, ptr);
			
			if (jtype == CONNECTION_TYPE::CONN_SERIAL) {
				if(s_prev == p_prev) {// case: ((), s_prev and p_prev both points to the first parenthesis
					s_prev->AddNoJudgeTransfer(cur_node); //add cur_node's tail to p_prev id done outof the if statement
				}
				else if(s_prev->tails_.size() == 0) {// case: [](), s_prev points to the []
					p_prev->RemoveTailNode(s_prev);
					s_prev->AddNoJudgeTransfer(cur_node);
				}
				else{ // case: ()()
					s_prev->ResetTailJudge(nullptr, cur_node);
					p_prev->RemoveTailSet(s_prev->GetTails());
					s_prev->tails_.clear();
				}
			}
			else p_prev->AddNoJudgeTransfer(cur_node);
			
			p_prev->AddTailSet(cur_node->GetTails());
			s_prev = cur_node;
		}
		else if (**ptr == ID_Identifier::STATE_END) {
			jtype = CONNECTION_TYPE::CONN_SERIAL;
			*ptr += 1;
			return;
		}
		else if (**ptr == ID_Identifier::REPEAT) {
			s_prev->ResetTailJudge(nullptr, s_prev);// s_prev transfers to itselft
			std::shared_ptr<Judgement> judge(new Judgement);
			std::shared_ptr<StateNode> node(new StateNode);
			_stateTableInsert(node);
			s_prev->AddJudgeTransfer(judge, node); 

			p_prev->RemoveTailSet(s_prev->GetTails());//s_prev cannot be equal to p_prev, because s_prev at least points to a ()
			s_prev->tails_.clear();
			p_prev->AddTail(s_prev->GetJudgeTransfer(judge->JudgeStr(), node)); //s_prev has only one 

			s_prev = node;
			jtype = CONNECTION_TYPE::CONN_SERIAL;
			*ptr += 1;
		}
		else if (**ptr == ID_Identifier::PARALLEL) {
			s_prev = p_prev;
			jtype = CONNECTION_TYPE::CONN_PARALLEL;
			*ptr += 1;
		}
		else { //*ptr == ID_Identifier::JUDGE_BEGIN || *ptr == xxx, dealed as parenthesis situation
			std::string judgestr = ExtractJudgeStr(ptr);
			std::shared_ptr<Judgement> judge(new Judgement(judgestr));
			std::shared_ptr<StateNode> node(new StateNode);
			_stateTableInsert(node);
			
			if (jtype == CONNECTION_TYPE::CONN_PARALLEL){
				p_prev->AddJudgeTransfer(judge, node);
				p_prev->AddTail(p_prev->GetJudgeTransfer(judge->JudgeStr(), node));
			}
			else 
			{
				if(s_prev == p_prev) {// case: ([], both s_prev and p_prev points to the parenthesis
					s_prev->AddJudgeTransfer(judge, node);
					p_prev->AddTail(s_prev->GetJudgeTransfer(judge->JudgeStr(), node));
				}
				else if(s_prev->tails_.size() == 0){ //case [][], s_prev points to the first []
					p_prev->RemoveTailNode(s_prev);
					s_prev->AddJudgeTransfer(judge, node);
					p_prev->AddTail(s_prev->GetJudgeTransfer(judge->JudgeStr(), node));
				}
				else{
					s_prev->ResetTailJudge(judge, node); //case ()[]
					TailJudge* tail = s_prev->GetTails();
					for (auto iter = tail->begin(); iter != tail->end(); iter++) {
						p_prev->AddTail((*iter)->second->GetJudgeTransfer(judge->JudgeStr(), node));
					}
					p_prev->RemoveTailSet(s_prev->GetTails());
					s_prev->tails_.clear();
				}
			}
			s_prev = node;
			jtype = CONNECTION_TYPE::CONN_SERIAL;
		}
	}
}

/* From state set 'from', we reach states that only transfer by epsilon(NO_JUDGE in this project).
 * We can traval 0 or more than 0 times.
 */
std::set<std::string> QIDStateBuilder::_stateSetNoJudgeTransfer(const std::set<std::string>& from){
	std::set<std::string> stateset;
	std::set<std::shared_ptr<StateNode>> nodeset;

	for(auto iter = from.begin(); iter != from.end(); iter++){
		nodeset.insert(_stateTableGet(*iter));
		stateset.insert(*iter);
	}

	while(!nodeset.empty()){
		std::shared_ptr<StateNode> node = *nodeset.begin();
		JudgeList& list = node->GetJudgeList();
		for(auto iter = list.begin(); iter != list.end(); iter++){
			if(iter->first == NO_JUDGE){
				if(stateset.find(iter->second.second->StateNodeName()) == stateset.end()){
					nodeset.insert(iter->second.second);
					stateset.insert(iter->second.second->StateNodeName());
				}
			}
		}
		nodeset.erase(node);
	}
	return stateset;
}

/* From states set 'from', we reach new states which transfer by 'judge'.
 * Notica about the difference to _stateSetNoJudgeTransfer. We can only tranfer by 'judge' one time here.
 */
std::set<std::string> QIDStateBuilder::_stateSetTransfer(const std::set<std::string>& from, const std::string judge){
	std::set<std::string> stateset;

	for(auto iter = from.begin(); iter != from.end(); iter++){
		std::shared_ptr<StateNode> node = _stateTableGet(*iter);
		assert(node != nullptr);
		JudgeList& list = node->GetJudgeList();
		for(auto iter = list.begin(); iter != list.end(); iter++){
			if(iter->first == judge) stateset.insert(iter->second.second->StateNodeName());
		}
	}

	std::set<std::string> nojudge_stateset = _stateSetNoJudgeTransfer(stateset);
	for(auto iter = nojudge_stateset.begin(); iter != nojudge_stateset.end(); iter++)
		stateset.insert(*iter);

	return stateset;
}

void QIDStateBuilder::_generateEpsilonClosure(){
	for(auto iter = stateTable_.begin(); iter != stateTable_.end(); iter++){
		std::set<std::string> dest;
		std::set<std::shared_ptr<StateNode>> nodeset;
		std::set<std::shared_ptr<StateNode>> nodevisit;

		nodeset.insert(iter->second); //epsilon closure should include state-self
		while(!nodeset.empty()){
			std::shared_ptr<StateNode> node = *nodeset.begin();
			dest.insert(node->StateNodeName());
			nodevisit.insert(node); //node may can transfer to itself, so set nodevisit before anything else
			nodeset.erase(node);

			const JudgeList& list = node->GetJudgeList();
			for(auto iter2 = list.begin(); iter2 != list.end(); iter2++){
				std::string judge = iter2->first;
				std::shared_ptr<StateNode> dest_node = iter2->second.second;

				if(judge == NO_JUDGE){
					dest.insert(dest_node->StateNodeName());
					if(nodevisit.find(dest_node) == nodevisit.end()) nodeset.insert(dest_node);
				}
			}
		}

		epClosure_.insert(std::make_pair(iter->first, dest));
	}
}

void QIDStateBuilder::_removeJudge(StateNode* from, const std::string& judge, StateNode* to){
	JudgeList& list = from->GetJudgeList();
	for(auto iter = list.begin(); iter != list.end(); /*++ inside*/){
		if(iter->second.second.get() == to && iter->second.first->JudgeStr() == judge) list.erase(iter++);
		else iter++;
	}
}

std::vector<std::pair<std::shared_ptr<Judgement>, std::shared_ptr<StateNode>>>
QIDStateBuilder::_handleXToY(const std::string& judge, std::shared_ptr<StateNode>& ptr){
	std::shared_ptr<StateNode> no_judge(new StateNode);
	_stateTableInsert(no_judge);
	ptr = no_judge;
	std::vector<std::pair<std::shared_ptr<Judgement>, std::shared_ptr<StateNode>>> vec;
	
	for (char c = judge[0]; c <= judge[2]; c++) {
		allChars_.insert(c);
		std::shared_ptr<StateNode> node (new StateNode);
		_stateTableInsert(node);
		std::string tmp(1, c);
		std::shared_ptr<Judgement> judgement(new Judgement(tmp));
		
		vec.push_back(std::make_pair(judgement, node));
		node->AddNoJudgeTransfer(no_judge);
	}

	return vec;
}

void QIDStateBuilder::_handleSpawnStates(std::tuple<std::shared_ptr<StateNode>, std::string, std::shared_ptr<StateNode>> tpl){
	std::shared_ptr<StateNode> prev = std::get<0>(tpl);
	std::shared_ptr<StateNode> from = std::get<0>(tpl);
	std::shared_ptr<StateNode> to = std::get<2>(tpl);
	std::string judge = std::get<1>(tpl);
	int ind = 0;

	while(1){
		if(ind == judge.length() - 1){
			if(prev == from) {
				allChars_.insert(judge[ind]);
				break;//only one char in judgestring
			}
			else{ //more than two chars and the last judge is a single char, for example, xxxa
				allChars_.insert(judge[ind]);
				std::string to_judge(1, judge[ind]);
				std::shared_ptr<Judgement> judgement(new Judgement(to_judge));
				prev->AddJudgeTransfer(judgement, to);
				break;
			}
		}
		else if(judge[ind + 1] == '-'){ //zzzx-yzzz
			std::shared_ptr<StateNode> no_judge;
			auto vec = _handleXToY(judge.substr(ind, 3), no_judge);
			if(prev == from){//x-yzzz
				_removeJudge(prev.get(), judge, to.get());
			}
			for(auto pr : vec) prev->AddJudgeTransfer(pr.first, pr.second);
			if(ind == judge.length() - 3){
				no_judge->AddNoJudgeTransfer(to);
				break;
			}
			prev = no_judge;
			ind += 3;
		}
		else{ //zzzxzzz
			std::string to_judge(1, judge[ind]);
			allChars_.insert(judge[ind]);
			std::shared_ptr<Judgement> judgement(new Judgement(to_judge));
			std::shared_ptr<StateNode> node(new StateNode);
			_stateTableInsert(node);
			if(prev == from) _removeJudge(prev.get(), judge, to.get());
			prev->AddJudgeTransfer(judgement, node);
			prev = node;
			ind++;
		}
	}	
}

/* State spawn means clone states by its judgement. For example, node A can transfer to node B under 
 * the judgement [a-c]. Before spawn, it is A--(judge:[a-c])-->B, after spawn it is A--(judge:a)-->B,
 * A--(judge:b)-->B, A--(judge:c)-->B. After spawn, every judgement has only one character. This is
 * more convient for generating the DFA.
 */
void QIDStateBuilder::_stateSpawn(){
	std::set<std::string> alledges;
	std::map<std::string, std::tuple<std::shared_ptr<StateNode>, std::string, std::shared_ptr<StateNode>>> edgetable;
	std::set<std::shared_ptr<StateNode>> nodeset;
	std::set<std::shared_ptr<StateNode>> visit;

	nodeset.insert(rootState_);

	while(!nodeset.empty()){
		std::shared_ptr<StateNode> node = *nodeset.begin();
		JudgeList& list = node->GetJudgeList();
		for(auto iter = list.begin(); iter != list.end(); iter++){
			if(visit.find(iter->second.second) == visit.end()) nodeset.insert(iter->second.second);

			if(iter->first == NO_JUDGE) continue;
			else{
				std::string edge = _transferToString(node.get(), iter->first, iter->second.second.get());
				alledges.insert(edge);
				edgetable[edge] = std::make_tuple(node, iter->first, iter->second.second);
			}
		}
		nodeset.erase(node);
		visit.insert(node);
	}

	while(!alledges.empty()){
		std::string edge = *alledges.begin();
		alledges.erase(edge);
		auto tpl = edgetable[edge];

		_handleSpawnStates(tpl);
	}
}

void QIDStateBuilder::_generateDFAGraphvz(std::shared_ptr<DFA> dfa) const {
	std::ofstream outfile("dfagraph.gv"); //check outfile here
	if(!outfile){
		assert(0);
	}
	/*outfile << "strict ";*/ //use 'strict' can  remove duplicate edges
	outfile << "digraph G {" << std::endl; //use 'strict' can  remove duplicate edges
	outfile << "node[shape = circle]" << std::endl;

	for(int ind = 0; ind < dfa->isTerminal_.size(); ind++){
		if(dfa->isTerminal_[ind]){
			std::string node = intToString(ind);
			std::string str = node + "[shape = doublecircle]";
			outfile << str << std::endl;
		}
	}

	for(int ind = 0; ind < dfa->table_.size(); ind++){
		auto& mp = dfa->table_[ind];
		std::string str_from = intToString(ind);
		for(auto iter = mp.begin(); iter != mp.end(); iter++){
			char c = iter->first;
			int to = iter->second;
			std::string str = str_from + "->" + intToString(to) + "[label = \"" + c + "\"]";
			outfile << str << std::endl;
		}
	}

	outfile << "}" << std::endl;
	outfile.close();
}
 
std::shared_ptr<DFA> QIDStateBuilder::GenerateDFA(){
	std::map<std::string, std::set<std::string>> newstate_oldstates;
	std::set<std::string> unvisit_newstate;
	std::map<std::string, int> newstate_index;
	int counter = 0;

	_generateEpsilonClosure();
	auto iter = epClosure_.find(rootState_->StateNodeName());
	assert(iter != epClosure_.end()); //iter->first: rootState_, iter->second:states set which epsilon transfers to
	std::string str = setToString(iter->second, STATE_CONNECT);
	newstate_oldstates.insert(std::make_pair(str, iter->second));
	newstate_index.insert(std::make_pair(str, counter++));

	unvisit_newstate.insert(str);
	while(!unvisit_newstate.empty()){
		std::string newstate = *unvisit_newstate.begin();
		unvisit_newstate.erase(newstate);
		
		auto iter_newstate_states = newstate_oldstates.find(newstate);//check iterstate_states here
		const std::set<std::string>& newstate_states = iter_newstate_states->second;

		std::map<char, std::string> tmp_mp;
		for(auto c_iter = allChars_.begin(); c_iter != allChars_.end(); c_iter++){
			if(*c_iter == epsilon_) continue;
			std::string judge(1, *c_iter);
			std::set<std::string> move_closure = _stateSetTransfer(newstate_states, judge);//move to *c_iter
			std::set<std::string> transfer_closure;
			for(auto s_iter = move_closure.begin(); s_iter != move_closure.end(); s_iter++){
				const std::set<std::string>& ep_closure = epClosure_.find(*s_iter)->second;
				setUnion(transfer_closure, ep_closure);
			}

			std::string transfer_str = setToString(transfer_closure, STATE_CONNECT);
			if (transfer_str.empty()) continue;
			tmp_mp.insert(std::make_pair(*c_iter, transfer_str));
			if(newstate_oldstates.find(transfer_str) == newstate_oldstates.end()){
				newstate_oldstates.insert(std::make_pair(transfer_str, transfer_closure));
				newstate_index.insert(std::make_pair(transfer_str, counter++));
				unvisit_newstate.insert(transfer_str);
			}
		}
		ruleTable_.insert(std::make_pair(newstate, tmp_mp));
	}

	std::shared_ptr<DFA> dfa(new DFA(newstate_index.size()));
	for(auto iter2 = newstate_oldstates.begin(); iter2 != newstate_oldstates.end(); iter2++){
		const std::set<std::string>& oldstates = iter2->second;
		int index = newstate_index[iter2->first];
		bool t_bool = _stateSetIsTerminal(oldstates);
		dfa->isTerminal_[index] = t_bool;
	}

	for(auto iter2 = ruleTable_.begin(); iter2 != ruleTable_.end(); iter2++){
		std::string from = iter2->first;
		const auto& mp = iter2->second;
		for(auto iter3 = mp.begin(); iter3 != mp.end(); iter3++){
			char c = iter3->first;
			std::string to = iter3->second;
			dfa->StateSet(newstate_index[from], c, newstate_index[to]);
		}
	}

	return dfa;
}

void QIDStateBuilder::_generateGraphvz(std::ofstream& outfile) const {
	std::set<StateNode*> nodeset;
	std::set<StateNode*> nodevisit;
	nodeset.insert(rootState_.get());

	while(!nodeset.empty()){
		StateNode* node = *nodeset.begin();
		nodeset.erase(node);
		nodevisit.insert(node);

		JudgeList& jlist = node->GetJudgeList();
		for(auto iter = jlist.begin(); iter != jlist.end(); iter++){
			StateNode* tmp = iter->second.second.get();
			std::string conn = node->StateNodeName() + " -> " + tmp->StateNodeName();
			conn += " [label=\"" + (iter->second.first)->JudgeStr() + "\"]; ";
			if(tmp->IsTerminal())
				outfile << tmp->StateNodeName() + "[shape=doublecircle]" << std::endl;
			outfile << conn <<std::endl;

			if(nodeset.find(tmp) == nodeset.end() && nodevisit.find(tmp) == nodevisit.end())
				nodeset.insert(tmp);
		}
	}
}

void QIDStateBuilder::GenerateGraphvz(std::string filename) const {
	std::ofstream outfile(filename); //check outfile here
	if(!outfile){
		assert(0);
	}
	outfile << "digraph G {" << std::endl;
	outfile << "node[shape = circle]" << std::endl;
	_generateGraphvz(outfile);
	outfile << "}" << std::endl;
	outfile.close();
}

void QIDStateBuilder::BuildIDState(const std::string& str) {
	const char* ptr = str.c_str();

	rootState_.reset(new StateNode); //StateNum_ is 0
	_stateTableInsert(rootState_);
	_recurStateBuild(rootState_, rootState_, CONNECTION_TYPE::CONN_SERIAL, &ptr);
	_stateSpawn();//stateTable_ will be set
	_setStateTerminal();
	
	std::shared_ptr<DFA> dfa = GenerateDFA();
	_generateDFAGraphvz(dfa);
}


class QIDStateBuilderFactory final : public IDStateBuilderFactory {
public:
	std::unique_ptr<IDStateBuilder> CreateIDStateBuilder() override {
		std::unique_ptr<IDStateBuilder> builder(new QIDStateBuilder);//builder should be checked here...
		return builder;
	}
};

FACTORY_REGISTRAR_DEFINE("QIDStateBuilderFactory", IDStateBuilder, QIDStateBuilderFactory);
