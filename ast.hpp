#ifndef CUBE_SYNTAX_AST_HPP
#define CUBE_SYNTAX_AST_HPP

#include <vector>
#include <string>
#include <memory> // std::unique_ptr için
#include "token.hpp" // Konum bilgisi için

namespace CCube {

// Tüm AST düğümleri için temel sınıf
struct ASTNode {
    // Hata raporlama için kaynak kodundaki konumu saklayabiliriz
    int line;
    int column;

    ASTNode(int line, int column) : line(line), column(column) {}

    // Sanal yıkıcı, türetilmiş sınıfların doğru şekilde temizlenmesini sağlar
    virtual ~ASTNode() = default;

    // AST üzerinde dolaşmak için bir ziyaretçi (visitor) deseni metodu eklenebilir.
     virtual void accept(Visitor& visitor) = 0;
};

// ------------ İfadeler (Expressions) ------------
// Bir değer üreten AST düğümleri
struct Expression : public ASTNode {
    Expression(int line, int column) : ASTNode(line, column) {}
    virtual ~Expression() = default;
};

// Literal değerler (sayı, string, boolean, None)
struct LiteralExpr : public Expression {
    Token value_token; // Değeri ve tipini içeren token

    LiteralExpr(const Token& token) : Expression(token.line, token.column), value_token(token) {}
};

// Değişken referansı
struct VariableExpr : public Expression {
    Token name_token; // Değişken adını içeren token

    VariableExpr(const Token& token) : Expression(token.line, token.column), name_token(token) {}
};

// İkili Operatör İfadeleri (örn: a + b)
struct BinaryExpr : public Expression {
    std::unique_ptr<Expression> left;
    Token operator_token; // Operatör tokenı (+, -, == vb.)
    std::unique_ptr<Expression> right;

    BinaryExpr(std::unique_ptr<Expression> left, const Token& op, std::unique_ptr<Expression> right)
        : Expression(left ? left->line : op.line, left ? left->column : op.column), // Başlangıç konumunu sol ifade veya operatörden al
          left(std::move(left)), // unique_ptr sahipliğini transfer et
          operator_token(op),
          right(std::move(right)) {}
};

// Birli Operatör İfadeleri (örn: -a, not b)
struct UnaryExpr : public Expression {
    Token operator_token; // Operatör tokenı (-, not)
    std::unique_ptr<Expression> operand;

    UnaryExpr(const Token& op, std::unique_ptr<Expression> operand)
        : Expression(op.line, op.column),
          operator_token(op),
          operand(std::move(operand)) {}
};

// Fonksiyon veya metot çağrısı
struct CallExpr : public Expression {
    std::unique_ptr<Expression> callee; // Çağrılan ifade (fonksiyon adı veya metot)
    std::vector<std::unique_ptr<Expression>> arguments; // Argüman listesi

    CallExpr(std::unique_ptr<Expression> callee, int line, int column)
        : Expression(line, column), callee(std::move(callee)) {} // Konumu parantezden alabilirsiniz
};


// ------------ Deyimler (Statements) ------------
// Bir aksiyonu temsil eden AST düğümleri
struct Statement : public ASTNode {
    Statement(int line, int column) : ASTNode(line, column) {}
    virtual ~Statement() = default;
};

// İfade olarak kullanılan deyim (örn: sadece bir fonksiyon çağrısı)
struct ExpressionStmt : public Statement {
    std::unique_ptr<Expression> expr;

    ExpressionStmt(std::unique_ptr<Expression> expr)
        : Statement(expr ? expr->line : 0, expr ? expr->column : 0), expr(std::move(expr)) {}
};

// Değişken bildirimi ve isteğe bağlı atama (örn: myVar = 10)
struct VarDeclStmt : public Statement {
    Token name_token; // Değişken adını içeren token
    std::unique_ptr<Expression> initializer; // Başlangıç değeri ifadesi (opsiyonel)

    VarDeclStmt(const Token& name_token, std::unique_ptr<Expression> initializer)
        : Statement(name_token.line, name_token.column),
          name_token(name_token),
          initializer(std::move(initializer)) {}
};

// Blok deyimi (süslü parantez içindeki kod bloğu veya girinti bloğu)
struct BlockStmt : public Statement {
    std::vector<std::unique_ptr<Statement>> statements;

