#ifndef C_CUBE_TOKEN_H
#define C_CUBE_TOKEN_H

#include <string>

enum class TokenType {
    // Tek karakterli tokenlar
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    // Bir veya iki karakterli tokenlar
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literaller
    IDENTIFIER, STRING, NUMBER,

    // Anahtar kelimeler (C-CUBE'a özel olanlar veya Python'dan esinlenilenler)
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NONE, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    MATCH, IMPORT,

    // Diğer (örneğin, dosya sonu)
    EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string lexeme;
    // Literal değer (sayılar, stringler için kullanılabilir, std::variant düşünebilirsiniz)
     std::variant<std::string, double, bool, std::monostate> literal; // Örnek
    int line; // Hata raporlama için

    // Constructor
    Token(TokenType type, std::string lexeme, int line)
        : type(type), lexeme(std::move(lexeme)), line(line) {}

    // Hata ayıklama veya yazdırma için kullanışlı bir metod
     std::string toString() const;
};

#endif // C_CUBE_TOKEN_H
