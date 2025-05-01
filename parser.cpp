#include "parser.hpp"
#include <iostream> // Hata mesajları için
#include <vector> // std::vector kullanımı için

namespace CCube {

// Parser Kurucu
Parser::Parser(Lexer& lexer)
    : lexer_(lexer), has_buffered_token_(false) // Lexer referansını sakla
{
    // Parser başladığında lexer'dan ilk token'ı alabilir (peekToken kullanılırsa gerek kalmaz)
    // Veya peekToken doğrudan lexer'ın tamponunu kullanıyorsa, parser kendi tamponunu tutmaz.
    // Bizim lexer tasarımımız peekToken ile 1 token ileri bakıyor. Parser doğrudan onu kullanabilir.
}


// --- Yardımcı Fonksiyon Implementasyonları ---

// Mevcut token'ı getirir (lexer'ın peekToken'ını kullanır)
Token Parser::currentToken() const {
    return lexer_.peekToken();
}

// Bir sonraki token'ın beklenen türde olup olmadığını kontrol eder ve eğer öyleyse tüketir.
// Eşleşmezse hata fırlatır.
Token Parser::consume(TokenType type, const std::string& message) {
    // currentToken() lexer'ın buffer'ını getirir. getNextToken() buffer'ı tüketir.
    // Yani consume önce check yapıp sonra getNextToken çağırmalı.
    if (check(type)) {
        return lexer_.getNextToken(); // Token türü eşleşti, tüket ve ilerle
    }

    // Beklenen token gelmediyse hata
    throw ParsingError(currentToken(), message);
}

// Bir sonraki token'ın beklenen türlerden biri olup olmadığını kontrol eder ve eğer öyleyse tüketir.
// Eşleşmezse false döndürür, tüketmez.
bool Parser::match(TokenType type) {
    if (check(type)) {
        lexer_.getNextToken(); // Token türü eşleşti, tüket
        return true;
    }
    return false; // Eşleşmedi
}

bool Parser::match(std::initializer_list<TokenType> types) {
     for (TokenType type : types) {
         if (check(type)) {
             lexer_.getNextToken(); // Eşleşen ilk türü tüket
             return true;
         }
     }
     return false; // Hiçbiri eşleşmedi
}

// Bir sonraki token'ın beklenen türde olup olmadığını kontrol eder (tüketmez).
bool Parser::check(TokenType type) const {
    if (isAtEnd()) return type == TokenType::END_OF_FILE;
    return currentToken().type == type;
}

// Dosya sonuna ulaşıldı mı?
bool Parser::isAtEnd() const {
    return currentToken().type == TokenType::END_OF_FILE;
}

// Bir parsing hatası durumunda eşitleme (synchronization) yapar.
// Implementasyon daha sonra detaylandırılacaktır.
void Parser::synchronize() {
    lexer_.getNextToken(); // Hatalı tokenı atla

    while (!isAtEnd()) {
        // Bir önceki token satır sonu ise (basit senkronizasyon) veya
        // sonraki token bir deyim başlangıcına benziyorsa devam et
        // (Örn: class, def, import, if, while, for, return gibi anahtar kelimeler)

        // Lexer'ın satır bilgisini kullanmalıyız.
        // Şimdilik sadece bazı anahtar kelimelerde duracak basit bir eşitleme yapalım.
        switch (currentToken().type) {
            case TokenType::CLASS:
            case TokenType::DEF:
            case TokenType::IMPORT:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::FOR:
            case TokenType::RETURN:
                return; // Bu tokenlarda parsinge devam edilebilir

            case TokenType::END_OF_FILE:
                return; // Dosya sonu
            // Python'daki gibi satır sonu senkronizasyonu burada ele alınmalıdır.
            // Lexer'ın Newline tokenı döndürdüğünü varsayarsak, Newline'da durabiliriz.
             case TokenType::NEWLINE: return; // Eğer lexer newline tokenı üretiyorsa

            default:
                // Devam et, bir sonraki token'a geç
                lexer_.getNextToken();
        }
    }
}


// --- Gramer Parsing Fonksiyon Implementasyonları ---

// Tüm kaynak kodu ayrıştırır
std::unique_ptr<Program> Parser::parseProgram() {
    auto program_node = std::make_unique<Program>();
    while (!isAtEnd()) {
        // Üst düzey deyimleri/deklarasyonları ayrıştır
        try {
             program_node->statements.push_back(parseStatement());
        } catch (const ParsingError& e) {
             std::cerr << e.what_with_location() << std::endl;
             // Hatadan sonra parsingi devam ettirmek için senkronize ol
             synchronize();
             // Senkronizasyondan sonra mevcut satırdaki/kalan deyimi atlamak gerekebilir.
             // Basitlik için şimdilik sadece senkronize olalım.
        }
    }
    return program_node;
}

// Genel deyimi ayrıştırır (hangi deyim türü olduğunu belirler)
std::unique_ptr<Statement> Parser::parseStatement() {
    // İlk token'a bakarak hangi deyim türü olduğunu belirle
    if (match(TokenType::IMPORT)) return parseImportStatement();
    if (match(TokenType::CLASS)) return parseClassDefinition();
    if (match(TokenType::DEF)) return parseFunctionDefinition();
    if (match(TokenType::IF)) return parseIfStatement();
    if (match(TokenType::WHILE)) return parseWhileStatement();
    if (match(TokenType::RETURN)) return parseReturnStatement();
    if (match(TokenType::BREAK)) return parseBreakStatement();
    if (match(TokenType::CONTINUE)) return parseContinueStatement();
    if (match(TokenType::MATCH)) return parseMatchStatement();

    // TODO: Değişken bildirimi (örn: 'var' anahtar kelimesi veya atama ile)
     if (match(TokenType::VAR_KEYWORD)) return parseVarDeclaration();
     else if (check(TokenType::IDENTIFIER) && check(TokenType::ASSIGN)) return parseVarDeclaration(); // İki token ileri bakmak gerekebilir

    // Hiçbir özel deyim türüne uymuyorsa, ifade deyimi olarak kabul et
    return parseExpressionStatement();
}

// Değişken bildirimini ayrıştırır (Örn: myVar = 10;)
// Şu an gramerde 'var' gibi bir keyword tanımlanmadı, atama syntax'ı kullanılabilir.
// Bu fonksiyon şimdilik sadece placeholder.
std::unique_ptr<Statement> Parser::parseVarDeclaration() {
     // Örnek implementasyon (eğer syntax: identifier = expression)
      Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
      consume(TokenType::ASSIGN, "Expect '=' after variable name.");
      auto initializer = parseExpression();
      consume(TokenType::SEMICOLON, "Expect ';' after variable declaration."); // C-CUBE'un syntax'ı ; kullanıyorsa
      return std::make_unique<VarDeclStmt>(name, std::move(initializer));
    throw ParsingError(currentToken(), "Variable declarations are not yet implemented.");
}

// İfade deyimini ayrıştırır
std::unique_ptr<Statement> Parser::parseExpressionStatement() {
    // Bir ifadeyi ayrıştır
    auto expr = parseExpression();
    // Eğer C-CUBE satır sonu tabanlıysa (Python gibi) noktalı virgül beklemeyiz.
    // Eğer C-CUBE C/C++ gibi noktalı virgül kullanıyorsa:
     consume(TokenType::SEMICOLON, "Expect ';' after expression statement.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

// Blok deyimini ayrıştırır ({ } veya girinti)
// Python tarzı girinti burada karmaşıktır. Lexer INDENT/DEDENT üretmiyorsa,
// Parser'ın satır ve sütun bilgilerine göre girintiyi takip etmesi gerekir.
// Bu örnekte basitleştirilmiş bir blok gösterimi ({}) veya temel bir INDENT/DEDENT varsayalım.
std::unique_ptr<BlockStmt> Parser::parseBlockStatement() {
    // Eğer bloklar süslü parantezdeyse:
     consume(TokenType::LBRACE, "Expect '{' before block.");
     auto block = std::make_unique<BlockStmt>(currentToken().line, currentToken().column); // Konum '{' tokenından alınabilir

    // Eğer bloklar girintiye dayanıyorsa (Lexer INDENT/DEDENT üretiyor varsayımıyla):
    consume(TokenType::INDENT, "Expect indented block.");
    auto block = std::make_unique<BlockStmt>(currentToken().line, currentToken().column); // Konum INDENT tokenından alınabilir

    while (!check(TokenType::DEDENT) && !isAtEnd()) {
         // Eğer bloklar süslü parantezdeyse: !check(TokenType::RBRACE)
        block->statements.push_back(parseStatement()); // Blok içindeki deyimleri ayrıştır
    }

    // Eğer bloklar süslü parantezdeyse:
     consume(TokenType::RBRACE, "Expect '}' after block.");
    // Eğer bloklar girintiye dayanıyorsa:
    consume(TokenType::DEDENT, "Expect dedent after block.");

    return block;
}

// Import deyimini ayrıştırır
std::unique_ptr<Statement> Parser::parseImportStatement() {
    // import modül_adı, başka_modül, ...
    int line = currentToken().line; // 'import' tokenının konumu
    int column = currentToken().column;

    std::vector<std::string> module_names;
    do {
        Token module_name_token = consume(TokenType::IDENTIFIER, "Expect module name after 'import' or ','.");
        module_names.push_back(module_name_token.lexeme);
    } while (match(TokenType::COMMA)); // Virgüle kadar modül isimlerini oku

    // Python gibi import syntax'ında satır sonu beklenir, C gibi noktalı virgül beklenmez.
    // Eğer noktalı virgül kullanılıyorsa:
     consume(TokenType::SEMICOLON, "Expect ';' after import statement.");

    return std::make_unique<ImportStmt>(module_names, line, column);
}

// Match deyimini ayrıştırır
std::unique_ptr<Statement> Parser::parseMatchStatement() {
    int line = currentToken().line; // 'match' tokenının konumu
    int column = currentToken().column;

    // Match değerini ayrıştır
    consume(TokenType::LPAREN, "Expect '(' after 'match'."); // match (value) { ... }
    auto value_expr = parseExpression();
    consume(TokenType::RPAREN, "Expect ')' after match value.");

    auto match_stmt = std::make_unique<MatchStmt>(std::move(value_expr), line, column);

    // Match case bloğunu ayrıştır (girinti veya {})
     consume(TokenType::LBRACE, "Expect '{' after match expression."); // Eğer {} kullanılıyorsa
    consume(TokenType::INDENT, "Expect indented block after match expression."); // Eğer girinti kullanılıyorsa

    while (!check(TokenType::DEDENT) && !isAtEnd()) { // Eğer girinti kullanılıyorsa: !check(TokenType::DEDENT) &&
          !check(TokenType::RBRACE) &&
         match_stmt->cases.push_back(parseMatchCase()); // case'leri oku
    }

    consume(TokenType::RBRACE, "Expect '}' after match cases."); // Eğer {} kullanılıyorsa
    consume(TokenType::DEDENT, "Expect dedent after match cases."); // Eğer girinti kullanılıyorsa

    return match_stmt;
}

// Match içindeki case'leri ayrıştırır
std::unique_ptr<MatchCase> Parser::parseMatchCase() {
     // case pattern: body
     consume(TokenType::CASE, "Expect 'case' keyword.");
     int line = currentToken().line; // 'case' tokenının konumu
     int column = currentToken().column;

     auto pattern_expr = parseExpression(); // Pattern'ı bir ifade olarak ayrıştır (şimdilik basitleştirilmiş)
     consume(TokenType::COLON, "Expect ':' after match pattern.");

     // Body kısmını ayrıştır (tek deyim veya blok)
     auto body_stmt = parseStatement();

     return std::make_unique<MatchCase>(std::move(pattern_expr), std::move(body_stmt), line, column);
}


// If deyimini ayrıştırır
std::unique_ptr<Statement> Parser::parseIfStatement() {
    int line = currentToken().line; // 'if' tokenının konumu
    int column = currentToken().column;

    consume(TokenType::LPAREN, "Expect '(' after 'if'.");
    auto condition = parseExpression(); // Koşul ifadesini ayrıştır
    consume(TokenType::RPAREN, "Expect ')' after if condition.");

    // If gövdesini ayrıştır (tek deyim veya blok)
    auto then_branch = parseStatement();

    // Else veya Elif var mı kontrol et
    std::unique_ptr<Statement> else_branch = nullptr;
    if (match(TokenType::ELSE)) {
        // Else gövdesini ayrıştır (tek deyim veya blok). Elif de burada else içindeki bir if olarak ele alınır.
        else_branch = parseStatement();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch), std::move(else_branch), line, column);
}

// While deyimini ayrıştırır
std::unique_ptr<Statement> Parser::parseWhileStatement() {
    int line = currentToken().line; // 'while' tokenının konumu
    int column = currentToken().column;

    consume(TokenType::LPAREN, "Expect '(' after 'while'.");
    auto condition = parseExpression(); // Koşul ifadesini ayrıştır
    consume(TokenType::RPAREN, "Expect ')' after while condition.");

    // While gövdesini ayrıştır (tek deyim veya blok)
    auto body = parseStatement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body), line, column);
}

