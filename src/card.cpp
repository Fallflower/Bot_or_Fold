#include "card.h"

const std::map<char, SUIT> smap = {
    {'H', SUIT::HEA},
    {'C', SUIT::CLU},
    {'D', SUIT::DIA},
    {'S', SUIT::SPA}
};

const std::map<char, CARDNUM> numap = {
    {'2', CARDNUM::NUM_2},
    {'3', CARDNUM::NUM_3},
    {'4', CARDNUM::NUM_4},
    {'5', CARDNUM::NUM_5},
    {'6', CARDNUM::NUM_6},
    {'7', CARDNUM::NUM_7},
    {'8', CARDNUM::NUM_8},
    {'9', CARDNUM::NUM_9},
    {'T', CARDNUM::NUM_10},
    {'J', CARDNUM::JACK},
    {'Q', CARDNUM::QUEEN},
    {'K', CARDNUM::KING},
    {'A', CARDNUM::ACE}
};

const std::string num2str(const CARDNUM& c) {
    switch (c)
    {
    case CARDNUM::NUM_2: return "2";
    case CARDNUM::NUM_3: return "3";
    case CARDNUM::NUM_4: return "4";
    case CARDNUM::NUM_5: return "5";
    case CARDNUM::NUM_6: return "6";
    case CARDNUM::NUM_7: return "7";
    case CARDNUM::NUM_8: return "8";
    case CARDNUM::NUM_9: return "9";
    case CARDNUM::NUM_10: return "T";
    case CARDNUM::JACK: return "J";
    case CARDNUM::QUEEN: return "Q";
    case CARDNUM::KING: return "K";
    case CARDNUM::ACE: return "A";
    }
    return "";
}

const std::string num2str(const SHORT_CARDNUM& c) {
    switch (c)
    {
    case SHORT_CARDNUM::NUM_6: return "6";
    case SHORT_CARDNUM::NUM_7: return "7";
    case SHORT_CARDNUM::NUM_8: return "8";
    case SHORT_CARDNUM::NUM_9: return "9";
    case SHORT_CARDNUM::NUM_10: return "T";
    case SHORT_CARDNUM::JACK: return "J";
    case SHORT_CARDNUM::QUEEN: return "Q";
    case SHORT_CARDNUM::KING: return "K";
    case SHORT_CARDNUM::ACE: return "A";
    }
    return "";
}

const std::string suit2str(const SUIT& s) {
    switch (s) {
    case SUIT::HEA: return "♥";
    case SUIT::CLU: return "♣";
    case SUIT::DIA: return "♦";
    case SUIT::SPA: return "♠";
    }
    return "";
}
