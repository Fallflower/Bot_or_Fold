#ifndef __GAME_LOG_H__
#define __GAME_LOG_H__

#include <fstream>
#include <string>
#include <ctime>

class GameLog {
    std::ofstream file_;
public:
    GameLog(const std::string& filename) {
        file_.open(filename, std::ios::app);
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        char buf[64];
        std::strftime(buf, sizeof(buf), "=== Game Log started at %Y-%m-%d %H:%M:%S ===\n\n", now);
        file_ << buf;
        file_.flush();
    }

    ~GameLog() {
        file_ << "\n=== Game Log End ===\n\n";
    }

    void write(const std::string& msg) {
        file_ << msg;
        file_.flush();
    }

    void writeLine(const std::string& msg) {
        file_ << msg << '\n';
        file_.flush();
    }

    std::ostream& stream() { return file_; }

    void flush() { file_.flush(); }
};

inline GameLog* g_log = nullptr;

#endif
