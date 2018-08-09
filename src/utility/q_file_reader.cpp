
#include <fstream>
#include "error.h"
#include "utility/file_reader.h"
#include "utility/file_reader_factory.h"
#include "utility/q_file_reader.h"
#include "environment/environment.h"

bool QFileReader::SetFilePath(const std::string& file) {
	if(fileOpened_) GENERATE_ERRMSG_PUSH(FILE_OPENED, this);
	else filePath_ = file;
	return !fileOpened_;
}

std::string QFileReader::GetFilePath() const {
	return filePath_;
}

bool QFileReader::OpenFile(const std::string& file){
	if(fileOpened_) {
		GENERATE_ERRMSG_PUSH(FILE_OPENED, this);
		return false;
	}
	
	filePath_ = file;
	file_.open(filePath_);
	if(!file_) {
		fileOpened_ = false;
		_resetBuffer();
		GENERATE_ERRMSG_PUSH(FILE_CANNOT_OPEN, this);
		return false;
	}
	
	fileOpened_ = true;
	ReadToBuffer();
	return true;
}

//prober_ will never stop at '\0'
char QFileReader::CurrentChar() const { return *prober_; }

char QFileReader::NextChar() {
	char c = CurrentChar();
	if (c == charEnter_ || c == charNewLine_) LineNumIncrease(), ColNumReset();
	else ColNumIncrease();
	prober_++, ReadToBuffer();
	return c;
}

void QFileReader::ReadToBuffer(){
	//only when prober_ reaches the end, we need to load buffer
	if (prober_ - buffer_ != BUFFER_SIZE()) return;

	file_.read(buffer_, BUFFER_SIZE());
	currLen_ = static_cast<size_t>(file_.gcount()); //get the real read bytes
	buffer_[currLen_] = '\0';
	prober_ = buffer_;
}

void QFileReader::_resetBuffer(){
	currLen_ = 0;
	prober_ = buffer_ + BUFFER_SIZE();
}

std::string QFileReader::TrimmedSubstrBeforeChar(char c){
	std::string sub("");
	bool done = false;
	while (!IsFileEnd() && !done) {
		if (CurrentChar() != c) sub += NextChar();
		else done = true;
	}

	const char* ptr = sub.c_str();
	while (*ptr == charBlank_) ptr++;
	sub = sub.substr(ptr - sub.c_str());
	if (sub.empty()) return sub;

	ptr = sub.c_str() + sub.length() - 1;
	while (ptr >= sub.c_str() && *ptr == charBlank_) ptr--;
	sub = sub.substr(0, ptr - sub.c_str() + 1);
	return sub;
}

std::string QFileReader::ReadLine(){
	std::string sub("");
	while(!IsFileEnd()){
		char c = NextChar();
		if (c == charEnter_ || c == charNewLine_) break;
		else sub += c;
	}
	return sub;
}

class QFileReaderFactory : public FileReaderFactory{
public:
	std::unique_ptr<FileReader> CreateFileReader(){
		std::unique_ptr<FileReader> reader(new QFileReader);
		return reader;
	}
};

FACTORY_REGISTRAR_DEFINE("QFileReader", FileReader, QFileReaderFactory);

