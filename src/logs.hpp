#pragma once
#include <iostream>
#include <fstream>
#include <streambuf>

enum class LogMode { Both, ConsoleOnly, FileOnly };

class TeeBuf : public std::streambuf {
	std::streambuf* console;
	std::ofstream file;
	LogMode mode = LogMode::Both;
	std::string ansiBuffer;
	bool inEscapeCode = false;
	std::string file_path;

	void writeToFile(char c) {
		if (c == '\033') {
			inEscapeCode = true;
			return;
		}
		if (inEscapeCode) {
			if (c == 'm') inEscapeCode = false;
			return;
		}
		file.put(c);
	}

	protected:
	int overflow(int c) override {
		if (c == EOF) return !EOF;
		if (mode == LogMode::Both || mode == LogMode::ConsoleOnly)
			console->sputc(c);
		if (mode == LogMode::Both || mode == LogMode::FileOnly)
			writeToFile(c);
		return c;
	}

	public:
	TeeBuf(const std::string& filename) : console(std::cout.rdbuf()),
		file(filename, std::ios::app), file_path(filename) {}

	~TeeBuf() {
		std::cout.rdbuf(console);
	}

	void clear() {
        file.close();
        std::ofstream(file_path, std::ios::trunc); // wipe the file
        file.open(file_path, std::ios::app);        // reopen for appending
    }

	void setMode(LogMode m) { mode = m; }
};
