#include "parser.h"
#include "token.h"
#include "ast.h" // AST düğüm yapılarını içerir
#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept> // Parsing hataları için

// Hata işleme: Basit bir Struct veya Class tanımlayabiliriz.
// Ya da şimdilik sadece konsola yazdırıp bir flag set edebiliriz.
// Hataları exception ile fırlatmak parsing mantığını daha temiz tutar.

struct ParseError : public std::runtime_error {
    Token token;
    ParseError(const Token& token, const std::string& message)
        : std::runtime_error(message), token(token) {}
};

// Hata raporlama fonksiyonu (main.cpp'de olmalı veya ayrı bir modül)
 extern bool hadError; // Global hata flag'i (basitlik için)
 void error(const Token& token, const std::string& message); // Hata raporlama fonksiyonu

// Parser sınıfı implementasyonu

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        // Her döngüde bir üst seviye bildirimi veya deyimi ayrıştırmayı dene
        // Hata durumunda senkronize ol
        try {
            statements.push_back(declaration());
        } catch (const ParseError& error) {
            // Hata raporla
             std::cerr << "[Line " << error.token.line << "] Error at '" << error.token.lexeme << "': " << error.what() << std::endl;
             hadError = true; // Global hatayı işaretle
            synchronize(); // Hata kurtarma
        }
    }
    return statements;
}

// Gramer Kurallarına Karşılık Gelen Ayrıştırma Metodları (En üstten başlayarak)

// declaration -> classDecl | funDecl | varDecl | statement
StmtPtr Parser::declaration() {
    // İlk token'a bakarak ne tür bir bildirim olduğunu anlamaya çalış
    // Sınıf, fonksiyon, değişken gibi özel anahtar kelimeleri kontrol et
    if (match(TokenType::CLASS)) return classDecl();
    if (match(TokenType::FUN)) return funDecl("function"); // Fonksiyonlar için

    // 'var' anahtar kelimesi değişken bildirimi için
    if (match(TokenType::VAR)) return varDecl();

    // Hiçbiri değilse, bir deyimdir
    return statement();
}

// classDecl -> "class" IDENTIFIER ( "<" IDENTIFIER )? "{" funDecl* "}"
StmtPtr Parser::classDecl() {
    // 'class' zaten tüketildi
    Token name = consume(TokenType::IDENTIFIER, "Expect class name after 'class'.");

    // Miras alıyor mu? (< SuperClass)
    ExprPtr superclass = nullptr; // Şimdilik miras yok
    if (match(TokenType::LESS)) {
        consume(TokenType::IDENTIFIER, "Expect superclass name after '<'.");
         superclass = std::make_shared<VariableExpr>(previous()); // Üst sınıf adını bir VariableExpr olarak sakla
        std::cerr << "Warning: Class inheritance parsing not fully implemented." << std::endl;
    }


    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

    // Metotları ayrıştır
    std::vector<StmtPtr> methods;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        methods.push_back(funDecl("method")); // Metotlar için funDecl çağır
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");

     return std::make_shared<ClassDeclStmt>(name, superclass, methods);
     std::cerr << "Warning: Class declaration parsing not fully implemented." << std::endl;
    return nullptr; // Yer tutucu
}

// funDecl -> "fun" IDENTIFIER "(" parameters? ")" block
// parameters -> IDENTIFIER ( "," IDENTIFIER )*
StmtPtr Parser::funDecl(const std::string& kind) {
    // 'fun' zaten tüketildi
    Token name = consume(TokenType::IDENTIFIER, "Expect " + kind + " name after 'fun'.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");

    // Parametreleri ayrıştır
    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            // Maksimum parametre sayısını kontrol et
             if (parameters.size() >= 255) { // Örnek limit
                error(peek(), "Can't have more than 255 parameters.");
             }
            parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");

    // Fonksiyon/metot gövdesini (blok) ayrıştır
    StmtPtr body = block();

     return std::make_shared<FunDeclStmt>(name, parameters, body);
     std::cerr << "Warning: Function declaration parsing not fully implemented." << std::endl;
    return nullptr; // Yer tutucu
}


