
#include <cassert>
#include <stack>
#include <fstream>
#include "syntax_specific.h"
#include "utility/utility_internal.h"

const std::string ContextFreeGrammar::Epsilon = std::string(1, SyntaxSemantics::EPSILON);
const std::string ContextFreeGrammar::Finish = std::string(1, SyntaxSemantics::FINISH);

void ContextFreeGrammar::PrintGrammar() const
{
	std::cout << "StartSymbol:" << std::endl << startSymbol << std::endl;
	   
	std::cout << "Nonterminals:" << std::endl;
	if (nonTerminals.size() == 0) std::cout << "no nonterminals" << std::endl;
	else for_each(nonTerminals.begin(), nonTerminals.end(), [](const std::string& str) {
		std::cout << str << std::endl;});

		std::cout << "Terminals: " << std::endl;
		if (terminals.size() == 0) std::cout << "no terminals" << std::endl;
		else for_each(terminals.begin(), terminals.end(), [](const std::string& str) {
			std::cout << str << std::endl; });

		std::cout << "Productions: " << std::endl;
		if (productions.size() == 0) std::cout << "no productions" << std::endl;
		else for_each(productions.begin(), productions.end(), [](auto pair) {
			std::cout << pair.first;
			std::cout << " " << SyntaxSemantics::PRODUCE;
			for (const auto& str : pair.second)
				std::cout << " " << str;
			std::cout << std::endl;
	});

	std::cout << "Current nullable:" << std::endl;
	
	for (const auto& p : nullable) {
		std::cout << p.first << " : ";
		if (p.second) std::cout << "true" << std::endl;
		else std::cout << "false" << std::endl;
	}

	_printFirst();
	_printFollow();
	_printSelect();
	_printLL1Table();
}

void ContextFreeGrammar::_printLL1Table() const {
	std::cout << "LL1 predictive parsing table:" << std::endl;

	for (auto iter = ll1table.begin(); iter != ll1table.end(); iter++) {
		const std::string& nonterm = iter->first;
		const LL1Table::LL1TableRow& row = iter->second;
		for (auto iter2 = row.begin(); iter2 != row.end(); iter2++) {
			std::cout << "(" << nonterm << "," << iter2->first << "):";
			_printProduction(iter2->second);
		}
	}
}

std::string ContextFreeGrammar::_InnerNameGenerator::GenerateName(){
	std::string name = prefix + intToString(counter++);
	return name;
}

void ContextFreeGrammar::_printFFTable(const FirstTable& t) const {
	for (const auto& p : t) {
		std::cout << p.first << " : ";
		for (const auto& str : p.second)
			std::cout << str << " ";
		std::cout << std::endl;
	}
}

bool ContextFreeGrammar::_isTerminal(const std::string& term) const {
	auto iter = terminals.find(term);
	return iter != terminals.end();
}

bool ContextFreeGrammar::_isNullable(const std::string& term) const {
	auto iter = nullable.find(term);
	return iter->second;
}

/* 1. If X is terminal, then First[X] = {X}
 * 2. if X->Y1Y2Y3...Yn, for i range (1, n)
 *     if Y1-Yi-1 are all nullable 
 *			First[X] = First[X]U{First[Yi] - #}
 *     if Yi is not nullable, finish.
 *     if Y1,Y2,...Yn are all nullable
 *          Add '#' to First[X], set X is nullable
 * 3. if X->#, we should add # to First[X], but this case will be dealed in 
 *    case Y1,Y2,...Yn are all nullable
 *
 * Iterate all the operations, until there is on change in First and nullable
 * set. Pay attention that, each operation such First[X]U{First[Yi]-#} or set
 * nullable[X] may or may not changed the set. The change will be really 
 * trimmed only when new terms has been set.
 *
 * <A>-><B><C>d
 * <B>->b|#
 * <C>->c|#
 *
 * A has follow char 'd', so we should First[A] = First[A]U{First[B] - #}. 
 * There may be terminals after <B><C> so, we should not add the '#' in
 * First[B] or First[C] to First[A].
 * Only when we know each term in production are nullable, then we can add 
 * '#' to first set.
 * <A>-><B><C>
 * <B>->b|#
 * <C>->c|#
 * In this case, First[A] has '#', because both B and C are nullable.
 */
