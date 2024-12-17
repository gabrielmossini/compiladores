#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

// Mapping keywords
std::unordered_map<std::string, std::string> keywords = {
    {"tuichaveva", "PROGRAM_START"},
    {"paha", "PROGRAM_END"},
    {"ramoramo", "IF_CONDITIONAL"},
    {"neiramo", "ELSE_CONDITIONAL"},
    {"upe_aja", "WHILE_LOOP"},
    {"guara", "FOR_LOOP"},
    {"kjaike", "DATA_INPUT"},
    {"koseva", "DATA_OUTPUT"},
    {"paite", "TYPE"},
    {"anate", "TYPE"},
    {"bool", "TYPE"},
    {"sa", "TYPE"},
    {"kar", "TYPE"}
};

// Mapping operators
std::unordered_map<std::string, std::string> operators = {
    {"==", "EQUALS"},
    {"=!=", "DIFFERENT"},
    {">=", "GREATER_EQUAL"},
    {"<=", "LESSER_EQUAL"},
    {"&", "AND"},
    {"&!", "OR"},
    {"+", "SUM"},
    {"-", "SUBTRACT"},
    {"*", "MULTIPLY"},
    {"%", "DIVIDE"},
    {"->", "ASSIGN"},
    {">", "GREATER"},
    {"<", "LESS"},
};

// Mapping symbols
std::unordered_map<std::string, std::string> symbols = {
    {"!(", "LEFT_PAREN"},
    {")!", "RIGHT_PAREN"},
    {"!{", "BLOCK_START"},
    {"}!", "BLOCK_END"},
    {";", "COMMAND_END"},
    {"->", "ASSIGN"},
    {"!\"", "QUOTE_MARK"},
    {",", "ARGUMENT_SEPARATOR"},
};

int errorCount = 0;

bool isIdentifier(const std::string& token) {
    if (token.empty() || !std::isalpha(token[0]))
        return false;

    return std::all_of(token.begin(), token.end(), [](char c) {
        return std::isalnum(c) || c == '_';
    });
}

bool isNumber(const std::string& token) {
    if (token.empty())
        return false;

    bool hasDecimalPoint = false;
    size_t start = 0;

    for (size_t i = start; i < token.size(); ++i) {
        if (token[i] == '.') {
            if (hasDecimalPoint)  // More than one decimal point is not allowed
                return false;
            hasDecimalPoint = true;
        } else if (!std::isdigit(token[i])) {
            return false;
        }
    }

    // Ensure the token is not just a decimal point
    if (token.size() == 1 && token == ".")
        return false;

    return true;
}

std::string separateSymbols(const std::string& line) {
    std::unordered_set<char> specialPunctuation = {'(', ')', '{', '}', ';', '!', '"', ','};
    std::unordered_set<std::string> multiCharSymbols = {"!(", ")!", "!{", "}!", "!\"", "\"!", "->"};

    std::string result;
    size_t i = 0;

    while (i < line.size()) {
        bool symbolMatched = false;

        for (const auto& symbol : multiCharSymbols) {
            if (line.substr(i, symbol.size()) == symbol) {
                result += " " + symbol + " ";  // Add a symbol to the spaces
                i += symbol.size();           // Advances the index to the next symbol
                symbolMatched = true;
                break;
            }
        }

        // If it is not a compound symbol, check single characters
        if (!symbolMatched) {
            if (specialPunctuation.count(line[i])) {
                result += ' ';
                result += line[i];
                result += ' ';
            } else {
                result += line[i];
            }
            i++;
        }
    }

    return result;
}

std::string formatOutput(const std::string& type, const std::string& value) {
    std::string formattedOutput;

    formattedOutput.append(R"(  {"type": ")")
            .append(type)
            .append(R"(", "value": ")")
            .append(value).append("\"}");

    if (value != "paha")
        formattedOutput.append(",").append("\n");


    return formattedOutput;
}

