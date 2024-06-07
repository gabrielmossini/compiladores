#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

enum TokenType {
    PAITE, ANATE, SA, KAR, BOOL, OPERADOR, RESERVADA, INVALIDO, ESPECIAL, LOGICO
};

struct Token {
    TokenType type;
    string value;
};

class Lexer {
public:
    Lexer();
    vector<Token> tokenize(const string& input);

private:
    Token makeSpecial(const string& input, size_t& pos);
    Token makeSpecialUnified(const std::string& input, size_t& pos);
    Token makeRelational(const string& input, size_t& pos);
    Token makeNumber(const string& input, size_t& pos);
    Token makeReal(const string& input, size_t start, size_t end);
    Token makeString(const string& input, size_t& pos);
    Token makeChar(const string& input, size_t& pos);
    Token makeBool(const string& input, size_t& pos);
    Token makeOperator(const string& input, size_t& pos);
    Token makeReserved(const string& input, size_t& pos);

    unordered_map<string, TokenType> reservedWords;
};

#endif // LEXER_H