// varDecl -> "var" IDENTIFIER ( "=" expression )? ";"
StmtPtr Parser::varDecl() {
    // 'var' zaten tüketildi
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name after 'var'.");

    // Başlangıç değeri var mı?
    ExprPtr initializer = nullptr;
    if (match(TokenType::EQUAL)) {
        initializer = expression(); // Başlangıç değerini ayrıştır
    }

    // Deyim sonlandırıcı (Noktalı virgül veya Python-like newline/indentasyon)
    // Python-like newline/indentasyon için gramer yapısı daha farklı olurdu.
    // Şimdilik C-like noktalı virgül varsayalım veya newline'ı opsiyonel yapalım.
    // Basitlik için noktalı virgül zorunlu tutalım.
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");

    return std::make_shared<VarDeclStmt>(name, initializer);
}


// statement -> exprStmt | forStmt | ifStmt | printStmt | returnStmt | whileStmt | block | importStmt | matchStmt
StmtPtr Parser::statement() {
    if (match(TokenType::PRINT)) return printStmt();
     if (match(TokenType::FOR)) return forStmt(); // forStmt daha sonra
    if (match(TokenType::IF)) return ifStmt();
    if (match(TokenType::RETURN)) return returnStmt();
    if (match(TokenType::WHILE)) return whileStmt();
    if (match(TokenType::LEFT_BRACE)) return block(); // Blok deyimi
    if (match(TokenType::IMPORT)) return importStmt(); // import deyimi
    if (match(TokenType::MATCH)) return matchStmt(); // match deyimi

    // Hiçbiri değilse, bir ifade deyimidir (sonunda noktalı virgül olan ifade)
    return expressionStmt();
}

// printStmt -> "print" expression ";"
StmtPtr Parser::printStmt() {
    // 'print' zaten tüketildi
    ExprPtr value = expression(); // Yazdırılacak ifadeyi ayrıştır
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_shared<PrintStmt>(value);
}

// returnStmt -> "return" expression? ";"
StmtPtr Parser::returnStmt() {
     // 'return' zaten tüketildi
     Token keyword = previous(); // 'return' token'ını sakla

     ExprPtr value = nullptr; // Döndürülen değer (opsiyonel)
     // Eğer bir sonraki token ';' değilse, bir ifadeyi ayrıştır
     if (!check(TokenType::SEMICOLON)) {
         value = expression();
     }

     consume(TokenType::SEMICOLON, "Expect ';' after return value.");
      return std::make_shared<ReturnStmt>(keyword, value);
      std::cerr << "Warning: Return statement parsing not fully implemented." << std::endl;
     return nullptr; // Yer tutucu
}

// block -> "{" declaration* "}"
StmtPtr Parser::block() {
    // '{' zaten tüketildi
    std::vector<StmtPtr> statements;

    // '}' veya dosya sonuna kadar bildirimleri/deyimleri ayrıştır
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");

    return std::make_shared<BlockStmt>(statements);
}

// ifStmt -> "if" "(" expression ")" statement ("else" statement)?
StmtPtr Parser::ifStmt() {
    // 'if' zaten tüketildi
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    ExprPtr condition = expression(); // Koşul ifadesini ayrıştır
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    StmtPtr thenBranch = statement(); // 'if' bloğunu ayrıştır

    // 'else' var mı?
    StmtPtr elseBranch = nullptr;
    if (match(TokenType::ELSE)) {
        elseBranch = statement(); // 'else' bloğunu ayrıştır
    }

    return std::make_shared<IfStmt>(condition, thenBranch, elseBranch);
}

// whileStmt -> "while" "(" expression ")" statement
StmtPtr Parser::whileStmt() {
    // 'while' zaten tüketildi
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    ExprPtr condition = expression(); // Koşul ifadesini ayrıştır
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");

    StmtPtr body = statement(); // Döngü gövdesini ayrıştır

    return std::make_shared<WhileStmt>(condition, body);
}

