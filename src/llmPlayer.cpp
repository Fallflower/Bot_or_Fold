#include "llmPlayer.h"
#include "openai/openai.hpp"
#include "assistant.h"
#include "gameLog.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <sstream>

std::string doubleToString(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

LLMPlayer::LLMPlayer(const std::string &name, int startingChips)
    : Player(name, startingChips) {
    openai::start("sk-lm-LehWfNh9:ZPHHLwYE6t4nZFcQ1ZLb", "", true, "http://100.67.63.50:1234/v1/");
    g_log->writeLine("[LLMPlayer] OpenAI initialized (custom endpoint)");
}

std::string LLMPlayer::buildPrompt(const gameInfo& info) const {
    const std::string stateStr[] = {"preflop", "flop", "turn", "river", "end"};
    std::string street = stateStr[info.stateCode];

    std::string legalStr;
    for (size_t i = 0; i < info.legalActions.size(); i++) {
        if (i > 0) legalStr += ", ";
        legalStr += action2str(info.legalActions[i]);
    }

    int minRaise = (info.chipsToCall > 0) ? info.chipsToCall * 2 : 1;

    std::string prompt;
    prompt += "Game State:\n";
    prompt += "- Street: " + street + "\n";
    prompt += "- Pot: " + std::to_string(info.pot) + "\n";
    prompt += "- Opening chips (current bet to match): " + std::to_string(info.chipsToCall) + "\n";
    prompt += "- Your hand: " + info.handCardsStr + "\n";
    prompt += "- Hand strength: " + info.handTypeStr + "\n";
    prompt += "- Your remaining chips: " + std::to_string(chips) + "\n";
    prompt += "- Your equity (win probability): " + doubleToString(info.winRate, 2) + "%\n";
    prompt += "- You committed this round: " + std::to_string(info.playerCommited) + "\n";
    prompt += "- Legal actions: " + legalStr + "\n";
    prompt += "- Minimum raise amount: " + std::to_string(minRaise) + " (additional chips)\n";
    return prompt;
}

ACTION LLMPlayer::makeAction(const gameInfo& info, int &betAmount) {
    try {
        std::string systemPrompt =
            "You are a professional Texas Hold'em poker AI. Choose the best action given the game state.\n"
            "Respond ONLY with a JSON object in this exact format, no other text:\n"
            "{\n"
            "  \"action\": \"Fold\",\n"
            "  \"bet_amount\": 0\n"
            "}\n"
            "All possible actions: Fold, Check, Call, Raise.\n"
            "Rules for bet_amount:\n"
            "- Fold, Check, Call: bet_amount = 0\n"
            "- Raise: bet_amount is the additional chips you put in (min raise amount is specified)";

        std::string userPrompt = buildPrompt(info);

        openai::Json input = {
            {"model", "AUTO_DETECT"},
            {"messages", {
                {{"role", "system"}, {"content", systemPrompt}},
                {{"role", "user"},   {"content", userPrompt}}
            }},
            {"temperature", 0.7},
            {"max_tokens", 8192}
        };

        std::cerr << "[LLMPlayer] " << name << " is consulting AI..." << std::endl;
        auto res = openai::chat().create(input);
        g_log->writeLine("[LLMPlayer] Full response: " + res.dump(2));
        std::string reply = res["choices"][0]["message"]["content"];
        g_log->writeLine("[LLMPlayer] Extracted reply: '" + reply + "'");

        // 解析JSON响应
        auto responseJson = openai::Json::parse(reply);

        std::string action = responseJson["action"];
        betAmount = responseJson.value("bet_amount", 0);

        // 校验映射到ACTION枚举
        if (action == "Fold") {
            return FOLD;
        }
        if (action == "Check") {
            return CHECK;
        }
        if (action == "Call") {
            // call的筹码扣除由toAct中的dispatch统一处理
            return CALL;
        }
        if (action == "Raise") {
            int minRaise = (info.chipsToCall > 0) ? info.chipsToCall * 2 : 1;
            if (betAmount < minRaise) betAmount = minRaise;
            if (betAmount >= chips) {
                betAmount = chips;
                setChips(0);
            } else {
                decChips(betAmount);
            }
            return RAISE;
        }

        // 未知action，默认安全策略
        throw Error(5, "Unknown action: " + action + ", defaulting to fold");

    } catch (const std::exception& e) {
        g_log->writeLine("[LLMPlayer] Error: " + std::string(e.what()));
        // 出错时选安全动作
        for (auto act : info.legalActions) {
            if (act == CHECK) return CHECK;
            if (act == CALL) return CALL;
        }
        return FOLD;
    }
}
