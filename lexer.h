#ifndef C_CUBE_LEXER_H
#define C_CUBE_LEXER_H

#include <string>
#include <vector>
#include "token.h"

class Lexer {
private:
    const std::string& source;
    std::vector<Token> tokens;
    int start = 0; // Şu anki token'ın başlangıç indeksi
    int current = 0; // Şu an incelenen karakterin indeksi
    int line = 1; // Kaynak kodundaki satır numarası

    // Yardımcı fonksiyonlar (private)
     bool isAtEnd();
     char advance();
     void addToken(TokenType type);
     void addToken(TokenType type, LiteralType literal); // Literal destekli
     void scanToken();
     bool match(char expected);
     char peek();
     char peekNext();
     void string();
     void number();
     void identifier();
     bool isDigit(char c);
     bool isAlpha(char c);
     bool isAlphaNumeric(char c);
     void handleComment(); // Yorum satırları için

public:
    Lexer(const std::string& source) : source(source) {}

    std::vector<Token> scanTokens();
};

#endif // C_CUBE_LEXER_H
