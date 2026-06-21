#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "card.h"
#include <vector>

enum ACTION {
	FOLD,
	CHECK,
	CALL,
	RAISE
};

inline std::string action2str(const ACTION& action) {
    static const std::string actionStr[] = {"Fold", "Check", "Call", "Raise"};
    return actionStr[action];
}

struct actInfo {
	int id;
	int stateCode;
	ACTION act;
	int betAmount;

	std::ostream &output(std::ostream &out) const;
};

inline std::ostream &operator<<(std::ostream &out, const actInfo& t) {
	t.output(out);
	return out;
}

struct gameInfo {
    int stateCode;       // 轮次: 0=preflop, 1=flop, 2=turn, 3=river
    int pot;             // 底池总筹码
    int chipsToCall;     // 需要跟注的金额
    int playerCommited;  // 当前玩家本轮已投入筹码
    std::vector<ACTION> legalActions;  // 合法操作列表
};

class Player {
	template<typename> friend class Game;
protected:
    std::string name;
    std::vector<actInfo> actionHistory;
    int chips;

	void decChips(const int& amount) {chips -= amount;}
	void addChips(const int& amount) {chips += amount;}
	void setChips(const int& amount) {chips = amount; }
	virtual void setEquity(const double&) {};
	virtual ACTION makeAction(const gameInfo&, int &) = 0;

	void addActionHistory(const actInfo& act) { actionHistory.push_back(act); }
public:
	Player(const std::string &name, int startingChips): name{name}, chips{startingChips} {}
	virtual ~Player() = default;

	void setName(const std::string &newName) { name = newName; }
	int getChips() const { return chips; }
	actInfo getLastAction() const {
		if (actionHistory.size() > 0) return actionHistory.back();
		return actInfo{-1, -1, CHECK, 0};
	}
	std::string getName() const { return name; }
};


#endif