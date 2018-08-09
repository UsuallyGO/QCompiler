
#pragma once

#include "rgespecific.h"

static const char UTILITY_BLANK = RgeularSemantics::BLANK;
static const char UTILITY_ENTER = RgeularSemantics::ENTER;

std::string intToString(int i);

//con_char is the connected char between two strings
std::string setToString(const std::set<std::string>& st, char con_char);

void setUnion(std::set<std::string>& dst, const std::set<std::string>& src);

void TrimmedPrefix(std::string& str, char blank = UTILITY_BLANK, char enter = UTILITY_ENTER);

void TrimmedPostfix(std::string& str, char blank = UTILITY_BLANK, char enter = UTILITY_ENTER);

bool CharExistInString(const std::string& str, char c);

std::string CharToString(char c);

