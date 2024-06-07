#include <cctype>
#include <iostream>
#include <cstring>

#include "lexer.h"

using namespace std;

Lexer::Lexer() {
    reservedWords = {
        {"tuichaveva", RESERVADA}, {"ramoramo", RESERVADA}, {"upeicharo", RESERVADA},
        {"neiramo", RESERVADA}, {"upeaja", RESERVADA}, {"guara", RESERVADA},
        {"kjaike", RESERVADA}, {"koseva", RESERVADA}
    };
}

vector<Token> Lexer::tokenize(const string& input) {
    vector<Token> tokens;
    size_t pos = 0;

    while (pos < input.size()) {
        if (isspace(input[pos])) {
            pos++; // Ignorar espaços em branco
        } else if (isdigit(input[pos])) {
            tokens.push_back(makeNumber(input, pos));
        } else if (isalpha(input[pos])) {
            string word;
            size_t start = pos;
            while (pos < input.size() && isalpha(input[pos])) {
                word += input[pos++];
            }
            if (reservedWords.find(word) != reservedWords.end()) {
                tokens.push_back({RESERVADA, word});
            } else {
                if (word == "paite" || word == "anate" || word == "sa" || word == "kar" || word == "bool") {
                    tokens.push_back({RESERVADA, word});
                } else {
                    tokens.push_back({INVALIDO, word});
                }
            }
        } else if (input[pos] == '!' && input[pos + 1] == '"') {
            tokens.push_back(makeString(input, pos));
        } else if (input[pos] == '\'' && pos + 2 < input.size() && input[pos + 2] == '\'') {
            tokens.push_back(makeChar(input, pos));
        } else if (input.substr(pos, 4) == "true" || input.substr(pos, 5) == "false") {
            tokens.push_back(makeBool(input, pos));
        } else if (strchr("+-*/%", input[pos]) || input.substr(pos, 2) == "->" || input.substr(pos, 2) == "==" || input.substr(pos, 2) == "=>" || input.substr(pos, 2) == "=<" || input.substr(pos, 2) == "&!") {
            tokens.push_back(makeOperator(input, pos));
        } else if (strchr("><=!", input[pos])) {
            tokens.push_back(makeRelational(input, pos));
        } else if ((input[pos] == '!' && (input[pos + 1] == '(' || input[pos + 1] == '{' || input[pos + 1] == '!')) || 
                   (input[pos] == ')' && input[pos + 1] == '!') || 
                   (input[pos] == '}' && input[pos + 1] == '!')) {
            tokens.push_back(makeSpecialUnified(input, pos));
        } else if (strchr(";(){}[]:,", input[pos])) {
            tokens.push_back(makeSpecial(input, pos));
        } else {
            string invalid(1, input[pos]);
            tokens.push_back({INVALIDO, invalid});
            pos++;
        }
    }
    
    return tokens;
}

Token Lexer::makeSpecial(const string& input, size_t& pos) {
    return {ESPECIAL, string(1, input[pos++])};
}

Token Lexer::makeSpecialUnified(const string& input, size_t& pos) {
    string special;
    if (input[pos] == '!' && (input[pos + 1] == '(' || input[pos + 1] == '{')) {
        special = input.substr(pos, 2);
        pos += 2;
    } else if ((input[pos] == ')' && input[pos + 1] == '!') || (input[pos] == '}' && input[pos + 1] == '!')) {
        special = input.substr(pos, 2);
        pos += 2;
    }
    return {ESPECIAL, special};
}


Token Lexer::makeRelational(const string& input, size_t& pos) {
    string relOp(1, input[pos++]);
    if (pos < input.size() && (input[pos - 1] == '>' || input[pos - 1] == '<' || input[pos - 1] == '=' || input[pos - 1] == '!')) {
        if (input[pos] == '=') {
            relOp += input[pos++];
        }
    }
    return {OPERADOR, relOp};
}

Token Lexer::makeNumber(const string& input, size_t& pos) {
    size_t start = pos;
    bool isReal = false;

    while (pos < input.size() && (isdigit(input[pos]) || input[pos] == '.')) {
        if (input[pos] == '.') {
            isReal = true;
        }
        pos++;
    }

    if (isReal) {
        return makeReal(input, start, pos);
    } else {
        return {PAITE, input.substr(start, pos - start)};
    }
}

Token Lexer::makeReal(const string& input, size_t start, size_t end) {
    return {ANATE, input.substr(start, end - start)};
}

Token Lexer::makeString(const string& input, size_t& pos) {
    if (input[pos] != '!' || pos + 1 >= input.size() || input[pos + 1] != '"') {
        // Não é uma string válida (deve começar com !")
        pos++;  // Avança para evitar loop infinito
        return {INVALIDO, ""};
    }

    size_t start = pos + 2; // Pular !" (2 caracteres)
    pos += 2;

    // Procurar pelo delimitador final "!
    while (pos + 1 < input.size() && !(input[pos] == '"' && input[pos + 1] == '!')) {
        pos++;
    }

    if (pos + 1 >= input.size() || input[pos] != '"' || input[pos + 1] != '!') {
        // String incompleta ou malformada
        return {INVALIDO, ""};
    }

    // String válida, extraí-la
    string str = input.substr(start, pos - start);
    
    // Pular o delimitador final "!
    pos += 2;

    return {SA, str};
}


/*
Código Original
Token Lexer::makeString(const string& input, size_t& pos) {
    size_t start = pos + 2; // Pular !"
    pos += 2;
    while (pos < input.size() && (input[pos] != '"' || input[pos - 1] == '\\')) {
        pos++;
    }
    pos++; // Pular o "
    return {SA, input.substr(start, pos - start - 1)};
}*/


Token Lexer::makeChar(const string& input, size_t& pos) {
    size_t start = pos + 1; // Pular '
    pos += 3; // Pular 'x'
    return {KAR, input.substr(start, 1)};
}

Token Lexer::makeBool(const string& input, size_t& pos) {
    size_t start = pos;
    if (input.substr(pos, 4) == "true") {
        pos += 4;
        return {BOOL, "true"};
    } else {
        pos += 5;
        return {BOOL, "false"};
    }
}

Token Lexer::makeOperator(const string& input, size_t& pos) {
    string op(1, input[pos++]);
    if (pos < input.size() && (op == "-" || op == "=" || op == "&")) {
        if (input[pos] == '>' || input[pos] == '=' || input[pos] == '!') {
            op += input[pos++];
        }
    }
    return {OPERADOR, op};
}