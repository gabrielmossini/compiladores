#include <iostream>

#include "lexer.h"

using namespace std;

int main() {
    Lexer lexer;
    string input;

    cout << "Insira uma sequência de caracteres (ou 'exit' para sair): ";
    while (getline(cin, input)) {
        if (input == "exit") break;

        vector<Token> tokens = lexer.tokenize(input);
        for (const auto& token : tokens) {
            switch (token.type) {
                case PAITE:
                    cout << "> Encontrei um inteiro: " << token.value << endl;
                    break;
                case ANATE:
                    cout << "> Encontrei um real: " << token.value << endl;
                    break;
                case SA:
                    cout << "> Encontrei uma string: " << token.value << endl;
                    break;
                case KAR:
                    cout << "> Encontrei um caractere: " << token.value << endl;
                    break;
                case BOOL:
                    cout << "> Encontrei um booleano: " << token.value << endl;
                    break;
                case OPERADOR:
                    cout << "> Encontrei um operador: " << token.value << endl;
                    break;
                case RESERVADA:
                    cout << "> Encontrei uma palavra reservada: " << token.value << endl;
                    break;
                case ESPECIAL:
                    cout << "> Encontrei um caractere especial: " << token.value << endl;
                    break;
                default:
                    cout << "> Token inválido: " << token.value << endl;
                    break;
            }
        }
        cout << "Insira uma sequência de caracteres (ou 'exit' para sair): ";
    }

    return 0;
}
