#include "control_flow_generator.hpp"
#include <iostream> // Hata ayıklama için

namespace CCube {

// ControlFlowGenerator Kurucu
ControlFlowGenerator::ControlFlowGenerator(CodeGenerator& code_gen, ErrorReporter& reporter)
    : code_gen_(code_gen), reporter_(reporter) {
    // Başlangıçta etiket yığınları boş
}

// AST'deki genel bir deyim için kod üretme dağıtıcısı
// Bu fonksiyon, gelen Statement türüne göre ilgili generate* fonksiyonunu çağırır.
// Ana CodeGenerator sınıfı da bu mantığı kullanabilir.
void ControlFlowGenerator::generateStatement(Statement& stmt) {
     // dynamic_cast kullanıyoruz (alternatif Visitor deseni)
    if (auto* s = dynamic_cast<BlockStmt*>(&stmt)) {
        generateBlockStatement(*s);
    } else if (auto* s = dynamic_cast<IfStmt*>(&stmt)) {
        generateIfStatement(*s);
    } else if (auto* s = dynamic_cast<WhileStmt*>(&stmt)) {
        generateWhileStatement(*s);
    } else if (auto* s = dynamic_cast<MatchStmt*>(&stmt)) {
        generateMatchStatement(*s);
    } else if (auto* s = dynamic_cast<BreakStmt*>(&stmt)) {
        generateBreakStatement(*s);
    } else if (auto* s = dynamic_cast<ContinueStmt*>(&stmt)) {
        generateContinueStatement(*s);
    } else if (auto* s = dynamic_cast<ReturnStmt*>(&stmt)) {
        generateReturnStatement(*s);
    }
    // TODO: Diğer deyim türleri (VarDeclStmt, ExpressionStmt vb.) burada ele alınmalı
    // VarDeclStmt ve ExpressionStmt genellikle doğrudan ana CodeGenerator tarafından işlenir,
    // ControlFlowGenerator sadece kontrol akışını yönetir.
    // Ancak BlockStmt içindeki tüm deyimler generateStatement ile çağrıldığı için,
    // bu dağıtıcıda tüm deyim türlerinin ele alınması gerekebilir.
    else if (auto* s = dynamic_cast<ExpressionStmt*>(&stmt)) {
        // Bir ifade deyimi için ifade kodunu üret (CodeGenerator'a pasla)
        // code_gen_.generateExpression(*s->expr);
         reporter_.reportWarning({stmt.line, stmt.column}, "ExpressionStatement generation not fully implemented here.");
    } else if (auto* s = dynamic_cast<VarDeclStmt*>(&stmt)) {
         // Değişken bildirimi kodunu üret (CodeGenerator'a pasla)
          code_gen_.generateVarDeclaration(*s); // Varsayılan CodeGenerator metodunu çağır
         reporter_.reportWarning({stmt.line, stmt.column}, "VarDeclarationStatement generation not fully implemented here.");
    }
     // TODO: Fonksiyon, Sınıf tanımları gibi üst düzey deklarasyonlar genellikle burada ele alınmaz,
     // ana CodeGenerator'ın program seviyesi fonksiyonları tarafından işlenir.
    else {
        // Bilinmeyen veya desteklenmeyen deyim türü
         reporter_.reportError({stmt.line, stmt.column}, "Internal Error: Unknown statement type encountered during control flow generation.");
    }
}


// Blok deyiminin kodunu üretir (yeni kapsamı ve içindeki deyimleri işler)
void ControlFlowGenerator::generateBlockStatement(BlockStmt& stmt) {
    // TODO: Kapsam yönetimi Kod Üretimi aşamasında da gerekiyorsa (örn: stack frame boyutunu belirleme)
    // burada CodeGenerator'a enter/exit scope sinyali gönderilebilir.
     code_gen_.enterScope(); // Placeholder

    // Blok içindeki her deyim için kod üret
    for (auto& child_stmt : stmt.statements) {
        generateStatement(*child_stmt); // generateStatement dağıtıcısını çağır
    }

    // TODO: Kapsamdan çıkış sinyali gönderilir
     code_gen_.exitScope(); // Placeholder
}


// If deyiminin kodunu üretir
void ControlFlowGenerator::generateIfStatement(IfStmt& stmt) {
    // Etiketleri oluştur
    // unique_ptr kullanmak, etiketlerin ömrünün generateIfStatement sonlandığında bitmesini sağlar,
    // ancak bu etiketlere CodeGenerator içindeki dallanmalar hala işaret ediyor olabilir.
    // CodeGenerator'ın createLabel metodunun Label'ların sahipliğini alması daha güvenli.
    // Burada sadece CodeGenerator'ın döndürdüğü Label* pointerlarını kullanıyoruz ve CodeGenerator'ın
    // bu Label nesnelerini doğru zamanda sildiğini varsayıyoruz.

    std::unique_ptr<Label> then_label_uptr = code_gen_.createLabel("if_then");
    Label* then_label = then_label_uptr.get(); // Ham işaretçiyi al

    std::unique_ptr<Label> else_label_uptr = code_gen_.createLabel("if_else");
    Label* else_label = else_label_uptr.get(); // Ham işaretçiyi al (else yoksa after_label olarak kullanılır)

    std::unique_ptr<Label> after_label_uptr = code_gen_.createLabel("if_after");
    Label* after_label = after_label_uptr.get(); // Ham işaretçiyi al


    // 1. Koşul ifadesi kodunu üret
    // Koşul ifadesinin değeri CodeGenerator tarafından işlenip bir boolean sonuç üretmeli.
    // Örneğin, sonuç stack'e veya bir register'a konulabilir.
     code_gen_.generateExpression(*stmt.condition); // CodeGenerator ana metotunu çağır

    // 2. Koşullu dallanma kodunu üret
    // Eğer koşul doğruysa 'then_label'a, yanlışsa 'else_label'a (veya else yoksa 'after_label'a) atla.
    code_gen_.emitConditionalBranch(*stmt.condition, then_label, stmt.else_branch ? else_label : after_label);

    // 3. 'Then' etiketini yerleştir ve 'Then' bloğunun kodunu üret
    code_gen_.emitLabel(then_label);
    generateStatement(*stmt.then_branch);

    // 4. Eğer 'Else' bloğu varsa, 'Then' bloğundan sonra 'After' etiketine koşulsuz atla
    if (stmt.else_branch) {
        code_gen_.emitUnconditionalBranch(after_label); // 'then' bloğundan sonra atla
        // 5. 'Else' etiketini yerleştir ve 'Else' bloğunun kodunu üret
        code_gen_.emitLabel(else_label);
        generateStatement(*stmt.else_branch);
    }

    // 6. 'After' etiketini yerleştir (buraya if/else dallarından gelinir)
    code_gen_.emitLabel(after_label);

    // Etiket unique_ptr'ları fonksiyon sonlandığında otomatik temizlenir,
    // ancak CodeGenerator'ın bu etiket pointerlarını tuttuğu ve yönettiği varsayılmıştır.
    // Eğer CodeGenerator kopyalarını tutuyorsa veya ham pointerları yönetiyorsa sorun olmaz.
}

// While deyiminin kodunu üretir
void ControlFlowGenerator::generateWhileStatement(WhileStmt& stmt) {
    // Etiketleri oluştur
    std::unique_ptr<Label> condition_label_uptr = code_gen_.createLabel("while_condition");
    Label* condition_label = condition_label_uptr.get();

    std::unique_ptr<Label> body_label_uptr = code_gen_.createLabel("while_body");
    Label* body_label = body_label_uptr.get();

    std::unique_ptr<Label> after_label_uptr = code_gen_.createLabel("while_after");
    Label* after_label = after_label_uptr.get();

    // Break ve Continue hedeflerini yığınlara ekle (bu döngü için)
    break_targets_.push(after_label);
    continue_targets_.push(condition_label);


    // 1. Koşul kontrolü etiketini yerleştir
    code_gen_.emitLabel(condition_label);

    // 2. Koşul ifadesi kodunu üret
    // code_gen_.generateExpression(*stmt.condition);

    // 3. Koşullu dallanma: Koşul doğruysa 'body_label'a, yanlışsa 'after_label'a atla
    code_gen_.emitConditionalBranch(*stmt.condition, body_label, after_label);

    // 4. Döngü gövdesi etiketini yerleştir
    code_gen_.emitLabel(body_label);

    // 5. Döngü gövdesi kodunu üret
    generateStatement(*stmt.body);

    // 6. Gövde bittikten sonra koşul kontrolü etiketine geri atla
    code_gen_.emitUnconditionalBranch(condition_label);

    // 7. Döngü sonu etiketini yerleştir (buraya koşul yanlışsa veya break ile gelinir)
    code_gen_.emitLabel(after_label);

    // Döngüden çıkarken etiket hedeflerini yığınlardan çıkar
    break_targets_.pop();
    continue_targets_.pop();
}


// Match deyiminin kodunu üretir (Bu karmaşık bir örnektir)
void ControlFlowGenerator::generateMatchStatement(MatchStmt& stmt) {
    // TODO: Match deyimi kod üretimi oldukça karmaşık olabilir.
    // - Eşleşecek değeri hesapla.
    // - Her case için:
    //   - Desen eşleştirme kodunu üret (bu kod, değer ile deseni karşılaştırır ve bool sonuç üretir).
    //   - Eğer desen eşleşirse ilgili case gövdesine atlayan koşullu dallanma üret.
    //   - Eğer desen eşleşmezse bir sonraki case'e atlayan koşulsuz dallanma üret.
    // - Tüm case'ler bittikten sonra veya bir case eşleştikten sonra atlanacak bir son etiket oluştur.

    // Basit bir iskelet:
    reporter_.reportWarning({stmt.line, stmt.column}, "Match statement code generation not fully implemented.");

    // 1. Eşleşecek değeri hesapla ve bir geçici değişkene veya register'a kaydet
     code_gen_.generateExpression(*stmt.value);
    // TODO: Sonucu sakla

    std::unique_ptr<Label> end_match_label_uptr = code_gen_.createLabel("match_end");
    Label* end_match_label = end_match_label_uptr.get();

    Label* next_case_label = nullptr; // Bir sonraki case'in etiketi

    for (size_t i = 0; i < stmt.cases.size(); ++i) {
        auto& match_case = stmt.cases[i];

        std::unique_ptr<Label> current_case_body_label_uptr = code_gen_.createLabel("match_case_body_" + std::to_string(i));
        Label* current_case_body_label = current_case_body_label_uptr.get();

        next_case_label = nullptr;
        if (i < stmt.cases.size() - 1) {
             // Son case değilse, bir sonraki case'in etiketini oluştur
            next_case_label = code_gen_.createLabel("match_case_check_" + std::to_string(i + 1)).get(); // CodeGen'in sahipliğinde
        } else {
            // Son case ise, bir sonraki hedef end_match_label'dır
             next_case_label = end_match_label;
        }

        // Önceki case'den buraya atlayan etiket (ilk case için buraya gelinmez)
        if (i > 0) {
            code_gen_.emitLabel(previous_case_check_label); // Önceki adımda oluşturulan etiket
        }


        // 2. Desen eşleştirme kodunu üret: Değer ve match_case->pattern karşılaştırılır
         code_gen_.generatePatternMatch(*stmt.value, *match_case->pattern); // Placeholder CodeGen metodu

        // 3. Koşullu dallanma: Eşleşirse current_case_body_label'a, eşleşmezse next_case_label'a atla
         code_gen_.emitConditionalBranch(pattern_match_result_expr, current_case_body_label, next_case_label);

        // 4. Case gövdesi etiketini yerleştir ve kodunu üret
        code_gen_.emitLabel(current_case_body_label);
        generateStatement(*match_case->body);

        // 5. Case gövdesi bittikten sonra end_match_label'a atla
        code_gen_.emitUnconditionalBranch(end_match_label);

        // Bir sonraki case kontrolü için etiket (eğer varsa)
         previous_case_check_label = next_case_label; // Sonraki iterasyon için hazırla
    }

    // 6. Tüm case'ler bittikten sonraki etiket (eşleşme olmazsa veya caseler biterse buraya gelinir)
    // Veya default case mantığı buraya eklenebilir.
    code_gen_.emitLabel(end_match_label);

    // Etiket unique_ptr'ları kapsam dışına çıkınca silinir (CodeGenerator sahipliğinde değilse!)
    // CodeGenerator'ın Label'ları yönettiği varsayılmıştır.
}

// Break deyiminin kodunu üretir
void ControlFlowGenerator::generateBreakStatement(BreakStmt& stmt) {
    // Break hedefleri yığını boşsa hata (semantik analizde yakalanmalı ama burada da kontrol)
    if (break_targets_.empty()) {
        reporter_.reportError({stmt.line, stmt.column}, "'break' statement outside of loop.");
        return; // Geçersiz break
    }

    // Yığının tepesindeki etikete atla (en içteki döngünün sonu)
    code_gen_.emitUnconditionalBranch(break_targets_.top());

    // TODO: Break'ten sonra ulaşılamayan kod (unreachable code) durumu CodeGenerator tarafından ele alınabilir.
}

// Continue deyiminin kodunu üretir
void ControlFlowGenerator::generateContinueStatement(ContinueStmt& stmt) {
    // Continue hedefleri yığını boşsa hata
    if (continue_targets_.empty()) {
        reporter_.reportError({stmt.line, stmt.column}, "'continue' statement outside of loop.");
        return; // Geçersiz continue
    }

    // Yığının tepesindeki etikete atla (en içteki döngünün koşul kontrolü veya bir sonraki iterasyon başı)
    code_gen_.emitUnconditionalBranch(continue_targets_.top());

    // TODO: Continue'dan sonra ulaşılamayan kod durumu CodeGenerator tarafından ele alınabilir.
}

// Return deyiminin kodunu üretir
void ControlFlowGenerator::generateReturnStatement(ReturnStmt& stmt) {
    // TODO: Return deyiminin bir fonksiyon içinde olup olmadığını kontrol et (semantic analysis'te yapıldı)

    // Dönüş değeri varsa, dönüş değeri ifadesinin kodunu üret
    // generateExpression metodu dönüş değerini uygun yere koymalıdır (örn: register)
     if (stmt.value) {
         code_gen_.generateExpression(*stmt.value);
     }

    // CodeGenerator'a dönüş kodunu üretme sinyali gönder
    code_gen_.emitReturn(stmt.value.get()); // Dönüş değeri ifadesine pointer gönderilir

    // TODO: Return'den sonra ulaşılamayan kod durumu CodeGenerator tarafından ele alınabilir.
}

// TODO: generateForStatement implementasyonu (while döngüsüne çevrilebilir veya özel işlenebilir)


} namespace CCube