void ContextFreeGrammar::GetFirstTable(){
	for(const auto& Z : terminals) first[Z].insert(Z); //for terminal Z, first[Z] = {Z}

	bool fir_mod = false, nul_mod = false;
	do{
		fir_mod = false, nul_mod = false;
		for(auto iter = productions.begin(); iter != productions.end(); iter++){
			bool all_null = true;
			std::string X = iter->first;
			const std::vector<std::string>& Y = iter->second;
			for(int i = 0; i < Y.size() && all_null; i++){ //all_null just for accelerating
				fir_mod = first.UnionExclude(X, first[Y[i]], Epsilon) ? true : fir_mod;
				if(!_isNullable(Y[i]))	all_null = false;
			}
			if (all_null){
				nul_mod = nullable.SetValue(X, true) ? true : nul_mod;
				fir_mod = first.Insert(X, Epsilon) ? true : fir_mod;
			}
		}
	}while(fir_mod || nul_mod);
}

/* 1. startySymbol always has '$' in follow set.
 * 2. Terminals has no follow sets. 
 * 3. for X->Y1Y2Y3.Yi..Yn
 *      if Yi is terminal, we donnot care about it, continue
 *      if Yi+1 to Yn are all nullable
 *          Follow[Yi] = Follow[Yi] U Follow[X]
 *      j range from(i+1, n)
 *			if Yi+1 to Yj-1 are all nullable
 *          	Follow[Yi] = Follow[Yi] U {First[Yj] - #}
 *
 * Notice about this, follow set has no epsilon symbol '#'. The same as 'GetFirstTable'
 * once there is no really change, finish the work.
 */
void ContextFreeGrammar::GetFollowTable(){
	bool fol_mod = false, nul_mod = false;

	follow.Insert(startSymbol, Finish);
	do{
		fol_mod = false, nul_mod = false;
		for (auto iter = productions.begin(); iter != productions.end(); iter++) {
			std::string X = iter->first;
			const std::vector<std::string>& Y = iter->second;
			int k = Y.size() - 1;
			for(int i = 0; i <= k; i++){
				if (_isTerminal(Y[i]) || Y[i] == Epsilon) continue;
				if(_allNullable(Y, i + 1, k))
					fol_mod = follow.Union(Y[i], follow[X]) ? true : fol_mod;

				for(int j = i + 1; j <= k; j++)
					if (_allNullable(Y, i + 1, j - 1)) 
						fol_mod = follow.UnionExclude(Y[i], first[Y[j]], Epsilon) ? true : fol_mod;
					else break;
			}
			if (_allNullable(Y, 0, k)) nul_mod = nullable.SetValue(X, true) ? true : nul_mod;
		}
	}while(fol_mod || nul_mod);
}

/* For ElimLeftRecur() this version, it can eliminate the left recursion, but it will leave
 * duplicate productions. Some duplicate productions are needed because we need parse the file
 * from start symbol, but the rest are all useless. In this version, we will donnot try to 
 * delete all the duplicate productions.
 * 
 * How to eliminate left recursion successfully ?
 *
 * Suppose there are n nonterminals and now we are dealing with nonterm_i, we substitute all
 * the nonterminals before nonterm_i(that means nonterm_0 to nonterm_i-1). And if there is left
 * recursion now, eliminate it.
 * Pay attention about this sample:
 * <S>-><Q>c|b
 * <Q>-><R>b|b
 * <R>-><S>a|a
 * Suppose we visit all the nonterminals in order Q, R, S, we there is no left recursion until
 * we substitute both Q and R in S. This tells us that, we need to substitute all the nonterminas
 * to eliminate left recursion. If we find out that there is no left recursion when we has 
 * substituted all the nonterminals in the right part of the last nonterminal's productions, it
 * doesn't matter, because the grammar is still legal. There just are some duplicate productions
 * right now.
 *
 * Some corner cases to watch out:
 * <A>-><S>b
 * <S>-><A>a
 * For this grammar, there is a underlying left recursion, that is: <A>-><A>ab. This grammar is
 * really bad, because the sentences deduced by this grammar is endless. We need to enroll it as
 * <A><A><A><A>...ab. No matter how much times we deduce, there is always an <A> in the first 
 * position. So, this grammar is meaningless. We should not design our grammar like this. At leat,
 * we need a production that <A>->#.
 * I am not going to deal with this case in eliminating left recursion, because we do not do
 * error-check here. We will translate the grammar <A>-><A>ab as: <A1>-><A1>ab|#. Actually, this
 * is just as the same as <A>-><A>ab|#, but we will missing they symbol S in the new grammar. The
 * absence of <A>-># should be responsed to this falut.
 *
 * Notice about this: once we have elminate a left recursion, we have to add new nonterminals
 * to the grammar, so the iterator of nonterminals would be unavailable, then we need to get it 
 * again by begin() function.
 */
