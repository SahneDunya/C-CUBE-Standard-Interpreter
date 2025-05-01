#ifndef CUBE_SYNTAX_PARSER_HPP
#define CUBE_SYNTAX_PARSER_HPP

#include <vector>
#include <string>
#include <memory>
#include <stdexcept> // Parsing hataları için
#include "lexer.hpp" // Lexer'ı kullanacak
#include "ast.hpp"   // AST düğümlerini oluşturacak

namespace CCube {

// Parsing hatası için özel istisna sınıfı
class ParsingError : public std::runtime_error {
public:
    Token token; // Hataya neden olan token
    ParsingError(const Token& token, const std::string& message)
        : std::runtime_error(message), token(token) {}

    // Hata mesajını daha detaylı hale getirme
    std::string what_with_location() const {
        return "Parsing Error at line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ": " + std::string(what());
    }
};

// Kaynak kodundan Token akışını alıp AST oluşturan sınıf
class Parser {
public:
    // Kurucu: Kullanılacak Lexer referansını alır.
    Parser(Lexer& lexer);

    // Yıkıcı
    ~Parser() = default;

    // Tüm kaynak kodu ayrıştırır ve Program AST düğümünü döndürür.
    std::unique_ptr<Program> parseProgram();

private:
    Lexer& lexer_; // Kullanılacak lexer

    // Parsing hatalarını kaydetmek için bir liste (isteğe bağlı, ilk hatada durulabilir de)
     std::vector<ParsingError> errors_;

    // Helper Fonksiyonlar

    // Mevcut token'ı getirir (peekToken kullanarak)
    Token currentToken() const;

    // Bir sonraki token'ın beklenen türde olup olmadığını kontrol eder ve eğer öyleyse tüketir.
    // Eşleşmezse hata fırlatır.
    Token consume(TokenType type, const std::string& message);

    // Bir sonraki token'ın beklenen türlerden biri olup olmadığını kontrol eder ve eğer öyleyse tüketir.
    // Eşleşmezse false döndürür, tüketmez.
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types); // Birden fazla tip için

    // Bir sonraki token'ın beklenen türde olup olmadığını kontrol eder (tüketmez).
    bool check(TokenType type) const;

    // Dosya sonuna ulaşıldı mı?
    bool isAtEnd() const;

    // Bir parsing hatası durumunda eşitleme (synchronization) yapar.
    // Bir sonraki olası deyim başlangıcını bularak parsinge devam etmeye çalışır.
    void synchronize();


    // Dilin gramer kurallarını ayrıştıran fonksiyonlar
    // Genellikle öncelik seviyelerine göre veya yapı türlerine göre ayrılır.

    // Genel deyimi ayrıştırır
    std::unique_ptr<Statement> parseStatement();

    // Değişken bildirimini ayrıştırır (örn: var myVar = 10;)
    std::unique_ptr<Statement> parseVarDeclaration();

    // İfade deyimini ayrıştırır (sadece bir ifade, örn: myFunction();)
     std::unique_ptr<Statement> parseExpressionStatement();

    // Blok deyimini ayrıştırır (girinti veya {} içindeki kod)
    std::unique_ptr<BlockStmt> parseBlockStatement(); // İndentation Handling needs to be here or in Lexer

    // Import deyimini ayrıştırır
    std::unique_ptr<Statement> parseImportStatement();

    // Match deyimini ayrıştırır
    std::unique_ptr<Statement> parseMatchStatement();
    std::unique_ptr<MatchCase> parseMatchCase(); // Match içindeki case'leri ayrıştırır

    // If deyimini ayrıştırır
    std::unique_ptr<Statement> parseIfStatement();

    // While deyimini ayrıştırır
    std::unique_ptr<Statement> parseWhileStatement();

    // Fonksiyon tanımını ayrıştırır
    std::unique_ptr<Statement> parseFunctionDefinition(); // def keywordü

    // Sınıf tanımını ayrıştırır
    std::unique_ptr<Statement> parseClassDefinition(); // class keywordü

    // Return deyimini ayrıştırır
    std::unique_ptr<Statement> parseReturnStatement();

    // Break/Continue deyimlerini ayrıştırır
    std::unique_ptr<Statement> parseBreakStatement();
    std::unique_ptr<Statement> parseContinueStatement();


    // İfadeleri öncelik sırasına göre ayrıştıran fonksiyonlar (örneğin, en düşük öncelikten en yükseğe)
    // Bu genellikle recursive descent parserlarda kullanılır.
    // Örnek bir sıralama (gerçek gramer kurallarına göre değişir):
    std::unique_ptr<Expression> parseExpression(); // En genel ifade (genellikle atama veya düşük öncelikli operatörler)
    std::unique_ptr<Expression> parseAssignment(); // Atama (=)
    std::unique_ptr<Expression> parseOr();         // Mantıksal OR (or)
    std::unique_ptr<Expression> parseAnd();        // Mantıksal AND (and)
    std::unique_ptr<Expression> parseEquality();   // Eşitlik (==, !=)
    std::unique_ptr<Expression> parseComparison(); // Karşılaştırma (<, >, <=, >=)
    std::unique_ptr<Expression> parseAddition();   // Toplama/Çıkarma (+, -)
    std::unique_ptr<Expression> parseMultiplication(); // Çarpma/Bölme (*, /)
    std::unique_ptr<Expression> parseUnary();      // Birli operatörler (-, not)
    std::unique_ptr<Expression> parseCallDotOrIndex(); // Fonksiyon çağrısı, nokta erişimi (metot/özellik), dizi erişimi
    std::unique_ptr<Expression> parsePrimary();    // En temel ifade (literaller, değişkenler, parantezli ifadeler)

    // Parantez içindeki ifadeleri ayrıştırır (örn: (a + b))
     std::unique_ptr<Expression> parseGrouping(); // parsePrimary içinde ele alınabilir

    // Literal değerleri ayrıştırır (sayı, string, boolean, None)
     std::unique_ptr<Expression> parseLiteral(); // parsePrimary içinde ele alınabilir

    // Tanımlayıcıları (değişken adları) ayrıştırır
     std::unique_ptr<Expression> parseIdentifier(); // parsePrimary içinde ele alınabilir

};

} namespace CCube

#endif // CUBE_SYNTAX_PARSER_HPP