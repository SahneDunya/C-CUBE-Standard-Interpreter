#ifndef C_CUBE_INTERPRETER_H
#define C_CUBE_INTERPRETER_H

#include <vector>
#include <string>
#include <memory>
#include <stdexcept> // std::runtime_error için

// Proje bağımlılıkları
#include "ast.h"            // AST düğümleri ve Visitor arayüzleri
#include "token.h"          // Token tipleri
#include "value.h"          // C-CUBE değer tipleri
#include "environment.h"    // Ortam (scope) yönetimi
#include "error_reporter.h" // Hata raporlama
#include "callable.h"       // Çağrılabilir nesneler (fonksiyonlar, sınıflar) için temel sınıf
#include "function.h"       // Fonksiyonlar için C-CUBE sınıfı
#include "object.h"         // C-CUBE obje sınıfı
#include "class.h"          // C-CUBE sınıf sınıfı
#include "builtin_functions.h" // Yerleşik fonksiyonlar
#include "gc.h"             // Çöp toplayıcı
#include "c_cube_module.h"  // C-CUBE modül tipi
#include "module_loader.h"  // Modül yükleme mekanizması
#include "utils.h"          // Yardımcı fonksiyonlar (örn. valueToString)

// Fonksiyon dönüşlerini işlemek için özel exception
// Bu dosyanın ayrı bir 'return_exception.h' olarak tanımlandığını varsayıyoruz.
 #include "return_exception.h" // Eğer ayrı bir dosya ise

// Eğer ReturnException ayrı bir dosya değilse ve buraya dahil edeceksek:
class ReturnException : public std::runtime_error {
public:
    Value value;
    ReturnException(Value value) : std::runtime_error("ReturnException"), value(std::move(value)) {}
};


class Interpreter : public ExprVisitor<Value>, public StmtVisitor<void> {
private:
    // Global ortam. Tüm programın genel değişkenlerini ve fonksiyonlarını tutar.
    std::shared_ptr<Environment> globals;
    // Mevcut yürütme ortamı. Fonksiyon çağrıları ve bloklar için değişir.
    std::shared_ptr<Environment> environment;
    ErrorReporter& errorReporter;
    Gc& gc; // Çöp toplayıcıya referans
    ModuleLoader& moduleLoader; // Modül yükleyiciye referans

    // Çözümleyici tarafından belirlenen lokal değişken mesafelerini tutmak için (Opsiyonel: Resolver varsa)
     std::unordered_map<Expr*, int> locals; // Bu, AST düğümlerine pointer kullanmayı gerektirir.
                                          // Daha güvenli: std::unordered_map<const Token*, int, TokenHasher> (Eğer Token hashing özelliği eklendiyse)

    // Yardımcı metotlar
    Value evaluate(ExprPtr expr);        // Bir ifadeyi değerlendirir
    void execute(StmtPtr stmt);          // Bir bildirimi yürütür
    bool isTruthy(const Value& value);   // Bir değerin doğruluk değerini kontrol eder
    bool isEqual(const Value& a, const Value& b); // İki değerin eşitliğini kontrol eder
    void checkNumberOperand(const Token& op, const Value& operand); // Sayı operandı kontrolü (tekli)
    void checkNumberOperands(const Token& op, const Value& left, const Value& right); // Sayı operandı kontrolü (ikili)

    // Çalışma zamanı hatası fırlatır
    RuntimeException runtimeError(const Token& token, const std::string& message);

    // Ortam yönetimi için özel metotlar
    void executeBlock(const std::vector<StmtPtr>& statements, std::shared_ptr<Environment> newEnvironment);

    // Değişken çözümlemesi (Eğer Resolver yoksa, Interpreter kendisi yapar)
    Value lookUpVariable(const Token& name);
    // Veya eğer Resolver varsa ve `locals` map'ini kullanıyorsa:
     Value lookUpVariable(const Token& name, ExprPtr expr);

public:
    // Constructor
    Interpreter(ErrorReporter& reporter, Gc& gc_instance, ModuleLoader& loader);

    // Programı yorumlamaya başlar
    void interpret(const std::vector<StmtPtr>& statements);

    // --- ExprVisitor Metodları (ifadeleri değerlendirme) ---
    Value visitBinaryExpr(std::shared_ptr<BinaryExpr> expr) override;
    Value visitCallExpr(std::shared_ptr<CallExpr> expr) override;
    Value visitGetExpr(std::shared_ptr<GetExpr> expr) override;
    Value visitGroupingExpr(std::shared_ptr<GroupingExpr> expr) override;
    Value visitLiteralExpr(std::shared_ptr<LiteralExpr> expr) override;
    Value visitLogicalExpr(std::shared_ptr<LogicalExpr> expr) override;
    Value visitSetExpr(std::shared_ptr<SetExpr> expr) override;
    Value visitSuperExpr(std::shared_ptr<SuperExpr> expr) override;
    Value visitThisExpr(std::shared_ptr<ThisExpr> expr) override;
    Value visitUnaryExpr(std::shared_ptr<UnaryExpr> expr) override;
    Value visitVariableExpr(std::shared_ptr<VariableExpr> expr) override;
    Value visitListLiteralExpr(std::shared_ptr<ListLiteralExpr> expr) override;

    // --- StmtVisitor Metodları (bildirimleri yürütme) ---
    void visitBlockStmt(std::shared_ptr<BlockStmt> stmt) override;
    void visitClassStmt(std::shared_ptr<ClassStmt> stmt) override;
    void visitExprStmt(std::shared_ptr<ExprStmt> stmt) override;
    void visitFunStmt(std::shared_ptr<FunStmt> stmt) override;
    void visitIfStmt(std::shared_ptr<IfStmt> stmt) override;
    void visitImportStmt(std::shared_ptr<ImportStmt> stmt) override;
    void visitReturnStmt(std::shared_ptr<ReturnStmt> stmt) override;
    void visitVarStmt(std::shared_ptr<VarStmt> stmt) override;
    void visitWhileStmt(std::shared_ptr<WhileStmt> stmt) override;
    void visitMatchStmt(std::shared_ptr<MatchStmt> stmt) override;

    // Genel REPL çıktıları için (eğer interpret REPL'de tek ifade yürütüyorsa)
    void printValue(const Value& value);
};

#endif // C_CUBE_INTERPRETER_H