void ContextFreeGrammar::ElimLeftRecur(){
	auto iter_i = nonTerminals.begin();

	while(iter_i != nonTerminals.end()){
		bool eliminated = false;
		ContextFreeGrammar tmp = *this;
		for(auto iter_j = nonTerminals.begin(); iter_j != iter_i; iter_j++)
			tmp._leftSubstitue(*iter_i, *iter_j);//substitute *iter_j with its productions in *iteri's productions

		eliminated = tmp._elimImmediateLeftRecur(*iter_i);
		if(eliminated) *this = tmp, iter_i = nonTerminals.begin();
		else iter_i++;
	}
}

/* For <A>-><S>xxx, we use <S>'s productions to substitute <S> itself in this production.
 * This means there will be more productions.
 * Try to find all productions that starts with <S> and all the <A>-<S>xxx productions.
 * Use each production starts with <S> to substitute <S> in <S>xxx. This means, we need to 
 * remove <A>-><S>xxx from the production table and insert new productions that has been 
 * substituted. Because 'remove' or 'insert' will lead the iterator to be unavailable, then
 * we should remember to get the iterator again.
 */
void ContextFreeGrammar::_leftSubstitue(const std::string& A, const std::string& S){
	std::vector<std::vector<std::string>*> s_prods;
	auto iter_pair = productions.equal_range(S);
	for(auto iter = iter_pair.first; iter != iter_pair.second; iter++)
		s_prods.push_back(&iter->second);

	if(s_prods.empty()) return;//actually, cannot be empty here
	iter_pair = productions.equal_range(A);
	auto iter = iter_pair.first;
	bool substitued = false;
	while(iter != iter_pair.second){
		std::vector<std::string> vec = iter->second;
		if(vec[0] == S){
			productions.erase(iter);
			for(int i = 0; i < s_prods.size(); i++){
				std::vector<std::string> new_vec(s_prods[i]->size() + vec.size() - 1); //skip S in vec
				std::copy(s_prods[i]->begin(), s_prods[i]->end(), new_vec.begin());
				std::copy(vec.begin() + 1, vec.end(), new_vec.begin() + s_prods[i]->size());
				productions.insert(std::make_pair(A, new_vec));
			}
			iter_pair = productions.equal_range(A);
			iter = iter_pair.first;
			substitued = true;
		}
		else iter++;
	}

	/* Here, we remove the duplicate productions. The first term S in A's productions has been replaced by 
	 * S's productions, if S has not been used in anywhere else, those S's productions are duplicate. We
	 * can remove them now.
	 * Check nonterminal S is using or not: if S appears in non-S's productions, then S is using.
	 */
	if(substitued && !_nontermIsUsing(S)){ //no substitued means no S appears as the first term in A's productions
		iter_pair = productions.equal_range(S);
		productions.erase(iter_pair.first, iter_pair.second);
	}
}

/* Eliminate immediate left recursions that productions begin with term.
 *
 * Divide all the productions begin with term into two groups, one is that has immediate left
 * recursion, the other is that has no immediate left recursion. 
 * 
 * For those productions have no immediate left recursions, generate a new nonterminal(we call <term1>
 * here), then use <term1> to substitute the left part of the productions and append <term1> to the end
 * of the right part. New nonterminal will be generated by nameGenerator, this guarantees all new 
 * nonterminals have unique name.
 * 
 * Special case: if the production is <term>->#, then <term1>->#<term1> is not we want, what we want is
 * <term1>-><term1>. This means that for this case, we generate a new production that <term1> produce 
 * <term1> directly but not append <term1> after #.
 *
 * For those productions have immediate left recursions, remove the <term> that appears in the first
 * position in right part and append new nonterminal <term1> to the end.
 *
 * At last, we need to add a new production that <term1>->#
 *
 * If no immediate left recursions need to be eliminated at all, return false. This is important to judge
 * that whether all the left recursions in this grammar have been eliminated or not.
 */
