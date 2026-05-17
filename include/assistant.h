#ifndef __ASSISTANT_H__
#define __ASSISTANT_H__

#include <iostream>
#include <iomanip>
#include <sstream>
#include <exception>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

inline int _getch() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

inline int Pos(char ch, const char* str)			// 返回指定字符ch在字符串str中的下标。不存在时返回-1
{
	int i;
	for (i = 0; str[i] != '\0'; i++)
		if (ch == str[i])
			return i;
	return -1;
}

inline int Choice(const char* prompt, const char* options)		// 函数定义。输出提示信息prompt，输入选择的字符并返回。
{
	int key;
	std::cout << prompt << "{";
	for (int i = 0; options[i] != '\0'; i++)
	{
		if (' ' < (options[i] & 0x7f) && (options[i] & 0x7f) < 127)	// 选项options中的可打印字符
			std::cout << options[i] << ' ';
		else														// 选项options中的部分控制字符
		{
			switch (options[i])
			{
			case '\t':   std::cout << "[Tab] ";   break;					// 选项中的Tab字符'\t'（即ASCII编码9）
			case '\x1b': std::cout << "[Esc] ";   break;					// 选项中的Esc字符'\x1b'（即ASCII编码27）
			case ' ':    std::cout << "[Space] "; break;					// 选项中的空格字符' '（即ASCII编码32）
			}
		}
	}
	std::cout << "\b}: ";					// '\b'退格。先退一格，在输出右花括号
	do
	{
		key = _getch();
	} while (options[0] != '\0' && Pos(key, options) < 0); // 若options为空长度为0的字符串，则输入无限制；否则输入项应该在optins中。
	std::cout << std::endl;
	return key;
}

class Error : public std::exception
{
private:
	unsigned int m_code;
	std::string m_what;
	std::string full_mes;
public:
	Error(unsigned int code,  const std::string &what_arg): m_code(code), m_what(what_arg){
		full_mes =  "Error code: " + std::to_string(m_code) + "\nError message: " + m_what;
	}
	virtual const char * what(void) const noexcept override
	{
		// char* mesg = new char[text.length()+1];
		// strcpy(mesg, text.c_str());
		return full_mes.c_str();
	}
	unsigned int code() const { return m_code; }
};

#endif