bool processLiteralString(std::string& token, std::stringstream& ss, int& lineCount, std::ofstream& outfile ) {
    std::string literal = token;
    bool isTerminated = false;

    // Verify if the token is already complete string
    if (literal.size() >= 2 && literal.substr(0, 2) == "!\"" &&
        literal.substr(literal.size() - 2) == "\"!") {
        isTerminated = true;
    } else {
        // Keeps capturing tokens unitl it finds the end of the String
        std::string nextPart;
        while (ss >> nextPart) {
            literal += " " + nextPart;
            if (nextPart.size() >= 2 && nextPart.substr(nextPart.size() - 2) == "\"!") {
                isTerminated = true;
                break;
            }
        }
    }

    if (!isTerminated) {
        std::cerr << "[ERROR: UNTERMINATED STRING LITERAL AT LINE "
                  << lineCount << "]" << std::endl;

        outfile << formatOutput("LEXICAL_ERROR", "Unterminated string literal");
        errorCount++;
    } else {
        token = literal.substr(2, literal.size() - 4);
        outfile << formatOutput("LITERAL_STRING", token);
    }

    return isTerminated;
}

void analyzeLine(const std::string& line, std::ofstream& outfile, int& lineCount) {
    std::string separatedLine = separateSymbols(line);
    std::stringstream ss(separatedLine);
    std::string token;

    while (ss >> token) {
        if (token == "//") {
            std::string comment;
            std::getline(ss, comment); 
            outfile << formatOutput("COMMENT", comment);
            break;
        }

        if (token.substr(0, 2) == "!\"") {
            processLiteralString(token, ss, lineCount, outfile);
            continue;
        }

        if (keywords.find(token) != keywords.end()) {
            outfile << formatOutput(keywords[token], token);
            continue; 
        }

        if (operators.find(token) != operators.end()) {
            outfile << formatOutput(operators[token], token);
        }

        if (symbols.find(token) != symbols.end()) {
            outfile << formatOutput(symbols[token], token);
            continue;
        }

        if (isNumber(token)) {
            outfile << formatOutput("NUMBER", token);
        }

        if (token.front() == '"') {
            if (processLiteralString(token, ss, lineCount, outfile)) {
                continue;
            }
        }

        if (isIdentifier(token)) {
            outfile << formatOutput("IDENTIFIER", token);
        } else {
        
            // If none 'if' above read the token, treat like a lexical error
            if (!isNumber(token) && !isIdentifier(token) &&
                keywords.find(token) == keywords.end() &&
                operators.find(token) == operators.end() &&
                symbols.find(token) == symbols.end()) {
                    std::cerr << "[ERRO LÉXICO: TOKEN DESCONHECIDO NA LINHA " << lineCount
                            << ": " << token << "]" << std::endl;
                    outfile << formatOutput("LEXICAL_ERROR", token);
                    errorCount++;
                }
        }
    }
}

int main() {
    std::string fileName;
    int lineCount = 1;

    std::cout << "Type the input file name: ";
    std::cin >> fileName;

    std::ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        std::cerr << "[ERROR OPENING INPUT FILE]" << std::endl;
        return 1;
    }

    std::ofstream outfile(fileName + ".tpa");
    if (!outfile.is_open()) {
        std::cerr << "[ERROR OPENING OUTPUT FILE]" << std::endl;
        return 1;
    }

    std::cout << "[READING INPUT FILE...]" << std::endl << std::endl;
    outfile << "[" << std::endl;

    std::cout << "[ERRORS]:" << std::endl;

    std::string line;
    while (std::getline(inputFile, line)) {
        analyzeLine(line, outfile, lineCount);
        lineCount++;
    }

    outfile << std::endl << "]";
    inputFile.close();
    outfile.close();

    std::cout << std::endl << "[READING FINISHED, OUTPUT FILE " << fileName << ".tpa READY!]" << std::endl;
    std::cout << "[PROGRAM FINISHED WITH " << errorCount << " ERRORS!]" << std::endl;

    return 0;
}