bool ContextFreeGrammar::_elimImmediateLeftRecur(const std::string& term){
	auto iter_pair = productions.equal_range(term);
	std::string new_term;
	bool changed = false;

	iter_pair = productions.equal_range(term);
	auto iter = iter_pair.first;
	while(iter != iter_pair.second){
		std::vector<std::string> vec = iter->second;
		if(vec[0] == term){
			if(!changed) new_term = nameGenerator.GenerateName();
			changed = true;
			productions.erase(iter);
			std::vector<std::string> new_vec(vec.size());
			std::copy(vec.begin() + 1, vec.end(), new_vec.begin());//skip term
			new_vec[new_vec.size()-1] = new_term;
			productions.insert(std::make_pair(new_term, new_vec));

			iter_pair = productions.equal_range(term);
			iter = iter_pair.first;
		}
		else iter++;
	}
	if(!changed) return changed; //cannot find any immediate left recursions at all

	iter_pair = productions.equal_range(term);
	if (addStartSymbol) {
		/* A->Aa1|...Aan|b1|...bn, this case is that there is no b1...bn, so we need to add production that
		 * New_A->A when add start symbol, or we would never reduce with the new productions. */
		if (iter_pair.first == iter_pair.second) {
			productions.insert(std::make_pair(term, std::vector<std::string>(1, new_term)));
		}
	}
	for(auto iter = iter_pair.first; iter != iter_pair.second; iter++){
		std::vector<std::string>& vec = iter->second;
		if(vec[0] != term){
			if(vec[0] == Epsilon) vec[0] = new_term;
			else vec.push_back(new_term);
		}
	}
	
	productions.insert(std::make_pair(new_term, std::vector<std::string>(1,Epsilon)));
	nonTerminals.insert(new_term);
	return changed;
}

/* Actually, left factoring problem should be as same complicated as eliminating left recursions. However, 
 * all the books give very little explanation to it. I have emailed Doctor Chen(chenyin@hit.edu.cn, Chenyin,
 * Harbin Insititute of Technology) to disscuss this problem. I will translate some contents here.(My poor
 * English, God!)
 *
 * In all books, they just deal with the case that: the productions' first term of the right part is terminal,
 * this means, for grammar: A->ab|ac, we can do left factoring and the result is, A->aA1, A1->b|c. This OK.
 * But for gramars that has no direct left factor but underlying(indirect) left factor, what should we do?
 * For example, A->Sa|b, S->b, there is no direct left factor, but once we use S->b to replace the S in the
 * first production, there is left factor. Generally, the books haven't talked about this case and they would
 * not try to extrac the underlying left factor.
 * Chen says, the reasons may be:
 * 1. Once we try to extract all the underlying left factors, we will introduce more new nonterminals, this
 *    makes the grammar to be more complicated.
 * 2. Generally, high level programming languages' grammar are not so complicated, the underlying left factors
 *    are rarely seen.
 *
 * If we really want to extract the left factors throughly, perhaps we can do it just like eliminating left
 * recursions. Replace all the nonterminals into grammar in order and then do left factoring. I guess this
 * should work, the complexity perhaps just as same as eliminating left recursions.
 */
void ContextFreeGrammar::LeftFactoring() {
	auto iter = nonTerminals.begin();
	while(iter != nonTerminals.end()) {
		FactorPrefix leftfactor = _getLeftFactor(*iter);
		if(leftfactor.size() == 0) iter++;
		else { _leftFactoring(*iter, leftfactor), iter = nonTerminals.begin(); } //_leftFactoring may add new nonterminals
	}
}

ContextFreeGrammar::FactorPrefix
ContextFreeGrammar::_getLeftFactor(const std::string& term) const {
	FactorPrefix prefix;

	auto comparator = [](const std::vector<std::string>& v1, const std::vector<std::string>& v2)
					  -> bool {
					      std::string s1;
						  for(const auto& s : v1) s1 += s;
						  std::string s2;
						  for(const auto& s : v2) s2 += s;
						  return s1 < s2;
					  };
	std::set<std::vector<std::string>, decltype(comparator)> all_rights(comparator);//sort all the right part

	auto iter_pair = productions.equal_range(term);
	for(auto iter = iter_pair.first; iter != iter_pair.second; iter++)
		if(iter->second[0] != term) all_rights.insert(iter->second); //this is illegal, left recursion should be eliminated firstly

	for(auto iter_i = all_rights.begin(); iter_i != all_rights.end(); iter_i++){
		auto iter_j = iter_i;
		for(iter_j++; iter_j != all_rights.end(); iter_j++)
			if((*iter_i)[0] != (*iter_j)[0]) break;

		if(iter_i == --iter_j) continue; //only appear once, this production has no common prefix
		int len = _findLongestLeftFactor(*iter_i, *iter_j);
		std::vector<std::string> subprefix(len);
		std::copy(iter_i->begin(), iter_i->begin() + len, subprefix.begin());
		prefix.insert(subprefix);
		iter_i = iter_j;
	}

	return prefix;
}

