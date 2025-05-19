#ifndef C_CUBE_PARSER_H
#define C_CUBE_PARSER_H

#include <vector>
#include "token.h"
#include "ast.h" // AST düğümlerini kullanacak

class Parser {
private:
    const std::vector<Token>& tokens;
    int current = 0; // Şu an işlenen token'ın indeksi

    // Yardımcı fonksiyonlar (private)
     StmtPtr statement();
     StmtPtr printStatement(); // print deyimi için
     StmtPtr varDeclaration(); // var deyimi için
     StmtPtr classDeclaration(); // class deyimi için
     StmtPtr funDeclaration(); // fonksiyon tanımı için
     StmtPtr expressionStatement();
     ExprPtr expression();
     ExprPtr assignment(); // Atama işlemleri için
     ExprPtr matchExpression(); // match ifadesi için parsing mantığı
     ExprPtr equality(); // ==, != gibi operatörler için
    // ... Diğer gramer kurallarına karşılık gelen metodlar ...

    // Token tüketme ve kontrol metodları
     bool match(TokenType type);
     bool check(TokenType type);
     Token advance();
     bool isAtEnd();
     Token peek();
     Token previous();
     Token consume(TokenType type, const std::string& message);
     void synchronize(); // Hata kurtarma için

    // Hata raporlama
     void error(const Token& token, const std::string& message);
    // Exception/Hata sınıfı da tanımlanabilir

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

    // Programı ayrıştırma metodu (genellikle bir deyim listesi döndürür)
    std::vector<StmtPtr> parse();
};

#endif // C_CUBE_PARSER_H