// importStmt -> "import" IDENTIFIER ( "." IDENTIFIER )* ( "as" IDENTIFIER )? ("," IDENTIFIER ( "." IDENTIFIER )* ( "as" IDENTIFIER )?)* ";"
StmtPtr Parser::importStmt() {
    // 'import' zaten tüketildi
    // Import yolu ve isteğe bağlı 'as' kısmını ayrıştır
     std::cerr << "Warning: Import statement parsing not fully implemented." << std::endl;
    // Örnek basitleştirilmiş import: import module_name;
     Token moduleName = consume(TokenType::IDENTIFIER, "Expect module name after 'import'.");
     consume(TokenType::SEMICOLON, "Expect ';' after import statement.");
     return std::make_shared<ImportStmt>(moduleName.lexeme); // Sadece ismi sakla
    // Daha karmaşık importlar (modül.altmodul, as ile isim değiştirme) daha detaylı ayrıştırma gerektirir
     while (!check(TokenType::SEMICOLON) && !isAtEnd()) {
         advance(); // Şimdilik tokenları atla
     }
     consume(TokenType::SEMICOLON, "Expect ';' after import statement."); // Terminator'ı tüket
     return nullptr; // Yer tutucu
}

// matchStmt -> "match" expression "{" case* "}"
// case -> "case" pattern ":" statement
StmtPtr Parser::matchStmt() {
     // 'match' zaten tüketildi
     ExprPtr subject = expression(); // Eşleşme yapılacak ifadeyi ayrıştır
     consume(TokenType::LEFT_BRACE, "Expect '{' after match expression.");

     std::vector<MatchCase> cases; // AST'nizde MatchCase yapısı olmalı (pattern ve body içerir)

     // '}' veya dosya sonuna kadar case'leri ayrıştır
     while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
         consume(TokenType::CASE, "Expect 'case' keyword inside match statement.");
         // Pattern ayrıştırma (Bu kısım dilinize göre değişir, basit bir örnek)
         ExprPtr pattern = expression(); // Şimdilik pattern'ın basit bir ifade olduğunu varsayalım
         consume(TokenType::COLON, "Expect ':' after match case pattern.");
         StmtPtr body = statement(); // Case gövdesini ayrıştır

          cases.push_back({pattern, body}); // MatchCase yapısına ekle
         std::cerr << "Warning: Match case pattern parsing is simplified." << std::endl; // Desen ayrıştırması daha karmaşık olabilir
     }

     consume(TokenType::RIGHT_BRACE, "Expect '}' after match body.");

      return std::make_shared<MatchStmt>(subject, cases);
      std::cerr << "Warning: Match statement parsing not fully implemented." << std::endl;
     return nullptr; // Yer tutucu
}


// expressionStmt -> expression ";"
StmtPtr Parser::expressionStmt() {
    ExprPtr expr = expression(); // İfadeyi ayrıştır
     consume(TokenType::SEMICOLON, "Expect ';' after expression."); // Eğer noktalı virgül zorunluysa
    // Python-like: Newline veya dosya sonu da olabilir. Basitlik için noktalı virgül.
     if (!match(TokenType::SEMICOLON)) {
         // Hata veya Python-like newline kontrolü eklenebilir
          std::cerr << "[Line " << peek().line << "] Warning: Missing ';' after expression statement (assuming Python-like newline termination)." << std::endl;
          // Eğer Python-like newline terminate ise, buraya konsolide etmeyiz,
          // statement() metodunda expression() çağrılır ve sonraki token'a bakılır.
          // Şu anki setup C-like ';' bekliyor gibi görünüyor. Eğer newline terminate ise,
          consume(TokenType::SEMICOLON) kaldırılmalı ve parser bu durumda devam etmeli.
     }

    return std::make_shared<ExpressionStmt>(expr);
}

// İfade Ayrıştırma Metodları (En düşük öncelikten başlayarak)

// expression -> assignment
ExprPtr Parser::expression() {
    return assignment();
}