void ContextFreeGrammar::_leftFactoring(const std::string& term, const FactorPrefix& prefix) {

	for(auto iter = prefix.begin(); iter != prefix.end(); iter++){

		const std::vector<std::string>& com_prefix = *iter;
		std::string new_nonterm = nameGenerator.GenerateName();
		
		auto iter_pair = productions.equal_range(term);
		auto iter_2 = iter_pair.first;
		while(iter_2 != iter_pair.second){
			const std::vector<std::string>& prods = iter_2->second;
			if(prods[0] == com_prefix[0]){
				if(prods.size() == com_prefix.size() || //prods equals to prefix, cannot access prds[com_prefix.size()] 
					prods[com_prefix.size()] != new_nonterm){
					std::vector<std::string> new_nonterm_prods(prods.size() - com_prefix.size());
					std::copy(prods.begin() + com_prefix.size(), prods.end(), new_nonterm_prods.begin());

					/* this case can appear only once, or there are duplicate productions, this is illegal*/
					if (new_nonterm_prods.size() == 0) new_nonterm_prods.push_back(Epsilon);

					productions.erase(iter_2);
					productions.insert(std::make_pair(new_nonterm, new_nonterm_prods));

					iter_pair = productions.equal_range(term);
					iter_2 = iter_pair.first;
				}
			}
			else iter_2++;
		}
		std::vector<std::string> old_nonterm_new_prods = com_prefix;
		old_nonterm_new_prods.push_back(new_nonterm);//new nonterminals;
		productions.insert(std::make_pair(term, old_nonterm_new_prods));
	}
}

int ContextFreeGrammar::_findLongestLeftFactor(const std::vector<std::string>& vec_i, 
							const std::vector<std::string>& vec_j) const {
	int len = 0;
	for(int i = 0; i < vec_i.size() && i < vec_j.size(); i++){
		if(vec_i[i] == vec_j[i]) len++;
		else break;
	}

	return len;
}
 
/* For production A->a, we first need to calculate First(a)
 * if '#'(Epsilon) is in First(a), then Select(p) = (First(a) - {Epsilon}) U Follow(A)
 * else Select(p) = First(a)
 *
 * Pay attention please, 'a' is the whole right part, not the first term. We need carefully compute
 * a's first set.
 */
void ContextFreeGrammar::GetSelectTable(){
	for(auto iter = productions.begin(); iter != productions.end(); iter++){
		const std::string& A = iter->first;
		const std::vector<std::string>& alpha = iter->second;
		std::set<std::string> st = _getSentenceFirst(alpha);
		if (st.find(Epsilon) != st.end())
			st.erase(Epsilon), setUnion(st, follow[A]);

		select.insert(std::make_pair(&*iter, st));
	}
}

std::set<std::string> ContextFreeGrammar::_getSentenceFirst(const std::vector<std::string>& sen) const {
	bool all_nullable = true;
	std::set<std::string> f;

	for(const auto& t : sen){
		setUnion(f, first.GetFirstSet(t));
		if(!nullable.GetValue(t)){ all_nullable = false; break; }
		else f.erase(Epsilon);
	}
	if(all_nullable) f.insert(Epsilon);

	return f;
}

bool ContextFreeGrammar::ConstructLL1Table(){
	bool ambigous = false;

	for(auto iter = select.begin(); iter != select.end(); iter++){
		Production* p = iter->first;
		const std::string& non_term = p->first;
		const std::set<std::string>& all_terms = iter->second;
		for(const auto& termi : all_terms){
			ambigous = ambigous ? ambigous : !ll1table.Insert(non_term, termi, p);
		}
	}
	return ambigous;
}

