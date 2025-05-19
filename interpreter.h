#ifndef C_CUBE_INTERPRETER_H
#define C_CUBE_INTERPRETER_H

#include "ast.h"
#include "environment.h" // Değişken ortamı için
#include "value.h" // Runtime değerleri için

// Yorumlayıcı genellikle AST ziyaretçisi olarak implement edilir
 class Interpreter : public ExprVisitor, public StmtVisitor { ... };

class Interpreter {
private:
    // Global değişken ortamı
    EnvironmentPtr globals = std::make_shared<Environment>();

    // Şu anki değişken ortamı (skop yönetimi için)
    EnvironmentPtr environment = globals;

    // Yorumlama sırasında kullanılan yardımcı metodlar
     ValuePtr evaluate(ExprPtr expr);
     void execute(StmtPtr stmt);
     void executeBlock(const std::vector<StmtPtr>& statements, EnvironmentPtr environment);
     ValuePtr lookUpVariable(const Token& name, ExprPtr expr); // Değişken arama

    // Ziyaretçi metodlarının implementasyonları (eğer visitor deseni kullanılıyorsa)
     ValuePtr visitLiteralExpr(const LiteralExpr& expr) override;
     ValuePtr visitBinaryExpr(const BinaryExpr& expr) override;
     void visitPrintStmt(const PrintStmt& stmt) override;
    // ... Diğer düğümler için metodlar ...

public:
    Interpreter(); // Kurulum (global fonksiyonları tanımlama vb.)
    void interpret(const std::vector<StmtPtr>& statements);

    // Hata raporlama
     void runtimeError(const RuntimeError& error);
};

#endif // C_CUBE_INTERPRETER_H