// Fonksiyon tanımını ayrıştırır (def keywordü)
std::unique_ptr<Statement> Parser::parseFunctionDefinition() {
     def functionName(param1, param2): body
    int line = currentToken().line; // 'def' tokenının konumu
    int column = currentToken().column;

    Token name_token = consume(TokenType::IDENTIFIER, "Expect function name after 'def'.");
    consume(TokenType::LPAREN, "Expect '(' after function name.");

    std::vector<Token> parameters;
    if (!check(TokenType::RPAREN)) { // Parametre listesi boş değilse
        do {
            parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (match(TokenType::COMMA)); // Virgüle kadar parametreleri oku
    }
    consume(TokenType::RPAREN, "Expect ')' after parameters.");

    consume(TokenType::COLON, "Expect ':' after function parameters.");

    // Fonksiyon gövdesini ayrıştır (blok)
    auto body = parseBlockStatement(); // Fonksiyon gövdesi mutlaka blok olmalı

    return std::make_unique<DefStmt>(name_token, parameters, std::move(body));
}


// Sınıf tanımını ayrıştırır (class keywordü)
std::unique_ptr<Statement> Parser::parseClassDefinition() {
     class ClassName: body
     class ClassName(SuperClass): body
    int line = currentToken().line; // 'class' tokenının konumu
    int column = currentToken().column;

    Token name_token = consume(TokenType::IDENTIFIER, "Expect class name after 'class'.");

    // Miras (inheritance) parantezleri burada ayrıştırılabilir
     if (match(TokenType::LPAREN)) {
          Superclass(lar) ayrıştırılır
         consume(TokenType::RPAREN, "Expect ')' after superclasses.");
     }

    consume(TokenType::COLON, "Expect ':' after class name.");

    // Sınıf gövdesini ayrıştır (blok)
    auto body_block = parseBlockStatement(); // Sınıf gövdesi mutlaka blok olmalı

    // Bloktan Statement listesini ClassDeclStmt'e aktar
    std::vector<std::unique_ptr<Statement>> body_stmts;
     for(auto& stmt : body_block->statements) {
         body_stmts.push_back(std::move(stmt));
     }


    return std::make_unique<ClassDeclStmt>(name_token, body_stmts);
}

// Return deyimini ayrıştırır
std::unique_ptr<Statement> Parser::parseReturnStatement() {
     int line = currentToken().line; // 'return' tokenının konumu
     int column = currentToken().column;
     match(TokenType::RETURN); // return tokenını tüket

     std::unique_ptr<Expression> value = nullptr;
     // Eğer dosya sonu veya deyim sonu (yeni satır/;) değilse, dönüş değeri vardır.
     // Python tarzı satır sonu bazlı bitişi varsayalım:
     if (!isAtEnd() && !check(TokenType::NEWLINE)) { // Eğer lexer NEWLINE tokenı veriyorsa
     if (!isAtEnd()) { // Basitçe dosya sonu değilse
         value = parseExpression(); // Dönüş değeri ifadesini ayrıştır
     }

     // Eğer C gibi noktalı virgül varsa:
      consume(TokenType::SEMICOLON, "Expect ';' after return value.");

     return std::make_unique<ReturnStmt>(std::move(value), line, column);
}

// Break ve Continue deyimlerini ayrıştırır
std::unique_ptr<Statement> Parser::parseBreakStatement() {
    int line = currentToken().line;
    int column = currentToken().column;
    match(TokenType::BREAK);
    // Eğer noktalı virgül varsa: consume(TokenType::SEMICOLON, "Expect ';' after break.");
    return std::make_unique<BreakStmt>(line, column);
}

std::unique_ptr<Statement> Parser::parseContinueStatement() {
    int line = currentToken().line;
    int column = currentToken().column;
    match(TokenType::CONTINUE);
    consume(TokenType::SEMICOLON, "Expect ';' after continue.");
    return std::make_unique<ContinueStmt>(line, column);
}


// --- İfade Ayrıştırma Fonksiyonları (Öncelik Sırasına Göre) ---

// En genel ifade (genellikle atama veya düşük öncelikli operatörler)
std::unique_ptr<Expression> Parser::parseExpression() {
    // C-CUBE'da atama da bir ifade midir? (örn: x = y = 5) veya bir deyim midir?
    // Python'da atama bir deyimdir. Biz de deyim olarak ele alalım (parseVarDeclaration gibi).
    // Bu durumda parseExpression en düşük öncelikli operatörle başlar, o da genellikle mantıksal OR'dur.
    return parseAssignment(); // Veya doğrudan parseOr() ile başlayabilirsiniz
}

// Atama ifadesini ayrıştırır (örn: identifier = expression). Eğer ifade ise.
// Genellikle identifier veya property/index erişimi ile başlar.
std::unique_ptr<Expression> Parser::parseAssignment() {
     // Atama genellikle sağdan sola önceliklidir.
     // Önce sol tarafı (atama hedefi) ayrıştırırız. Bu bir değişken veya özellik erişimi olmalı.
     auto expr = parseOr(); // Veya atama hedefinden bir önceki öncelik seviyesi
     auto expr = parseCallDotOrIndex(); // Eğer sadece değişkenler/özellikler atanabiliyorsa

     if (match(TokenType::ASSIGN)) { // '=' operatörü geldi mi?
          Token assign_token = lexer_.peekToken(); // Atama tokenını al (match consume etti)
          // Sağ tarafı ayrıştır. Atama sağdan sola öncelikli olduğu için yine parseAssignment'ı çağırırız.
          auto value = parseAssignment();

          // Atama hedefi VariableExpr veya GetExpr (obj.prop) olmalı.
          // Burada tip kontrolü veya anlamsal analiz yapılır, şimdilik sadece AST yapısını kuruyoruz.
          if (auto var_expr = dynamic_cast<VariableExpr*>(expr.get())) {
               // Değişken ataması: myVar = value
               // AST'de bunu özel bir atama ifadesi olarak temsil edebiliriz
               // Veya daha sonra anlamsal analizde ele alabiliriz.
               // Şimdilik VariableExpr'ı döndüren ifadeyi, atamanın sol tarafı olarak kabul edip
               // Anlamsal analizde buranın atanabilir olup olmadığını kontrol ederiz.
               // Alternatif olarak, bir AssignmentExpr AST düğümü oluşturabiliriz.
               // Basitlik için şimdilik sadece sol tarafı VariableExpr veya GetExpr ise devam edelim.
               std::cerr << "Warning: Assignment parsing needs proper AST node (e.g., AssignmentExpr)." << std::endl;
                // Şimdilik sadece left=value şeklinde BinaryExpr gibi temsil edelim (semantik olarak yanlış olabilir!)
                return std::make_unique<BinaryExpr>(std::move(expr), assign_token, std::move(value));

          } else if (auto get_expr = dynamic_cast<GetExpr*>(expr.get())) {
               // Özellik ataması: obj.prop = value
                std::cerr << "Warning: Property assignment parsing needs proper AST node (e.g., SetExpr)." << std::endl;
                 return std::make_unique<SetExpr>(std::move(get_expr->object), get_expr->name_token, std::move(value));
          }
          else {
               // Atama hedefi geçerli değil (örn: 1 + 2 = 5)
               throw ParsingError(assign_token, "Invalid assignment target.");
          }
     }

     // Atama operatörü yoksa, sadece sol tarafı ifade olarak döndür
     return expr;
}


// Mantıksal OR (or)
std::unique_ptr<Expression> Parser::parseOr() {
    auto expr = parseAnd(); // Bir sonraki öncelik seviyesini ayrıştır

    while (match(TokenType::OR)) { // 'or' operatörü olduğu sürece
        Token operator_token = lexer_.peekToken(); // Operatör tokenı (match tarafından tüketildi)
        auto right = parseAnd(); // Sağ tarafı ayrıştır
        expr = std::make_unique<BinaryExpr>(std::move(expr), operator_token, std::move(right)); // Yeni bir BinaryExpr düğümü oluştur
    }
    return expr;
}

// Mantıksal AND (and)
std::unique_ptr<Expression> Parser::parseAnd() {
    auto expr = parseEquality(); // Bir sonraki öncelik seviyesini ayrıştır

    while (match(TokenType::AND)) { // 'and' operatörü olduğu sürece
        Token operator_token = lexer_.peekToken(); // Operatör tokenı
        auto right = parseEquality(); // Sağ tarafı ayrıştır
        expr = std::make_unique<BinaryExpr>(std::move(expr), operator_token, std::move(right)); // Yeni bir BinaryExpr düğümü oluştur
    }
    return expr;
}

// Eşitlik (==, !=)
std::unique_ptr<Expression> Parser::parseEquality() {
    auto expr = parseComparison(); // Bir sonraki öncelik seviyesini ayrıştır

    while (match({TokenType::EQ, TokenType::NEQ})) { // == veya != olduğu sürece
        Token operator_token = lexer_.peekToken(); // Operatör tokenı
        auto right = parseComparison(); // Sağ tarafı ayrıştır
        expr = std::make_unique<BinaryExpr>(std::move(expr), operator_token, std::move(right)); // Yeni bir BinaryExpr düğümü oluştur
    }
    return expr;
}

// Karşılaştırma (<, >, <=, >=)
std::unique_ptr<Expression> Parser::parseComparison() {
     auto expr = parseAddition(); // Bir sonraki öncelik seviyesini ayrıştır

     while (match({TokenType::LT, TokenType::GT, TokenType::LTE, TokenType::GTE})) { // <, >, <=, >= olduğu sürece
         Token operator_token = lexer_.peekToken(); // Operatör tokenı
         auto right = parseAddition(); // Sağ tarafı ayrıştır
         expr = std::make_unique<BinaryExpr>(std::move(expr), operator_token, std::move(right)); // Yeni bir BinaryExpr düğümü oluştur
     }
     return expr;
}


// Toplama/Çıkarma (+, -)
std::unique_ptr<Expression> Parser::parseAddition() {
     auto expr = parseMultiplication(); // Bir sonraki öncelik seviyesini ayrıştır

     while (match({TokenType::PLUS, TokenType::MINUS})) { // + veya - olduğu sürece
         Token operator_token = lexer_.peekToken(); // Operatör tokenı
         auto right = parseMultiplication(); // Sağ tarafı ayrıştır
         expr = std::make_unique<BinaryExpr>(std::move(expr), operator_token, std::move(right)); // Yeni bir BinaryExpr düğümü oluştur
     }
     return expr;
}

// Çarpma/Bölme (*, /)
std::unique_ptr<Expression> Parser::parseMultiplication() {
    auto expr = parseUnary(); // Bir sonraki öncelik seviyesini ayrıştır

    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) { // *, /, % olduğu sürece
        Token operator_token = lexer_.peekToken(); // Operatör tokenı
        auto right = parseUnary(); // Sağ tarafı ayrıştır
        expr = std::make_unique<BinaryExpr>(std::move(expr), operator_token, std::move(right)); // Yeni bir BinaryExpr düğümü oluştur
    }
    return expr;
}

// Birli operatörler (-, not)
std::unique_ptr<Expression> Parser::parseUnary() {
     if (match({TokenType::MINUS, TokenType::NOT})) { // - veya not operatörü varsa
         Token operator_token = lexer_.peekToken(); // Operatör tokenı
         auto operand = parseUnary(); // Sağ tarafı (yine bir unary ifade olabilir) ayrıştır
         return std::make_unique<UnaryExpr>(operator_token, std::move(operand)); // UnaryExpr düğümü oluştur
     }
     // Unary operatör yoksa, bir sonraki öncelik seviyesini ayrıştır
     return parseCallDotOrIndex();
}

// Fonksiyon çağrısı, nokta erişimi (metot/özellik), dizi erişimi (sol ilişkili operatörler)
std::unique_ptr<Expression> Parser::parseCallDotOrIndex() {
     auto expr = parsePrimary(); // En yüksek öncelikli ifadelerle başla

     while (true) {
         if (match(TokenType::LPAREN)) { // '(' gelirse fonksiyon/metot çağrısı
             // Argümanları ayrıştır ve yeni CallExpr düğümü oluştur
             auto callee = std::move(expr); // Sol taraf çağrılan ifade olur
             int call_line = currentToken().line; // '(' tokenının konumu
             int call_column = currentToken().column;

             auto call_expr = std::make_unique<CallExpr>(std::move(callee), call_line, call_column);

             // Argüman listesini ayrıştır
             if (!check(TokenType::RPAREN)) { // Argüman listesi boş değilse
                 do {
                     call_expr->arguments.push_back(parseExpression()); // Argüman ifadesini ayrıştır
                 } while (match(TokenType::COMMA)); // Virgüle kadar argümanları oku
             }
             consume(TokenType::RPAREN, "Expect ')' after arguments."); // Kapanış parantezini tüket

             expr = std::move(call_expr); // Yeni ifade artık CallExpr
         } else if (match(TokenType::DOT)) { // '.' gelirse özellik/metot erişimi
             // Sağ taraf bir tanımlayıcı (özellik/metot adı) olmalı
             Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
             // Yeni bir GetExpr düğümü oluştur
              expr = std::make_unique<GetExpr>(std::move(expr), name);
              std::cerr << "Warning: Dot access parsing needs proper AST node (e.g., GetExpr)." << std::endl;
               // Şimdilik basitleştirilmiş gösterim: Noktadan sonraki adı bir VariableExpr yapıp binary gibi birleştir. (Semantik olarak yanlış!)
                auto prop_name_expr = std::make_unique<VariableExpr>(name);
                 expr = std::make_unique<BinaryExpr>(std::move(expr), currentToken(), std::move(prop_name_expr)); // '.' tokenını kullanabilirsiniz

         } else if (match(TokenType::LSQUARE)) { // '[' gelirse dizi/liste erişimi
             // Dizi indeksini ayrıştır ve yeni IndexExpr düğümü oluştur
              auto index_expr = parseExpression();
              consume(TokenType::RSQUARE, "Expect ']' after index.");
              expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index_expr));
              std::cerr << "Warning: Index access parsing needs proper AST node (e.g., IndexExpr)." << std::endl;
               consume(TokenType::RSQUARE, "Expect ']' after index."); // Sadece tüketelim şimdilik
         }
         else {
             break; // Çağrı, nokta veya index operatörü yoksa döngüden çık
         }
     }
     return expr; // Zincirlenmiş çağrıların/erişimlerin sonucunu döndür
}


