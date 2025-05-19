#ifndef C_CUBE_AST_H
#define C_CUBE_AST_H

#include "token.h"
#include <vector>
#include <memory> // std::unique_ptr veya std::shared_ptr için

// AST Düğümleri için Temel Sınıflar
 class Expr;
 class Stmt;

// Ziyaretçi Deseni (AST üzerinde işlem yapmak için yaygın olarak kullanılır)
 class ExprVisitor { ... };
 class StmtVisitor { ... };


// Örnek AST Düğümü Tanımları (Basitlik için)
 class LiteralExpr : public Expr { ... };
 class BinaryExpr : public Expr { ... };
 class PrintStmt : public Stmt { ... };
 class VarDeclStmt : public Stmt { ... };
 class AssignExpr : public Expr { ... };
 class VariableExpr : public Expr { ... };
 class BlockStmt : public Stmt { ... }; // { ... } blokları için
 class IfStmt : public Stmt { ... };
 class MatchStmt : public Stmt { ... }; // Match ifadesi için özel düğüm

// Paylaşılan işaretçiler kullanmak belleği yönetmek için başlangıçta faydalı olabilir
 using ExprPtr = std::shared_ptr<Expr>;
 using StmtPtr = std::shared_ptr<Stmt>;

// İleri bildirimler
class Interpreter; // Interpreter sınıfını burada kullanacağımız için

// Deyim (Statement) Ziyaretçisi
class StmtVisitor {
public:
    virtual void visitExpressionStmt(const struct ExpressionStmt& stmt) = 0;
    virtual void visitPrintStmt(const struct PrintStmt& stmt) = 0;
    virtual void visitVarDeclStmt(const struct VarDeclStmt& stmt) = 0;
    virtual void visitBlockStmt(const struct BlockStmt& stmt) = 0;
    virtual void visitIfStmt(const struct IfStmt& stmt) = 0;
    virtual void visitWhileStmt(const struct WhileStmt& stmt) = 0;
    virtual void visitFunDeclStmt(const struct FunDeclStmt& stmt) = 0; // Fonksiyon tanımı
    virtual void visitReturnStmt(const struct ReturnStmt& stmt) = 0;
    virtual void visitClassDeclStmt(const struct ClassDeclStmt& stmt) = 0; // Sınıf tanımı
    virtual void visitImportStmt(const struct ImportStmt& stmt) = 0; // Import deyimi
    virtual void visitMatchStmt(const struct MatchStmt& stmt) = 0; // Match deyimi

    virtual ~StmtVisitor() = default; // Sanal yıkıcı
};

// İfade (Expression) Ziyaretçisi
class ExprVisitor {
public:
    virtual ValuePtr visitAssignExpr(const struct AssignExpr& expr) = 0;
    virtual ValuePtr visitBinaryExpr(const struct BinaryExpr& expr) = 0;
    virtual ValuePtr visitCallExpr(const struct CallExpr& expr) = 0; // Fonksiyon/metot çağrısı
    virtual ValuePtr visitGetExpr(const struct GetExpr& expr) = 0; // Obje özelliği alma
    virtual ValuePtr visitGroupingExpr(const struct GroupingExpr& expr) = 0; // Parantez içindeki ifade
    virtual ValuePtr visitLiteralExpr(const struct LiteralExpr& expr) = 0;
    virtual ValuePtr visitLogicalExpr(const struct LogicalExpr& expr) = 0; // and, or
    virtual ValuePtr visitSetExpr(const struct SetExpr& expr) = 0; // Obje özelliği atama
    virtual ValuePtr visitSuperExpr(const struct SuperExpr& expr) = 0; // Super çağrısı
    virtual ValuePtr visitThisExpr(const struct ThisExpr& expr) = 0; // this anahtar kelimesi
    virtual ValuePtr visitUnaryExpr(const struct UnaryExpr& expr) = 0; // -, !
    virtual ValuePtr visitVariableExpr(const struct VariableExpr& expr) = 0; // Değişken kullanımı
    virtual ValuePtr visitMatchExpr(const struct MatchExpr& expr) = 0; // Match ifadesi (eğer expression olarak da kullanılacaksa)

    virtual ~ExprVisitor() = default; // Sanal yıkıcı
};

// Her AST düğüm sınıfına bir 'accept' metodu ekleyin
/*
struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) const = 0;
};

struct Expr {
    virtual ~Expr() = default;
    virtual ValuePtr accept(ExprVisitor& visitor) const = 0;
};

// Örnek: Print Deyimi
struct PrintStmt : public Stmt {
    ExprPtr expression; // Yazdırılacak ifade

    PrintStmt(ExprPtr expression) : expression(std::move(expression)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visitPrintStmt(*this);
    }
};

// Örnek: İkili Operatör İfadesi
struct BinaryExpr : public Expr {
    ExprPtr left;
    Token op; // Operatör token'ı
    ExprPtr right;

    BinaryExpr(ExprPtr left, Token op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    ValuePtr accept(ExprVisitor& visitor) const override {
        return visitor.visitBinaryExpr(*this);
    }
};

// ... Diğer tüm düğüm sınıfları için accept metodu ekleyin ...


 using StatementList = std::vector<StmtPtr>;
 using ExpressionPtr = std::shared_ptr<Expr>; // ExprPtr kullanılıyorsa

#endif // C_CUBE_AST_H
