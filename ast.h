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

// ... Diğer AST düğümü tanımları ...

#endif // C_CUBE_AST_H
