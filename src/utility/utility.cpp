
#include <string>
#include <sstream>
#include <set>
#include "utility/utility_internal.h"

std::string intToString(int i){
	std::stringstream ss;
	ss << i;
	std::string res;
	ss >> res;
	return res;
}

std::string setToString(const std::set<std::string>& st, char con_char){
	std::string res;

	if(st.empty()) return res;

	auto iter = st.begin();
	res += *iter++;

	while(iter != st.end()){
		res += con_char + *iter++;
	}

	return res;
}

void setUnion(std::set<std::string>& dst, const std::set<std::string>& src){
	for(const auto& s : src) dst.insert(s);
}

void TrimmedPrefix(std::string& str, char blank, char enter) {
	const char* ptr = str.c_str();
	while (*ptr == blank
		|| *ptr == enter)
		ptr++;
	str = str.substr(ptr - str.c_str());
}

void TrimmedPostfix(std::string& str, char blank, char enter) {
	bool prefix = true;
	const char* ptr = str.c_str() + str.length() - 1;
	while (ptr >= str.c_str()) {
		if (*ptr == blank || *ptr == enter)
			ptr--;
		else
			break;
	}
	str = str.substr(0, ptr - str.c_str() + 1);
}

bool CharExistInString(const std::string& str, char c) {
	size_t index = str.find_first_of(c);
	return index != std::string::npos;
}

std::string CharToString(char c){
	std::string str(1, c);
	return str;
}