// En temel ifade (literaller, değişkenler, parantezli ifadeler)
std::unique_ptr<Expression> Parser::parsePrimary() {
    Token current = currentToken(); // Mevcut tokena bak

    switch (current.type) {
        case TokenType::FALSE_KW:
        case TokenType::TRUE_KW:
        case TokenType::NONE_KW:
        case TokenType::INTEGER:
        case TokenType::FLOAT:
        case TokenType::STRING:
            lexer_.getNextToken(); // Literal tokenı tüket
            return std::make_unique<LiteralExpr>(current); // LiteralExpr düğümü oluştur

        case TokenType::IDENTIFIER:
            lexer_.getNextToken(); // Tanımlayıcı tokenı tüket
            return std::make_unique<VariableExpr>(current); // VariableExpr düğümü oluştur

        case TokenType::LPAREN: // Parantezli ifade
            lexer_.getNextToken(); // '(' tokenını tüket
            auto expr = parseExpression(); // Parantez içindeki ifadeyi ayrıştır
            consume(TokenType::RPAREN, "Expect ')' after expression."); // Kapanış parantezini tüket
            // Parantezli ifade için ayrı bir AST düğümüne genellikle gerek yoktur,
            // sadece içindeki ifadenin kendisi döndürülür. Öncelik zaten parantezlerle sağlanmıştır.
            return expr;

        // TODO: Liste ([]), Dictionary ({}) literal parsingi eklenecek.

        default:
            // Beklenmedik token
            throw ParsingError(current, "Expect expression.");
    }
}


} namespace CCube