std::unique_ptr<SyntaxTree> ContextFreeGrammar::LL1Parsing(const std::vector<std::string>& sentence) const{
	std::stack<SyntaxNode*> st;
	std::unique_ptr<SyntaxTree> tree(new SyntaxTree);

	if(sentence.size() == 0) return tree; //null syntax tree, perhaps should never be here

	tree->GetHead()->addChild(startSymbol, SyntaxNode::NONTERMINAL);
	tree->CountIncrease();
	tree->GetHead()->addChild(Finish, SyntaxNode::FINISH);
	tree->CountIncrease();
	st.push(tree->GetHead()->getChild(1));
	st.push(tree->GetHead()->getChild(0));

	std::vector<std::string> new_sen = sentence;
	new_sen.push_back(Finish);
	
	auto iter = new_sen.begin();
	
	while(!st.empty() && iter != new_sen.end()){
		SyntaxNode* curNode = st.top();
		st.pop();
		if(curNode->getTerm() == Epsilon) continue;

		std::string term = *iter;
		if(term == curNode->getTerm()) { 
			{ iter++; continue; }
		}
		
		Production* p = ll1table.Parse(curNode->getTerm(), term);
		assert(p != nullptr);
		
		for(auto iter2 = p->second.begin(); iter2 != p->second.end(); iter2++){
			SyntaxNode::NodeType t = _isTerminal(*iter2) ? SyntaxNode::TERMINAL : SyntaxNode::NONTERMINAL;
			curNode->addChild(*iter2, t);
			tree->CountIncrease();
		}

		for(int i = p->second.size() - 1; i >= 0; i--){ //reverse order to stack
			SyntaxNode* node = curNode->getChild(i);
			st.push(node);
		}
	}

	if(st.empty() && iter == new_sen.end()) tree->Accepted();
	else tree->NotAccepted();

	return tree;
}

bool ContextFreeGrammar::_nontermIsUsing(const std::string& term) const {
	for(auto iter = productions.begin(); iter != productions.end(); iter++){
		if(iter->first == term) continue; //term uses itself, ignore this case
		const std::vector<std::string>& right = iter->second;
		for(const auto& s : right){
			if(s == term) return true;
		}
	}
	return false;
}

bool ContextFreeGrammar::_allNullable(const std::vector<std::string>& Y, int beg, int end) {
	bool b = true;
	while (b && beg <= end)//beg > end returns true, this is useful for get first, get follow
		if (nullable[Y[beg]]) beg++;
		else b = false;
	return b;
}

bool FirstTable::Union(const std::string& term, const std::set<std::string>& st) {
	changed = false;
	std::set<std::string>& this_set = table[term];
	for (const auto& str : st) {
		if (this_set.find(str) == this_set.end()) {
			changed = true;
			this_set.insert(str);
		}
	}
	return changed;
}

bool FirstTable::UnionExclude(const std::string& term, const std::set<std::string>& st, 
								const std::string& exclude){
	changed = false;
	std::set<std::string>& this_set = table[term];
	for(const auto& str : st){
		if(str != exclude && this_set.find(str) == this_set.end()){
			changed = true;
			this_set.insert(str);
		}
	}
	return changed;
}

bool FirstTable::Insert(const std::string& term, const std::string& str) {
	changed = false;
	std::set<std::string>& this_set = table[term];
	if(this_set.find(str) == this_set.end())
		this_set.insert(str), changed = true;
	return changed;
}

std::set<std::string> FirstTable::GetFirstSet (const std::string& term) {
	return (*this)[term];
}

std::set<std::string>& FirstTable::operator[](const std::string& term) {
	return table[term];
}

std::set<std::string> FirstTable::GetFirstSet(const std::string& term) const{
	auto iter = table.find(term);
	return iter->second;
}

void SyntaxTree::GenerateGraphvz(const std::string& filename) {
	std::ofstream outfile(filename); //check outfile here
	if(!outfile){
		assert(0);
	}
	outfile << "digraph G {" << std::endl;
	outfile << "node[shape = plaintext]" << std::endl;
	_generateGraphvz(outfile);
	outfile << "}" << std::endl;
	outfile.close();
}

void SyntaxTree::_generateGraphvz(std::ofstream& outfile) {
	std::set<SyntaxNode*> st;
	std::map<SyntaxNode*, std::string> mp;
	int counter = 0;

	st.insert(head.get());
	mp.insert(std::make_pair(head.get(), head->getTerm()));
	while(!st.empty()){
		auto iter = st.begin();
		SyntaxNode* node = *iter;
		st.erase(iter);

		for(int i = 0; i < node->children.size(); i++){
			std::string from = mp[node];
			std::string to  = node->getChild(i)->getTerm();
			std::string inner_to = to + intToString(counter++);
			mp.insert(std::make_pair(node->getChild(i), inner_to));
			outfile << "\"" + inner_to + "\"[label = \"" + to + "\"]" << std::endl;
			outfile << "\"" + from + "\"-> \"" + inner_to + "\"" << std::endl;
			
			st.insert(node->getChild(i));
		}
	}
}