    // Konum bilgisi için blok başlangıcını belirten token (örn: '{' veya INDENT tokenı)
    BlockStmt(int line, int column) : Statement(line, column) {}
};

// If-Else Ifadesi
struct IfStmt : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> then_branch; // 'if' doğruysa çalışacak kısım
    std::unique_ptr<Statement> else_branch; // 'else' varsa çalışacak kısım (opsiyonel)

    IfStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> then_branch, std::unique_ptr<Statement> else_branch, int line, int column)
        : Statement(line, column),
          condition(std::move(condition)),
          then_branch(std::move(then_branch)),
          else_branch(std::move(else_branch)) {}
};

// While Döngüsü
struct WhileStmt : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    WhileStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body, int line, int column)
        : Statement(line, column),
          condition(std::move(condition)),
          body(std::move(body)) {}
};

// Import Deyimi
struct ImportStmt : public Statement {
    // Modül adları veya Alias'lar için daha karmaşık yapılar gerekebilir
    std::vector<std::string> module_names; // Basitlik için string listesi

    ImportStmt(const std::vector<std::string>& names, int line, int column)
        : Statement(line, column), module_names(names) {}
};

// Match Deyimi
struct MatchStmt : public Statement {
    std::unique_ptr<Expression> value; // Eşleşecek değer ifadesi
    std::vector<std::unique_ptr<MatchCase>> cases; // Case listesi

    MatchStmt(std::unique_ptr<Expression> value, int line, int column)
        : Statement(line, column), value(std::move(value)) {} // Konum 'match' tokenından alınabilir
};

// Match Case (Pattern ve gövde)
struct MatchCase : public ASTNode {
    std::unique_ptr<Expression> pattern; // Eşleşme deseni (şimdilik basit bir ifade)
    std::unique_ptr<Statement> body; // Eşleşirse çalışacak kod bloğu

    MatchCase(std::unique_ptr<Expression> pattern, std::unique_ptr<Statement> body, int line, int column)
        : ASTNode(line, column), pattern(std::move(pattern)), body(std::move(body)) {} // Konum 'case' tokenından alınabilir
};


// Fonksiyon Tanımı
struct DefStmt : public Statement {
    Token name_token; // Fonksiyon adını içeren token
    std::vector<Token> parameters; // Parametre isimlerini içeren tokenlar (Basitlik için)
    std::unique_ptr<BlockStmt> body; // Fonksiyon gövdesi

    DefStmt(const Token& name_token, const std::vector<Token>& params, std::unique_ptr<BlockStmt> body)
        : Statement(name_token.line, name_token.column),
          name_token(name_token),
          parameters(params),
          body(std::move(body)) {}
};

// Sınıf Tanımı
struct ClassDeclStmt : public Statement {
    Token name_token; // Sınıf adını içeren token
    // Miras alınan sınıf(lar) için ifadeler eklenebilir (örn: Python'daki gibi parantez içinde)
     std::vector<std::unique_ptr<Expression>> superclasses;
    std::vector<std::unique_ptr<Statement>> body; // Metotlar ve alanlar (statements/declarations)

    ClassDeclStmt(const Token& name_token, const std::vector<std::unique_ptr<Statement>>& body)
        : Statement(name_token.line, name_token.column),
          name_token(name_token) {
        // Body içeriğini kopyala veya taşı (unique_ptr olduğu için taşıma daha uygun)
        for (auto& stmt : body) {
            this->body.push_back(std::move(stmt));
        }
    }
};

// Return Deyimi
struct ReturnStmt : public Statement {
    std::unique_ptr<Expression> value; // Döndürülen değer ifadesi (opsiyonel)

    ReturnStmt(std::unique_ptr<Expression> value, int line, int column)
        : Statement(line, column), value(std::move(value)) {}
};

// Break ve Continue Deyimleri
struct BreakStmt : public Statement {
    BreakStmt(int line, int column) : Statement(line, column) {}
};

struct ContinueStmt : public Statement {
    ContinueStmt(int line, int column) : Statement(line, column) {}
};


// ------------ Programın Kök Düğümü ------------
// Tüm kaynak dosyasını temsil eden en üst düzey düğüm
struct Program : public ASTNode {
    std::vector<std::unique_ptr<Statement>> statements; // Top-level deyimler ve deklarasyonlar

    // Program düğümünün konumu genellikle 1. satır, 1. sütun olabilir.
    Program() : ASTNode(1, 1) {}
};


} namespace CCube

#endif // CUBE_SYNTAX_AST_HPP