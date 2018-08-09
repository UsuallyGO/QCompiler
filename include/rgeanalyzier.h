#pragma once

#include <memory>
#include <string>
#include "rgespecific.h"

class RgeAnalyzier {
public:
	RgeAnalyzier(){}
	virtual ~RgeAnalyzier(){}
	virtual RGEDomainSpecific* RgeAnalyse() = 0;
	virtual bool OpenFile(const std::string&) = 0;
};

std::unique_ptr<RgeAnalyzier> CreateRgeAnalyzier(const std::string& analyzier_name);
