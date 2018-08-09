
#include <set>
#include <string>
#include <iostream>
#include "rgespecific.h"

#define RGEDOMAIN_SET_INSERT(d) bool d##Insert(const std::string& str) \
							  { \
								bool res = false; \
								if(d##_.find(str) == d##_.end()) \
									res = d##_.insert(str).second; \
								return res; \
							  }

#define RGEDOMAIN_SET_GET(d) const std::set<std::string>& d() const \
						  {\
								return d##_; \
						  }

#define RGEDOMAIN_STRING_GET(d) const std::string& d() const \
							 {\
									return d##_; \
							 }

#define RGEDOMAIN_STRING_SET(d) void d##Set(const std::string& str) \
							 {\
									d##_ = str; \
							 }

class RGEDomainImpl final {
public:
	RGEDOMAIN_SET_INSERT(keywords)
	RGEDOMAIN_SET_INSERT(symbols)
	RGEDOMAIN_SET_INSERT(operators)
	RGEDOMAIN_SET_INSERT(datatypes)

	/*DOMAIN_XXX_GET function return as reference, but it is forbidded to change the contents
	  through the return value.
	  The right way is to use DOAMIN_INSERT function.
	*/
	RGEDOMAIN_SET_GET(keywords)
	RGEDOMAIN_SET_GET(symbols)
	RGEDOMAIN_SET_GET(operators)
	RGEDOMAIN_SET_GET(datatypes)

	RGEDOMAIN_STRING_SET(id)
	RGEDOMAIN_STRING_SET(line_comment)
	RGEDOMAIN_STRING_SET(mulcom_begin)
	RGEDOMAIN_STRING_SET(mulcom_end)

	RGEDOMAIN_STRING_GET(id)
	RGEDOMAIN_STRING_GET(line_comment)
	RGEDOMAIN_STRING_GET(mulcom_begin)
	RGEDOMAIN_STRING_GET(mulcom_end)

private:
	std::set<std::string> keywords_{};
	std::set<std::string> symbols_{};
	std::set<std::string> operators_{};
	std::set<std::string> datatypes_{};
	std::string id_;
	std::string line_comment_;
	std::string mulcom_begin_;
	std::string mulcom_end_;
};

#undef RGEDOMAIN_SET_INSERT
#undef RGEDOMAIN_SET_GET
#undef RGEDOMAIN_STRING_GET
#undef RGEDOMAIN_STRING_SET

RGEDomainSpecific::RGEDomainSpecific() : impl_(new RGEDomainImpl) {}
RGEDomainSpecific::~RGEDomainSpecific() {}

#define RGEDOMAINSPECIFIC_INSERT(d, _d) bool RGEDomainSpecific::d##Insert(const std::string& str) \
									 {\
										return impl_->_d##Insert(str); \
									 }

RGEDOMAINSPECIFIC_INSERT(Keywords, keywords)
RGEDOMAINSPECIFIC_INSERT(Operators, operators)
RGEDOMAINSPECIFIC_INSERT(Symbols, symbols)
RGEDOMAINSPECIFIC_INSERT(Datatypes, datatypes)

#define RGEDOMAINSPECIFIC_STRING_SET(d, _d) void RGEDomainSpecific::d##Set(const std::string& str) \
										 {\
											return impl_->_d##Set(str); \
										 }

RGEDOMAINSPECIFIC_STRING_SET(ID, id)
RGEDOMAINSPECIFIC_STRING_SET(LineComment, line_comment)
RGEDOMAINSPECIFIC_STRING_SET(MulcomBegin, mulcom_begin)
RGEDOMAINSPECIFIC_STRING_SET(MulcomEnd, mulcom_end)

#undef RGEDOMAINSPECIFIC_INSERT
#undef RGEDOMAINSPECIFIC_STRING_SET

void RGEDomainSpecific::DisplayRGEDomainSpecific() const {
	std::cout << "Keywords:" << std::endl;
	for (auto i : impl_->keywords()) std::cout << i << std::endl;
	
	std::cout << "Symbols:" << std::endl;
	for (auto i : impl_->symbols()) std::cout << i << std::endl;

	std::cout << "Operators:" << std::endl;
	for (auto i : impl_->operators()) std::cout << i << std::endl;

	std::cout << "Datatypes:" << std::endl;
	for (auto i : impl_->datatypes()) std::cout << i << std::endl;

	std::cout << "ID definition:" << impl_->id() << std::endl;
	std::cout << "Line comment:" << impl_->line_comment() << std::endl;
	std::cout << "Multi-line comments begin:" << impl_->mulcom_begin() << std::endl;
	std::cout << "Multi-line comments end:" << impl_->mulcom_end() << std::endl;
}

const std::string RgeularSemantics::KEYWORDS = "keywords";
const std::string RgeularSemantics::SYMBOLS = "symbols";
const std::string RgeularSemantics::OPERATORS = "operators";
const std::string RgeularSemantics::DATATYPES = "datatypes";
const std::string RgeularSemantics::ID = "ID";

std::string RGEDefinition_Type_String(RGEDefinition_Type type) {
	switch (type) {
	case RGEDefinition_Type::KEYWORDS:
		return RgeularSemantics::KEYWORDS;
	case RGEDefinition_Type::OPERATORS:
		return RgeularSemantics::OPERATORS;
	case RGEDefinition_Type::SYMBOLS:
		return RgeularSemantics::SYMBOLS;
	case RGEDefinition_Type::DATATYPE:
		return RgeularSemantics::DATATYPES;
	case RGEDefinition_Type::ID:
		return RgeularSemantics::ID;
	default:
		return std::string("Unknown");
	}
}