#ifndef CUBE_SEMANTICS_SEMANTIC_ANALYZER_HPP
#define CUBE_SEMANTICS_SEMANTIC_ANALYZER_HPP

#include <memory>
#include <vector>
#include "Syntax/ast.hpp" // AST düğümleri için
#include "Semantics/symbol_table.hpp" // Sembol tablosu için
#include "Data Types/type.hpp" // Tip sistemi için
#include "ErrorHandling/error_reporter.hpp" // Hata raporlama için

namespace CCube {

// AST üzerinde semantik analiz yapan sınıf
class SemanticAnalyzer {
public:
    // Kurucu: Ayrıştırılmış AST'nin kök düğümünü ve bir hata raporlayıcıyı alır.
    // AST'yi referans olarak alıyoruz, ownership SemanticAnalyzer'da değil.
    SemanticAnalyzer(Program& ast, ErrorReporter& reporter);

    // Yıkıcı
    ~SemanticAnalyzer() = default;

    // Semantik analiz işlemini başlatır.
    // Analiz sırasında AST düğümlerine tip bilgisi eklenebilir veya hatalar raporlanabilir.
    void analyze();

private:
    Program& ast_; // Analiz edilecek AST'nin referansı
    ErrorReporter& reporter_; // Hata raporlayıcısı
    SymbolTable symbol_table_; // Kapsam ve sembol yönetimi

    // TODO: Tip sistemi yönetimi için bir üye eklenecek
     TypeSystem type_system_; // Tip uyumluluk kontrolleri için

    // Semantik kontroller sırasında takip edilmesi gereken durumlar
    bool in_loop_; // Şu anda bir döngü (while, for) içinde miyiz? (break/continue kontrolü için)
     Type* current_function_return_type_; // Şu anki fonksiyonun dönüş tipi (return kontrolü için)


    // AST üzerinde dolaşarak semantik analiz yapan yardımcı (private) fonksiyonlar
    // Her fonksiyon ilgili AST düğüm türünü işler.

    // Program seviyesini analiz eder
    void analyzeProgram(Program& program);

    // Genel bir deyimi analiz eder
    void analyzeStatement(Statement& stmt);

    // İfade deyimini analiz eder
    void analyzeExpressionStatement(ExpressionStmt& stmt);

    // Değişken bildirimi deyimini analiz eder
    void analyzeVarDeclaration(VarDeclStmt& stmt);

    // Blok deyimini analiz eder (yeni kapsam oluşturur)
    void analyzeBlockStatement(BlockStmt& stmt);

    // Import deyimini analiz eder (modülleri çözümleme - karmaşık olabilir)
    void analyzeImportStatement(ImportStmt& stmt);

    // Match deyimini analiz eder
    void analyzeMatchStatement(MatchStmt& stmt);
    void analyzeMatchCase(MatchCase& match_case);

    // If deyimini analiz eder
    void analyzeIfStatement(IfStmt& stmt);

    // While deyimini analiz eder
    void analyzeWhileStatement(WhileStmt& stmt);

    // Fonksiyon tanımını analiz eder (yeni kapsam oluşturur)
    void analyzeFunctionDefinition(DefStmt& stmt);

    // Sınıf tanımını analiz eder (yeni kapsam oluşturur)
    void analyzeClassDefinition(ClassDeclStmt& stmt);

    // Return deyimini analiz eder (dönüş tipi kontrolü)
    void analyzeReturnStatement(ReturnStmt& stmt);

    // Break deyimini analiz eder (döngü içinde mi kontrolü)
    void analyzeBreakStatement(BreakStmt& stmt);

    // Continue deyimini analiz eder (döngü içinde mi kontrolü)
    analyzeContinueStatement(ContinueStmt& stmt);


    // İfadeleri analiz eder ve tiplerini belirler
    // İfade fonksiyonları genellikle ifadenin belirlenen tipini döndürür veya AST düğümüne ekler.
    // Bu fonksiyon, AST düğümüne resolved_type ekler ve/veya tipin unique_ptr'ını döndürür.
    std::unique_ptr<Type> analyzeExpression(Expression& expr);

    // Literal ifadeyi analiz eder
    std::unique_ptr<Type> analyzeLiteralExpr(LiteralExpr& expr);

    // Değişken ifadesini analiz eder (isim çözümleme)
    std::unique_ptr<Type> analyzeVariableExpr(VariableExpr& expr);

    // İkili operatör ifadesini analiz eder (tip uyumluluğu kontrolü)
    std::unique_ptr<Type> analyzeBinaryExpr(BinaryExpr& expr);

    // Birli operatör ifadesini analiz eder (tip uyumluluğu kontrolü)
    std::unique_ptr<Type> analyzeUnaryExpr(UnaryExpr& expr);

    // Fonksiyon/metot çağrısı ifadesini analiz eder (isim çözümleme, argüman kontrolü, dönüş tipi)
    std::unique_ptr<Type> analyzeCallExpr(CallExpr& expr);

    // TODO: Diğer ifade türleri için analiz fonksiyonları eklenecek (GetExpr, SetExpr, IndexExpr vb.)

};

} namespace CCube

#endif // CUBE_SEMANTICS_SEMANTIC_ANALYZER_HPP