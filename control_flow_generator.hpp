#ifndef CUBE_CONTROL_STRUCTURES_CONTROL_FLOW_GENERATOR_HPP
#define CUBE_CONTROL_STRUCTURES_CONTROL_FLOW_GENERATOR_HPP

#include <vector>
#include <stack> // Döngü etiketlerini yönetmek için
#include <string>
#include <memory> // unique_ptr için (AST düğümleri)

#include "Syntax/ast.hpp" // AST düğümleri için
#include "ErrorHandling/error_reporter.hpp" // Hata raporlama için

// --- Kod Üretici (Code Generator) için Placeholder Tanımlar ---
// ControlFlowGenerator'ın ihtiyaç duyduğu minimal arayüzü temsil eder.
// Gerçek CodeGenerator sınıfı ve Label yapısı CodeGen modülünde tanımlanacaktır.

namespace CCube {

// Etiketleri temsil eden basit yapı (CodeGenerator tarafından yönetilir)
struct Label {
    std::string name_hint; // Hata ayıklama için etiket adı ipucu
    // CodeGenerator'ın kendi içindeki etiket karşılığı (örn: LLVM BasicBlock*, Register*, Assembly Label Adı)
    void* backend_label = nullptr;

    Label(const std::string& hint = "") : name_hint(hint) {}
    // Yıkıcı, backend kaynağını temizlememeli, CodeGenerator yönetmeli
    ~Label() = default;
};

// CodeGenerator için minimalist arayüz (ControlFlowGenerator'ın ihtiyaç duyduğu kadar)
// Gerçek CodeGenerator CodeGen klasöründe detaylandırılacaktır.
class CodeGenerator {
public:
    // Sanal yıkıcı
    virtual ~CodeGenerator() = default;

    // Yeni bir etiket oluşturur
    virtual std::unique_ptr<Label> createLabel(const std::string& name_hint = "") = 0;

    // Verilen etikete atlar (koşulsuz dallanma)
    virtual void emitUnconditionalBranch(Label* target_label) = 0;

    // Koşullu dallanma: Condition ifadesinin sonucuna göre true_label veya false_label'a atlar.
    // Condition ifadesinin kodunu üretmek bu metodun veya ayrı bir metodun sorumluluğudur.
    // Basitlik için burada condition ifadesinin zaten bool tipinde olduğu ve CodeGenerator'ın bunu işleyebildiği varsayılır.
    virtual void emitConditionalBranch(Expression& condition_expr, Label* true_label, Label* false_label) = 0;

    // Bir etiketi mevcut kod akışına yerleştirir
    virtual void emitLabel(Label* label) = 0;

    // İfade kodunu üretir (bu metot rekürsif olarak alt ifadeleri üretir)
    // Bu metot ana CodeGenerator sınıfında olmalıdır, ControlFlowGenerator onu çağırır.
     virtual void generateExpression(Expression& expr) = 0;

    // Deyim kodunu üretir (bu metot rekürsif olarak alt deyimleri üretir veya ControlFlowGenerator'ı çağırır)
     virtual void generateStatement(Statement& stmt) = 0;

    // Fonksiyonun dönüş kodunu üretir (opsiyonel dönüş değeri ile)
    virtual void emitReturn(Expression* return_value_expr) = 0; // return_value_expr nullptr olabilir

    // TODO: Break ve Continue için doğrudan emit metotları eklenebilir veya ControlFlowGenerator kendi dallanmayı yönetir.
     virtual void emitBreak(Label* break_target) = 0;
     virtual void emitContinue(Label* continue_target) = 0;
};

}  namespace CCube

// --- ControlFlowGenerator Tanımı ---

namespace CCube {

// AST'deki kontrol yapısı deyimlerini hedef koda çeviren sınıf
class ControlFlowGenerator {
public:
    // Kurucu: Kullanılacak CodeGenerator ve ErrorReporter referanslarını alır.
    ControlFlowGenerator(CodeGenerator& code_gen, ErrorReporter& reporter);

    // Yıkıcı
    ~ControlFlowGenerator() = default;

    // Belirli kontrol yapısı deyimlerinin kodunu üreten metotlar.
    // Bu metotlar, CodeGenerator'ın emit* metotlarını kullanarak hedef kodu oluşturur.

    // Blok deyiminin kodunu üretir (yeni kapsamı ve içindeki deyimleri işler)
    void generateBlockStatement(BlockStmt& stmt);

    // If deyiminin kodunu üretir
    void generateIfStatement(IfStmt& stmt);

    // While deyiminin kodunu üretir
    void generateWhileStatement(WhileStmt& stmt);

    // Match deyiminin kodunu üretir (karmaşık, dallanmalar ve desen eşleştirme içerir)
    void generateMatchStatement(MatchStmt& stmt);

    // Break deyiminin kodunu üretir
    void generateBreakStatement(BreakStmt& stmt);

    // Continue deyiminin kodunu üretir
    void generateContinueStatement(ContinueStmt& stmt);

    // Return deyiminin kodunu üretir
    void generateReturnStatement(ReturnStmt& stmt);

    // TODO: For döngüsü gibi diğer kontrol yapıları buraya eklenecek.
     void generateForStatement(ForStmt& stmt);


private:
    CodeGenerator& code_gen_; // Gerçek kod üretimini yapan CodeGenerator referansı
    ErrorReporter& reporter_; // Hata raporlayıcısı

    // Break ve Continue deyimlerinin hedefleyeceği etiketleri saklayan yığınlar.
    // İç içe geçmiş döngülerde doğru etiketi bulmak için kullanılır.
    // shared_ptr kullanmak, etiketlerin ömrünü yöneticisinin (CodeGenerator?) kontrolünde tutar.
    // Veya CodeGenerator Label* döndürüyorsa ham pointer saklanır (tehlikeli olabilir).
    // CodeGenerator'ın unique_ptr döndürdüğünü ve burada ham pointer veya shared_ptr tuttuğumuzu varsayalım.
     shared_ptr daha güvenli olabilir eğer başka yerler de aynı etiketi referans alıyorsa.
    // Basitlik için Label* ham pointerlarını yığında saklayalım, ancak CodeGenerator'ın Label'ların ömrünü yönettiğini varsayalım.
    std::stack<Label*> break_targets_;
    std::stack<Label*> continue_targets_;

    // AST'deki genel bir deyim için kod üretme (CodeGenerator'a passthrough veya burada dağıtıcı)
    // Bu genellikle ana CodeGenerator'da olur, ancak burada yardımcı bir wrapper olabilir.
    void generateStatement(Statement& stmt); // Bu metot, generateBlockStatement içinde kullanılabilir
};

} namespace CCube

#endif // CUBE_CONTROL_STRUCTURES_CONTROL_FLOW_GENERATOR_HPP