// assignment -> (call | variable | get | set) "=" assignment | logic_or
ExprPtr Parser::assignment() {
    ExprPtr expr = logic_or(); // Sağ tarafın en düşük öncelikli ifadesini ayrıştır

    // Eğer bir sonraki token '=' ise, bu bir atama ifadesidir
    if (match(TokenType::EQUAL)) {
        Token equals = previous(); // '=' token'ını sakla
        ExprPtr value = assignment(); // Sağ tarafı ayrıştır (atama sağdan birleşiktir)

        // Sol tarafın geçerli bir atama hedefi olup olmadığını kontrol et
        // Geçerli hedefler: değişkenler (VariableExpr), obje üyeleri (GetExpr)
        if (auto var_expr = std::dynamic_pointer_cast<VariableExpr>(expr)) {
            // Değişken ataması
            Token name = var_expr->name; // Değişkenin adını al
            return std::make_shared<AssignExpr>(name, value);
        }
        // Obje üyelerine atama (SetExpr) buraya eklenecek
         if (auto get_expr = std::dynamic_pointer_cast<GetExpr>(expr)) { ... }


        // Geçerli bir atama hedefi değilse hata ver
         error(equals, "Invalid assignment target.");
         std::cerr << "[Line " << equals.line << "] Error: Invalid assignment target." << std::endl;
        // Hata durumunda value'yu döndürmek veya nullptr dönmek stratejinize bağlı
        return nullptr; // Hata durumunda null
    }

    // '=' yoksa, sadece logic_or ifadesidir
    return expr;
}

// logic_or -> logic_and ("or" logic_and)*
ExprPtr Parser::logic_or() {
    ExprPtr expr = logic_and(); // Sol operandı ayrıştır

    // 'or' token'ları olduğu sürece devam et
    while (match(TokenType::OR)) {
        Token op = previous(); // 'or' token'ını sakla
        ExprPtr right = logic_and(); // Sağ operandı ayrıştır
        expr = std::make_shared<LogicalExpr>(expr, op, right); // Yeni bir LogicalExpr düğümü oluştur
    }

    return expr;
}

// logic_and -> equality ("and" equality)*
ExprPtr Parser::logic_and() {
    ExprPtr expr = equality(); // Sol operandı ayrıştır

    // 'and' token'ları olduğu sürece devam et
    while (match(TokenType::AND)) {
        Token op = previous(); // 'and' token'ını sakla
        ExprPtr right = equality(); // Sağ operandı ayrıştır
        expr = std::make_shared<LogicalExpr>(expr, op, right); // Yeni bir LogicalExpr düğümü oluştur
    }

    return expr;
}


// equality -> comparison (("==" | "!=") comparison)*
ExprPtr Parser::equality() {
    ExprPtr expr = comparison(); // Sol operandı ayrıştır

    // '==' veya '!=' token'ları olduğu sürece devam et
    while (match(TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL)) {
        Token op = previous(); // Operatör token'ını sakla
        ExprPtr right = comparison(); // Sağ operandı ayrıştır
        expr = std::make_shared<BinaryExpr>(expr, op, right); // Yeni bir BinaryExpr düğümü oluştur
    }

    return expr;
}

// comparison -> term ((">" | ">=" | "<" | "<=") term)*
ExprPtr Parser::comparison() {
    ExprPtr expr = term(); // Sol operandı ayrıştır

    // Karşılaştırma operatörleri olduğu sürece devam et
    while (match(TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL)) {
        Token op = previous(); // Operatör token'ını sakla
        ExprPtr right = term(); // Sağ operandı ayrıştır
        expr = std::make_shared<BinaryExpr>(expr, op, right); // Yeni bir BinaryExpr düğümü oluştur
    }

    return expr;
}

// term -> factor (("+" | "-") factor)*
ExprPtr Parser::term() {
    ExprPtr expr = factor(); // Sol operandı ayrıştır

    // '+' veya '-' operatörleri olduğu sürece devam et
    while (match(TokenType::PLUS, TokenType::MINUS)) {
        Token op = previous(); // Operatör token'ını sakla
        ExprPtr right = factor(); // Sağ operandı ayrıştır
        expr = std::make_shared<BinaryExpr>(expr, op, right); // Yeni bir BinaryExpr düğümü oluştur
    }

    return expr;
}

// factor -> unary (("*" | "/") unary)*
ExprPtr Parser::factor() {
    ExprPtr expr = unary(); // Sol operandı ayrıştır

    // '*' veya '/' operatörleri olduğu sürece devam et
    while (match(TokenType::STAR, TokenType::SLASH)) {
        Token op = previous(); // Operatör token'ını sakla
        ExprPtr right = unary(); // Sağ operandı ayrıştır
        expr = std::make_shared<BinaryExpr>(expr, op, right); // Yeni bir BinaryExpr düğümü oluştur
    }

    return expr;
}

