#ifndef CUBE_COMPILER_AND_LINKER_COMPILER_HPP
#define CUBE_COMPILER_AND_LINKER_COMPILER_HPP

#include <string>
#include <memory>   // unique_ptr için
#include <vector>   // Hata listesi için

// Gerekli derleyici aşamalarının başlık dosyaları
#include "ErrorHandling/error_reporter.hpp" // Hata yönetimi
#include "Syntax/lexer.hpp"             // Sözdizimsel analiz (Tokenization)
#include "Syntax/parser.hpp"            // Ayrıştırma (AST oluşturma)
#include "Syntax/ast.hpp"               // AST düğümleri için
#include "Semantics/semantic_analyzer.hpp" // Anlamsal analiz
#include "Semantics/symbol_table.hpp"   // SymbolTable - SemanticAnalyzer tarafından yönetilir
#include "Data Types/type_system.hpp"   // TypeSystem - Singleton

// TODO: Kod Üretici (Code Generator) başlık dosyasını dahil edin
// Bu sınıfın bir arayüzü veya somut implementasyonu CodeGen klasöründe olacaktır.
// Örnek ileri bildirim:
namespace CCube {
    class CodeGenerator; // CodeGenerator sınıfı ileride CodeGen/code_generator.hpp'de tanımlanacak
}


namespace CCube {

// C-CUBE Derleyicisinin ana sınıfı
// Derleme sürecini (tokenization, parsing, semantic analysis, code generation) yönetir.
class Compiler {
public:
    // Kurucu: Derleyici için başlangıç ayarları yapılabilir (örn: hedef platform, optimizasyon seviyesi)
    Compiler();

    // Yıkıcı
    ~Compiler();

    // Kaynak kodu derler.
    // source_code: Derlenecek C-CUBE kaynak kodu stringi.
    // filename_hint: Hata raporlama için dosya adı ipucu (isteğe bağlı).
    // Başarılı olursa true, derleme hataları oluşursa false döner.
    bool compile(const std::string& source_code, const std::string& filename_hint = "");

    // Derleme sırasında hata oluşup oluşmadığını kontrol eder.
    bool hasErrors() const;

    // Raporlanan hata ve uyarıların listesini döndürür.
    const std::vector<Error>& getErrors() const;

    // TODO: Derleme sonucu oluşan kodu (örn: IR, Assembly, .bloker formatı) döndüren metotlar eklenecek.
    std::string getGeneratedCode() const;
    std::unique_ptr<CompiledModule> getCompiledResult();


private:
    // Derleyici aşamalarının ve yardımcı bileşenlerin sahipliği
    /// unique_ptr kullanarak bellek yönetimini Compiler sınıfına devrediyoruz.
    std::unique_ptr<ErrorReporter> error_reporter_;       // Merkezi hata raporlayıcı
    std::unique_ptr<Lexer> lexer_;                       // Sözdizimsel analizci
    std::unique_ptr<Parser> parser_;                      // Ayrıştırıcı
    std::unique_ptr<SemanticAnalyzer> semantic_analyzer_; // Anlamsal analizci
    std::unique_ptr<CodeGenerator> code_generator_;      // Kod üretici (placeholder)

    // Başarılı parsing sonucu oluşan AST'nin kök düğümü
    std::unique_ptr<Program> ast_;

    // Kopya oluşturmayı ve atamayı yasakla (Derleyici nesnesi genellikle tekil olur)
    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;
};

} namespace CCube

#endif // CUBE_COMPILER_AND_LINKER_COMPILER_HPP