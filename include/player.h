#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "card.h"
#include <vector>

enum ACTION {
	FOLD,
	CHECK,
	CALL,
	BET,
	RAISE,
	ALLIN,
	ALLINTOCALL
};

inline std::string action2str(const ACTION&);

class Player {
	friend class Game;
protected:
    std::string name;
    std::vector<Card> hand;
    int chips;

	void decChips(const int& amount) {chips -= amount;}
	void addChips(const int& amount) {chips += amount;}
	void setChips(const int& amount) {chips = amount; }
	void setHand(const std::vector<Card> &cards);
	virtual ACTION makeAction(const int&, int &) = 0;
public:
	Player(const std::string &name, int startingChips);
	virtual ~Player() = default;

	void setName(const std::string &newName) { name = newName; }
	int getChips() const { return chips; }
	std::string getName() const { return name; }
	std::vector<Card> getHand() const { return hand; }
};


#endif