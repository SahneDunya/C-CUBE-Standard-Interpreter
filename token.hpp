#ifndef CUBE_SYNTAX_TOKEN_HPP
#define CUBE_SYNTAX_TOKEN_HPP

#include <string>
#include <vector> // Bazı keyword listeleri için kullanılabilir, veya sadece enum yeterli

namespace CCube {

// C-CUBE dilindeki jeton (token) türleri
enum class TokenType {
    // Tek karakterli operatörler ve noktalama işaretleri
    PLUS, MINUS, STAR, SLASH, PERCENT, // +, -, *, /, %
    ASSIGN, EQ, NEQ, LT, GT, LTE, GTE, // =, ==, !=, <, >, <=, >=
    AND, OR, NOT,                      // and, or, not (şimdilik anahtar kelime, sonra sembol olabilir)
    DOT, COMMA, SEMICOLON, COLON,
    LPAREN, RPAREN, LBRACE, RBRACE, LSQUARE, RSQUARE, // (, ), {, }, [, ]

    // Anahtar Kelimeler (keywords) - Python'dan ve C-CUBE özelliklerinden ilhamla
    IMPORT, MATCH, IF, ELSE, WHILE, FOR, DEF, CLASS, RETURN, BREAK, CONTINUE,
    NONE_KW, TRUE_KW, FALSE_KW, // None, True, False

    // Literaller (değerler)
    IDENTIFIER, // Değişken, fonksiyon, sınıf adı vb.
    INTEGER, FLOAT, STRING,

    // Yorumlar (genellikle lexer tarafından atlanır ama token olarak da temsil edilebilir)
    COMMENT, // Şimdilik token olarak tanımlandı, lexer bunu atlayabilir

    // Diğer
    UNKNOWN, // Tanımlanamayan karakter dizisi
    END_OF_FILE // Dosya sonu

    // Not: İhtiyaç oldukça buraya yeni token türleri eklenecektir.
};

// Bir jetonu (token) temsil eden yapı
struct Token {
    TokenType type;
    std::string lexeme; // Kaynak kodundaki karşılığı (örn: "if", "myVar", "123")
    int line;           // Satır numarası
    int column;         // Sütun numarası

    // Kolay hata ayıklama için token'ı yazdırma fonksiyonu
    std::string toString() const;
};

// TokenType enum değerini string olarak döndüren yardımcı fonksiyon
std::string tokenTypeToString(TokenType type);

} namespace CCube

#endif // CUBE_SYNTAX_TOKEN_HPP