// unary -> ("!" | "-") unary | call
ExprPtr Parser::unary() {
    // Unary operatör var mı?
    if (match(TokenType::BANG, TokenType::MINUS)) {
        Token op = previous(); // Operatör token'ını sakla
        ExprPtr right = unary(); // Sağ operandı ayrıştır (unary sağdan birleşiktir)
        return std::make_shared<UnaryExpr>(op, right); // Yeni bir UnaryExpr düğümü oluştur
    }

    // Unary operatör yoksa, bir çağrı ifadesidir
    return call();
}

// call -> primary ("(" arguments? ")" | "." IDENTIFIER)*
// arguments -> expression ("," expression)*
ExprPtr Parser::call() {
    ExprPtr expr = primary(); // Çağrının sol tarafını (fonksiyon, obje vb.) ayrıştır

    // Parantez '(' veya nokta '.' olduğu sürece devam et (çağrı veya özellik erişimi)
    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            // Fonksiyon veya metot çağrısı
            expr = finishCall(expr); // Argümanları ayrıştır ve CallExpr oluştur
        } else if (match(TokenType::DOT)) {
            // Obje özelliği erişimi
            Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
             expr = std::make_shared<GetExpr>(expr, name); // GetExpr düğümü oluştur
             std::cerr << "Warning: Property access parsing not fully implemented." << std::endl;
             return nullptr; // Yer tutucu, geliştirilmeli
        }
        // Başka zincirleme operatörler (örn: [indeks]) buraya eklenebilir
        else {
            break; // Parantez veya nokta yoksa döngüden çık
        }
    }

    return expr;
}

// finishCall metodu (call metoduna yardımcı)
ExprPtr Parser::finishCall(ExprPtr callee) {
    // '(' zaten tüketildi
    std::vector<ExprPtr> arguments;
    // Kapanış parantezi ')' değilse ve dosya sonu değilse argümanları ayrıştır
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            // Maksimum argüman sayısını kontrol et
             if (arguments.size() >= 255) { // Örnek limit
                error(peek(), "Can't have more than 255 arguments.");
             }
            arguments.push_back(expression()); // Argümanı ayrıştır
        } while (match(TokenType::COMMA)); // Virgül varsa bir argüman daha var
    }

    Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

     return std::make_shared<CallExpr>(callee, paren, arguments);
     std::cerr << "Warning: Call expression parsing not fully implemented." << std::endl;
    return nullptr; // Yer tutucu
}


// primary -> "true" | "false" | "none" | NUMBER | STRING | "(" expression ")" | IDENTIFIER | "this" | "super" ( "." IDENTIFIER | "(" arguments? ")" ) | "match" expression "{" case* "}" (eğer match ifade ise)
ExprPtr Parser::primary() {
    // Literal değerler
    if (match(TokenType::FALSE)) return std::make_shared<LiteralExpr>(previous()); // 'false' token'ını içerir
    if (match(TokenType::TRUE)) return std::make_shared<LiteralExpr>(previous());  // 'true' token'ını içerir
    if (match(TokenType::NONE)) return std::make_shared<LiteralExpr>(previous());  // 'none' token'ını içerir
    if (match(TokenType::NUMBER)) return std::make_shared<LiteralExpr>(previous()); // Sayı token'ını içerir
    if (match(TokenType::STRING)) return std::make_shared<LiteralExpr>(previous()); // String token'ını içerir

    // Parantez içindeki ifade
    if (match(TokenType::LEFT_PAREN)) {
        ExprPtr expr = expression(); // Parantez içindeki ifadeyi ayrıştır
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_shared<GroupingExpr>(expr); // GroupingExpr düğümü oluştur
    }

    // Tanımlayıcı (değişken)
    if (match(TokenType::IDENTIFIER)) {
        return std::make_shared<VariableExpr>(previous()); // Değişken token'ını içerir
    }

    // 'this' anahtar kelimesi
    if (match(TokenType::THIS)) {
         return std::make_shared<ThisExpr>(previous());
         std::cerr << "Warning: 'this' parsing not fully implemented." << std::endl;
         return nullptr; // Yer tutucu
    }

    // 'super' anahtar kelimesi (şimdilik metot çağrısı veya özellik erişimi beklenir)
    if (match(TokenType::SUPER)) {
          Token keyword = previous();
         consume(TokenType::DOT, "Expect '.' after 'super'.");
         Token method = consume(TokenType::IDENTIFIER, "Expect superclass method name.");
         return std::make_shared<SuperExpr>(keyword, method);
         std::cerr << "Warning: 'super' parsing not fully implemented." << std::endl;
         return nullptr; // Yer tutucu
    }

    // Match ifadesi (eğer expression olarak da kullanılabiliyorsa)
    if (match(TokenType::MATCH)) {
         // Bu, matchStmt'deki mantığa benzer olur, ama MatchExpr düğümü döndürür.
         std::cerr << "Warning: Match expression parsing not fully implemented." << std::endl;
         // Implementasyonu matchStmt'ye benzer olacaktır.
         return nullptr; // Yer tutucu
    }


    // Hiçbirine uymuyorsa hata ver
     throw error(peek(), "Expect expression.");
     std::cerr << "[Line " << peek().line << "] Error: Expect expression." << std::endl;
      hadError = true;
     return nullptr; // Hata durumunda null döndür
}


