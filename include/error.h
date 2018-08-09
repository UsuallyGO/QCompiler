#pragma once

#include <array>
#include <vector>
#include "rgeanalyzier.h"

struct ErrMsg {
	int errNum;
	const char* msg;

	std::string fileName;
	size_t lineNum;
	size_t colNum;

	ErrMsg(int n, const char* p, const std::string& f, size_t l, size_t c) :
		errNum(n), msg(p), fileName(f), lineNum(l), colNum(c) {}
	ErrMsg(ErrMsg&& e) {
		errNum = e.errNum;
		msg = e.msg;
		lineNum = e.lineNum;
		colNum = e.colNum;
		fileName = move(e.fileName);
	}
};

enum Error_Number : int {
//Errors for FileReader
	FILE_CANNOT_OPEN = 0,
	FILE_OPENED,

	INCOMPLETE_COMMENT,
	MISSING_CHAR,
	ILLEGAL_IDENTITY,
	INCOMPLETE_DEFINITION,
	ILLEGAL_DEFINITION,
	MULTIPLE_DEFINITION,
	ILLEGAL_ID_DEFINITION,

	TOTAL_ERROR //use for count
};

std::vector<ErrMsg>& GetAllErrors();
void PushError(ErrMsg&& emsg);
bool PrintAllErrors();
void ErrorPrint(const ErrMsg& emsg);
extern std::array<const char*, TOTAL_ERROR> Error_Message;

#define GENERATE_ERRMSG_PUSH(error, reader) do{ \
										ErrMsg errmsg{error, Error_Message[error], \
										reader->GetFilePath(), reader->LineNumber(), reader->ColNumber() }; \
										PushError(std::move(errmsg)); \
									}while(0)

