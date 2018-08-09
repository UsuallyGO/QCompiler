
#pragma once

#include "factory_template.h"

class FileReader {
public:
	FileReader(){}
	virtual ~FileReader(){}
	virtual bool OpenFile(const std::string& file) = 0;
	virtual char CurrentChar() const = 0;
	virtual char NextChar() = 0;

	virtual bool SetFilePath(const std::string& file) = 0;
	virtual std::string GetFilePath() const = 0;

	void LineNumIncrease() { lineNumber_++; }
	void ColNumIncrease() { colNumber_++; }
	void LineNumReset() { lineNumber_ = 0; }
	void ColNumReset() { colNumber_ = 0; }

	size_t LineNumber() const { return lineNumber_; }
	size_t ColNumber() const { return colNumber_; }

private:
	size_t lineNumber_ { 1 };//lineNumber begins from 1 but not 0, this is more friendly to read
	size_t colNumber_ { 0 };
};