// Yardımcı Ayrıştırma Metodları

// Mevcut token'a bak (tüketme)
Token Parser::peek() const {
    return tokens[current];
}

// Bir önceki token'a bak
Token Parser::previous() const {
    return tokens[current - 1];
}

// Mevcut token'ı tüket ve döndür
Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

// Mevcut token'ın tipini kontrol et
bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

// Mevcut token, verilen tiplerden biriyle eşleşiyor mu?
// Eşleşiyorsa tüket ve true döndür, aksi halde false döndür.
template<typename... Types>
bool Parser::match(TokenType type, Types... rest) {
    if (check(type)) {
        advance();
        return true;
    }
    // Variadic template için recursive check
    if constexpr (sizeof...(rest) > 0) {
        return match(rest...);
    }
    return false;
}


// Mevcut token'ı beklenen tipteyse tüket, değilse hata ver
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }

    // Hata oluştur ve fırlat
    throw error(peek(), message);
    // Veya sadece hata raporla ve hata durumunu işaretle:
     error(peek(), message);
     return previous(); // Hata durumunda son token'ı döndürmek iyi olmayabilir
     return Token(TokenType::EOF_TOKEN, "", -1); // Hata durumunda geçersiz bir token döndür
}

// Ayrıştırma hatası oluşturma (fırlatılan ParseError nesnesi)
ParseError Parser::error(const Token& token, const std::string& message) {
    // Global hata flag'ini set etmek main fonksiyonunun durmasına yardımcı olur
     hadError = true;
    return ParseError(token, message); // Bir ParseError nesnesi fırlat
}


// Hata kurtarma: Bir hata oluştuğunda, bir sonraki deyimin başına atlamaya çalış
void Parser::synchronize() {
    advance(); // Hata token'ını atla

    while (!isAtEnd()) {
        // Bir önceki token ';' ise, muhtemelen bir deyim sonudur
        if (previous().type == TokenType::SEMICOLON) return;

        // Belirli anahtar kelimeleri gördüğümüzde de bir deyim başlangıcı olabilir
        // Bu anahtar kelimeler genellikle bir deyimin veya bildirimin başında bulunur
        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
            case TokenType::IMPORT:
            case TokenType::MATCH: // Match deyimi başlangıcı
                return; // Burada dur ve ayrıştırmaya devam et

            default:
                // Diğer durumlarda tokenları atlamaya devam et
                break;
        }

        advance(); // Token'ı atla
    }
}


// Tüm tokenlar tüketildi mi?
bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

// Variadic template match fonksiyonu için explicit specialization
// Bu, C++17'den önce gerekebilir, modern C++'ta template otomatik olarak çözülür.
template bool Parser::match<TokenType>(TokenType);
template bool Parser::match<TokenType, TokenType>(TokenType, TokenType);
// ... gerektiği kadar ekleyin veya C++17 kullanın.
