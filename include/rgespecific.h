#pragma once

#include <memory>
#include <string>

#include "sys_env.h"

class RGEDomainImpl;

#define RGEDOMAINSPECIFIC_INSERT_DECLARE(d) bool d##Insert(const std::string& str)
#define RGEDOMAINSPECIFIC_SET_DECLARE(d) void d##Set(const std::string& str)

class RGEDomainSpecific {
public:
	RGEDomainSpecific();
	~RGEDomainSpecific();
	RGEDOMAINSPECIFIC_INSERT_DECLARE(Keywords);
	RGEDOMAINSPECIFIC_INSERT_DECLARE(Operators);
	RGEDOMAINSPECIFIC_INSERT_DECLARE(Symbols);
	RGEDOMAINSPECIFIC_INSERT_DECLARE(Datatypes);
	RGEDOMAINSPECIFIC_SET_DECLARE(ID);
	RGEDOMAINSPECIFIC_SET_DECLARE(LineComment);
	RGEDOMAINSPECIFIC_SET_DECLARE(MulcomBegin);
	RGEDOMAINSPECIFIC_SET_DECLARE(MulcomEnd);

	void DisplayRGEDomainSpecific() const;
private:
	std::unique_ptr<RGEDomainImpl> impl_;
};

enum class RGEDefinition_Type {
	KEYWORDS, SYMBOLS, OPERATORS,
	DATATYPE, ID, COMMENTS
};

struct RgeularSemantics {
	static const char LINE_COM{ '#' };
	static const char MUL_COM_BEGIN{ '!' };
	static const char MUL_COM_END{ '!' };
	static const char BLANK{ ' ' };
	static const char ENTER{ _ENTER_ };
	static const char NEWLINE{ _NEWLINE_ };
	static const char ASSIGN_OPERATOR{ '=' };
	static const char DEFINITION_BEGIN{ '{' };
	static const char DEFINITION_END{ '}' };
	static const char DEFINITION_INTERVAL{ ',' };
	static const std::string KEYWORDS;
	static const std::string SYMBOLS;
	static const std::string OPERATORS;
	static const std::string DATATYPES;
	static const std::string ID;
};

std::string RGEDefinition_Type_String(RGEDefinition_Type type);

