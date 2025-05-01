#ifndef CUBE_SEMANTICS_SYMBOL_TABLE_HPP
#define CUBE_SEMANTICS_SYMBOL_TABLE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory> // shared_ptr ve unique_ptr için
#include "Data Types/type.hpp" // Symbol tipleri için
#include "ErrorHandling/error_reporter.hpp" // Hata raporlama için

namespace CCube {

// Sembolün türü (neyi temsil ediyor?)
enum class SymbolKind {
    UNKNOWN, VARIABLE, PARAMETER, FUNCTION, CLASS, FIELD, METHOD,
    // Gelecekte eklenecekler: MODULE, ENUM, CONST vb.
};

// Deklare edilmiş bir sembolü temsil eden yapı
struct Symbol {
    std::string name;
    SymbolKind kind;
    std::unique_ptr<Type> type; // Sembolün tipi (örn: int, string, FunctionType)
    SourceLocation declaration_location; // Nerede tanımlandığı
    int scope_level; // Hangi kapsam seviyesinde tanımlı (0 = global)

    // İleride eklenecekler: memory_location (codegen için), is_global, is_static vb.

    Symbol(std::string name, SymbolKind kind, std::unique_ptr<Type> type, SourceLocation loc, int level)
        : name(std::move(name)), kind(kind), type(std::move(type)), declaration_location(loc), scope_level(level) {}

     // Hata ayıklama için sembolün string gösterimi
     std::string toString() const {
         return name + " (" + type->toString() + ", " + std::to_string(scope_level) + ")";
     }
};

// Tek bir kapsamı (scope) temsil eden yapı
struct Scope {
    // Sembolleri isimlerine göre saklayan harita.
    // shared_ptr kullanmak, AST düğümlerinin sembollere raw pointer veya weak_ptr ile erişmesini kolaylaştırır.
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols;

    // Üst kapsama referans (kapsam zinciri için)
    Scope* parent_scope;

    Scope(Scope* parent = nullptr) : parent_scope(parent) {}
};

// Kapsam yığınını ve sembol arama/bildirme mantığını yöneten sınıf
class SymbolTable {
public:
    // Kurucu: Hata raporlayıcıyı alır ve global kapsamı oluşturur.
    SymbolTable(ErrorReporter& reporter);

    // Yıkıcı: Kapsamları temizler. unique_ptr vektörü kullanılıyorsa otomatik olur.
    ~SymbolTable() = default;

    // Yeni bir kapsam başlatır (örn: fonksiyon girişi, blok girişi, sınıf gövdesi)
    void enterScope();

    // Mevcut kapsamdan çıkar
    void exitScope();

    // Mevcut kapsama bir sembol ekler.
    // Başarılı olursa true, zaten tanımlıysa false döner ve hata raporlar.
    bool declareSymbol(const std::string& name, SymbolKind kind, std::unique_ptr<Type> type, SourceLocation loc);

    // Sembolü mevcut kapsamdan başlayarak üst kapsamlara doğru arar.
    // Bulursa sembolün shared_ptr'ını, bulamazsa nullptr döndürür.
    std::shared_ptr<Symbol> resolveSymbol(const std::string& name);

     // Mevcut kapsam seviyesini döndürür (Global = 0)
    int getCurrentScopeLevel() const { return current_scope_level_; }

private:
    ErrorReporter& reporter_; // Hata raporlayıcısı

    // Kapsamların tutulduğu vektör. unique_ptr'lar kapsamlara sahip olur.
    std::vector<std::unique_ptr<Scope>> scopes_;

    // Şu anki aktif kapsamın ham işaretçisi (scopes_ vektöründeki bir Scope nesnesine işaret eder)
    Scope* current_scope_;

    int current_scope_level_; // Mevcut kapsamın seviyesi
};

} namespace CCube

#endif // CUBE_SEMANTICS_SYMBOL_TABLE_HPP