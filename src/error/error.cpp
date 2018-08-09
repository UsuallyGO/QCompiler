
#include <iostream>
#include <string>
#include <vector>
#include "error.h"
#include "rgeanalyzier.h"

std::vector<ErrMsg>& GetAllErrors() {
	static std::vector<ErrMsg> allErrors;
	return allErrors;
}

std::array<const char*, TOTAL_ERROR> Error_Message = {
	"Cannot open file",
	"Incomplete multiple comments",
	"Cannot find specific char in file",
	"Illegal identity",
	"Incomplete definition",
	"Illegal definition",
	"Multiple definitions",
	"Illegal ID definition",
	
//Errors for FileReader
	"File has been already opened",
};

void PushError(ErrMsg&& emsg) {
	GetAllErrors().push_back(std::move(emsg));
}

//return 'true' if there is any error, or return 'false' if there is no error
bool PrintAllErrors() {
	std::vector<ErrMsg>& allerr = GetAllErrors();
	size_t ind = 0;
	for (; ind < allerr.size(); ind++)
		ErrorPrint(allerr[ind]);
	return ind != 0;
}

void ErrorPrint(const ErrMsg& emsg) {
	std::cout << "Error in " << emsg.fileName << " file, " << emsg.lineNum;
	std::cout << " line, " << emsg.colNum << " column: ";
	std::cout << emsg.msg << std::endl;
}
