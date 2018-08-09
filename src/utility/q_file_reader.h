
#pragma once

#include <fstream>
#include "utility/file_reader.h"
#include "environment/environment.h"
#include "sys_env.h"

class QFileReader final : public FileReader {
public:
	~QFileReader(){ if(fileOpened_) file_.close(); }
	
	bool SetFilePath(const std::string& file) override;

	std::string GetFilePath() const override;

	bool OpenFile(const std::string& file) override;

	char CurrentChar() const override;

	char NextChar() override;

	//There is a bug in g++ for 'constexpr', perhaps the newest g++ has fixed it up, but we cannot
	//be so strictly for the g++ version. So, comment it.
	/*constexpr*/ size_t BUFFER_SIZE() const { return BUFFER_SIZE_ - 1; }

	bool IsFileOpened() const { return fileOpened_; }

	bool IsFileEnd() const {
		bool b = currLen_ == BUFFER_SIZE() ? true : false;
		if (!b && _bufferEnd()) return true;
		else return false;
	}

	void ReadToBuffer();

	//substring before 'c' that trimmed blank
	std::string TrimmedSubstrBeforeChar(char c);

	std::string ReadLine();

private:

	void _resetBuffer();

	//buffer end is not the file end
	bool _bufferEnd() const { return *prober_ == '\0'; }
	
	Environment* env_{ nullptr };

	std::string filePath_;
	bool fileOpened_{ false };
	std::fstream file_;

	const static int BUFFER_SIZE_{ 8192 };
	char buffer_[BUFFER_SIZE_]{ '\0' };
	size_t currLen_{ 0 };

	char* prober_{ buffer_ + BUFFER_SIZE() }; //points to the last, means that there is no available data right now

	const static char charBlank_{ ' ' };
	const static char charEnter_{ _ENTER_ };
	const static char charNewLine_{ _NEWLINE_ };
};

