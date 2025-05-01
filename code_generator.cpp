#include "code_generator.hpp"
#include <iostream> // Hata ayıklama için
#include <stdexcept> // Hata durumları için
#include <cassert> // Assertions için

// TODO: Çalışma zamanı somut Object tiplerini ve Value tiplerini dahil etmeniz gerekebilir
#include "Data Types/IntegerObject.hpp"
#include "Data Types/StringObject.hpp"
// ...

namespace CCube {

// --- Operand Kopyalama/Atama Implementasyonu (Union içerdiği için gerekli) ---
Operand::Operand(const Operand& other) : type(other.type) {
    switch (type) {
        case OperandType::INT: value.i = other.value.i; break;
        case OperandType::FLOAT: value.f = other.value.f; break;
        case OperandType::BOOL: value.b = other.value.b; break;
        case OperandType::STRING_INDEX: value.index = other.value.index; break;
        case OperandType::VAR_INDEX: value.index = other.value.index; break;
        case OperandType::BLOCK_INDEX: value.index = other.value.index; break;
        case OperandType::SYMBOL_INDEX: value.index = other.value.index; break;
        case OperandType::NONE: break; // Değer yok
    }
}

Operand& Operand::operator=(const Operand& other) {
    if (this != &other) {
        this->~Operand(); // Mevcut union üyesini temizle (basit tipler için gerekmeyebilir)
        type = other.type;
        switch (type) {
            case OperandType::INT: value.i = other.value.i; break;
            case OperandType::FLOAT: value.f = other.value.f; break;
            case OperandType::BOOL: value.b = other.value.b; break;
            case OperandType::STRING_INDEX: value.index = other.value.index; break;
            case OperandType::VAR_INDEX: value.index = other.value.index; break;
            case OperandType::BLOCK_INDEX: value.index = other.value.index; break;
            case OperandType::SYMBOL_INDEX: value.index = other.value.index; break;
            case OperandType::NONE: break;
        }
    }
    return *this;
}

// Instruction string gösterimi (hata ayıklama için)
std::string Instruction::toString() const {
     std::stringstream ss;
     ss << "<OpCode: " << static_cast<int>(opcode) << ">";
     for (const auto& op : operands) {
         ss << " <Operand: Type=" << static_cast<int>(op.type) << ", Value=";
         switch(op.type) {
             case OperandType::INT: ss << op.value.i; break;
             case OperandType::FLOAT: ss << op.value.f; break;
             case OperandType::BOOL: ss << (op.value.b ? "true" : "false"); break;
             case OperandType::STRING_INDEX: ss << "str_idx(" << op.value.index << ")"; break;
             case OperandType::VAR_INDEX: ss << "var_idx(" << op.value.index << ")"; break;
             case OperandType::BLOCK_INDEX: ss << "block_idx(" << op.value.index << ")"; break;
             case OperandType::SYMBOL_INDEX: ss << "sym_idx(" << op.value.index << ")"; break;
             case OperandType::NONE: ss << "none"; break;
         }
         ss << ">";
     }
     return ss.str();
}


// --- CodeGenerator Implementasyonu ---

// CodeGenerator Kurucu
CodeGenerator::CodeGenerator(Program& ast, SymbolTable& symbol_table, TypeSystem& type_system, ErrorReporter& reporter)
    : ast_(ast), symbol_table_(symbol_table), type_system_(type_system), reporter_(reporter) {
    // Başlangıçta IR modülü boş
    // String literal havuzu boş
    std::cout << "CodeGenerator created." << std::endl; // Hata ayıklama
}

// CodeGenerator Yıkıcı
CodeGenerator::~CodeGenerator() {
    // unique_ptr'lar otomatik temizlenir.
    std::cout << "CodeGenerator destroyed." << std::endl; // Hata ayıklama
}

// Kod üretim sürecini başlatır.
void CodeGenerator::generate() {
    std::cout << "Starting code generation..." << std::endl; // Hata ayıklama

    // Program seviyesindeki deyimleri ve tanımlamaları üret
    generateProgram(ast_);

    std::cout << "Code generation finished." << std::endl; // Hata ayıklama

    // Hata ayıklama için üretilen IR'ı yazdır (basit)
    std::cout << "\n--- Generated IR ---" << std::endl;
    for (const auto& func_uptr : generated_module_.functions) {
        const auto& func = *func_uptr;
        std::cout << "Function: " << func.symbol->name << std::endl;
        for (const auto& block_uptr : func.basic_blocks) {
            const auto& block = *block_uptr;
            std::cout << "  BasicBlock " << block.id << ":" << std::endl;
            for (const auto& instruction : block.instructions) {
                std::cout << "    " << instruction.toString() << std::endl;
            }
        }
    }
     std::cout << "\nString Literals:" << std::endl;
     for(size_t i = 0; i < generated_module_.string_literals.size(); ++i) {
         std::cout << "  " << i << ": \"" << generated_module_.string_literals[i] << "\"" << std::endl;
     }
     std::cout << "--------------------\n" << std::endl;
}

// Program seviyesindeki kod üretimi
void CodeGenerator::generateProgram(Program& program) {
    // Global kapsamdaki değişkenler için yer ayırma mantığı (IR'a bağlı)
    // Global deyimlerin kodunu üretmek için bir başlangıç fonksiyonu (main gibi) oluşturulabilir.

    // Global kod için geçici bir "main" fonksiyonu oluşturalım
    // SymbolTable'da "main" gibi bir sembol olmayabilir, bu nedenle Symbol oluşturmak gerekebilir.
     SourceLocation dummy_loc = {1, 1};
     auto main_symbol = std::make_shared<Symbol>("__global_main", SymbolKind::FUNCTION, type_system_.getVoidType()->clone(), dummy_loc, 0); // Dönüş tipi VOID
     current_function_ = new Function(); // new kullanıyoruz, Function sahipliği generated_module_ alır
     current_function_->symbol = main_symbol;
     generated_module_.functions.push_back(std::unique_ptr<Function>(current_function_)); // Sahipliği aktar

     // Başlangıç temel bloğunu oluştur
     current_block_ = createBasicBlock("entry");

     // Global deyimlerin kodunu üret
    for (auto& stmt : program.statements) {
        generateStatement(*stmt);
    }

    // Global kod bloğunun sonuna bir dönüş komutu ekle (eğer eklenmemişse)
    // Eğer son komut dallanma değilse, örtülü bir dönüş eklenir.
    if (current_block_ && (current_block_->instructions.empty() ||
                           (current_block_->instructions.back().opcode != OpCode::JUMP &&
                            current_block_->instructions.back().opcode != OpCode::BRANCH_IF &&
                            current_block_->instructions.back().opcode != OpCode::RETURN))) {
         emit(OpCode::RETURN); // main fonksiyonu için void return
    }

    // Global kod üretimi bitti, current_function_ ve current_block_ sıfırlanır
    current_function_ = nullptr;
    current_block_ = nullptr;
}

// AST'deki genel bir deyim için kod üretir
void CodeGenerator::generateStatement(Statement& stmt) {
     // Deyim türüne göre ilgili generate* fonksiyonunu çağır
     if (auto* s = dynamic_cast<BlockStmt*>(&stmt)) {
         generateBlockStatement(*s);
     } else if (auto* s = dynamic_cast<VarDeclStmt*>(&stmt)) {
         generateVarDeclaration(*s);
     } else if (auto* s = dynamic_cast<ExpressionStmt*>(&stmt)) {
         generateExpressionStatement(*s);
     } else if (auto* s = dynamic_cast<IfStmt*>(&stmt)) {
         generateIfStatement(*s);
     } else if (auto* s = dynamic_cast<WhileStmt*>(&stmt)) {
         generateWhileStatement(*s);
     } else if (auto* s = dynamic_cast<MatchStmt*>(&stmt)) {
         generateMatchStatement(*s); // Oldukça karmaşık
     } else if (auto* s = dynamic_cast<BreakStmt*>(&stmt)) {
         generateBreakStatement(*s);
     } else if (auto* s = dynamic_cast<ContinueStmt*>(&stmt)) {
         generateContinueStatement(*s);
     } else if (auto* s = dynamic_cast<ReturnStmt*>(&stmt)) {
         generateReturnStatement(*s);
     } else if (auto* s = dynamic_cast<DefStmt*>(&stmt)) {
         generateFunctionDefinition(*s);
     } else if (auto* s = dynamic_cast<ClassDeclStmt*>(&stmt)) {
         generateClassDefinition(*s); // Oldukça karmaşık
     }
     // TODO: Diğer deyim türleri (ImportStmt, ForStmt vb.)
     else {
         reporter_.reportError({stmt.line, stmt.column}, ErrorCode::INTERNAL_ERROR, "Unknown statement type encountered during code generation.");
     }
}

// AST'deki genel bir ifade için kod üretir (sonucu yığına koyar)
void CodeGenerator::generateExpression(Expression& expr) {
     // İfade türüne göre ilgili generate* fonksiyonunu çağır
     if (auto* e = dynamic_cast<LiteralExpr*>(&expr)) {
         generateLiteralExpr(*e);
     } else if (auto* e = dynamic_cast<VariableExpr*>(&expr)) {
         generateVariableExpr(*e);
     } else if (auto* e = dynamic_cast<BinaryExpr*>(&expr)) {
         generateBinaryExpr(*e);
     } else if (auto* e = dynamic_cast<UnaryExpr*>(&expr)) {
         generateUnaryExpr(*e);
     } else if (auto* e = dynamic_cast<CallExpr*>(&expr)) {
         generateCallExpr(*e);
     }
     // TODO: generateGetExpr, generateSetExpr, generateIndexExpr vb.
     else {
         reporter_.reportError({expr.line, expr.column}, ErrorCode::INTERNAL_ERROR, "Unknown expression type encountered during code generation.");
         // Bilinmeyen ifade durumunda yığına ne konulacağı IR'a bağlı. Boş bırakmak veya hata değeri basmak.
     }
}

// --- IR Emisyon Metotları ---

// Basic Block oluşturur ve mevcut blok olarak ayarlar.
BasicBlock* CodeGenerator::createBasicBlock(const std::string& name_hint) {
    if (!current_function_) {
        reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "Trying to create BasicBlock outside a function.");
        return nullptr;
    }
    auto new_block = std::make_unique<BasicBlock>(name_hint);
    new_block->id = generated_module_.functions.back()->basic_blocks.size(); // Basit kimlik atama
    current_block_ = new_block.get(); // Ham pointerı kaydet
    generated_module_.functions.back()->basic_blocks.push_back(std::move(new_block)); // Sahipliği aktar
    return current_block_;
}


// Verilen blok indexine/pointerına koşulsuz dallanma komutu ekler.
void CodeGenerator::emitUnconditionalBranch(BasicBlock* target_block) {
    if (!current_block_) { /* hata */ return; }
    int target_block_index = getBasicBlockIndex(target_block);
    if (target_block_index != -1) {
         emit(OpCode::JUMP, Operand::createBlockIndex(target_block_index));
         // Mevcut bloktan sonraki komutlar ulaşılamaz, yeni bir blok gerekebilir
         // Basit IR'da manuel olarak yeni blok oluşturmak gerekebilir.
          current_block_ = createBasicBlock("unreachable_or_next"); // Örnek
    } else {
         reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "Trying to jump to an invalid BasicBlock.");
    }
}

// Koşullu dallanma komutu ekler. Yığının tepesindeki bool değere göre dallanır.
void CodeGenerator::emitConditionalBranch(BasicBlock* true_block, BasicBlock* false_block) {
     if (!current_block_) { /* hata */ return; }
     int true_block_index = getBasicBlockIndex(true_block);
     int false_block_index = getBasicBlockIndex(false_block);

     if (true_block_index != -1 && false_block_index != -1) {
         // Yığının tepesinde boolean sonuç olduğu varsayılır.
         emit(OpCode::BRANCH_IF, Operand::createBlockIndex(true_block_index), Operand::createBlockIndex(false_block_index));
          // Bu komut her zaman mevcut bloğu sonlandırır, yeni bir blok oluşturulmalı.
           current_block_ = createBasicBlock("after_branch"); // Örnek
     } else {
         reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "Trying to branch to invalid BasicBlock(s).");
     }
}

// Mevcut Basic Block'a bir dönüş komutu ekler (değer döndürmeyen).
void CodeGenerator::emitReturn() {
    if (!current_block_) { /* hata */ return; }
    // Yığından dönüş değeri alınmaz
    emit(OpCode::RETURN);
     // Dönüş komutu her zaman bloğu sonlandırır.
      current_block_ = nullptr; // Veya yeni bir blok oluşturulmalı
}

// Mevcut Basic Block'a bir dönüş komutu ekler (değer döndüren).
void CodeGenerator::emitReturn(Expression& return_value_expr) {
     if (!current_block_) { /* hata */ return; }
     // Dönüş değeri ifadesi kodunu üret (sonucu yığına koyar)
     generateExpression(return_value_expr);
     // Yığının tepesindeki değeri döndürür
     emit(OpCode::RETURN);
      // Dönüş komutu her zaman bloğu sonlandırır.
       current_block_ = nullptr; // Veya yeni bir blok oluşturulmalı
}


// IR komutu ekler (0 operand)
void CodeGenerator::emit(OpCode opcode) {
     if (!current_block_) { /* hata */ return; }
     current_block_->instructions.push_back({opcode, {}});
}

// IR komutu ekler (1 operand)
void CodeGenerator::emit(OpCode opcode, Operand op1) {
     if (!current_block_) { /* hata */ return; }
     current_block_->instructions.push_back({opcode, {op1}});
}

// IR komutu ekler (2 operand)
void CodeGenerator::emit(OpCode opcode, Operand op1, Operand op2) {
     if (!current_block_) { /* hata */ return; }
     current_block_->instructions.push_back({opcode, {op1, op2}});
}

// TODO: Daha fazla emit aşırı yüklemesi veya std::vector<Operand> alan tek emit metodu


// --- AST Düğümü Kod Üretimi ---

// Blok deyiminin kodunu üretir
void CodeGenerator::generateBlockStatement(BlockStmt& stmt) {
    // Blok içindeki deyimleri üret
    // Kapsam yönetimi CodeGen'de de gerekiyorsa (değişken offsetleri) burada ele alınır.
     symbol_table_.enterCodeGenScope(); // Placeholder
    for (auto& child_stmt : stmt.statements) {
        generateStatement(*child_stmt);
    }
     symbol_table_.exitCodeGenScope(); // Placeholder
}

// Değişken bildiriminin kodunu üretir
void CodeGenerator::generateVarDeclaration(VarDeclStmt& stmt) {
    // Değişken sembolünü Semantik Analiz aşamasında çözülmüş olarak al
    std::shared_ptr<Symbol> var_symbol = symbol_table_.resolveSymbol(stmt.name_token.lexeme); // Veya AST düğümünde işaretçi tutulur

    if (!var_symbol) {
        reporter_.reportError({stmt.line, stmt.column}, ErrorCode::INTERNAL_ERROR, "Variable symbol not found during code generation.");
        return;
    }

    // Değişkenin IR'daki yerini belirle (yığın offseti, global adres vb.)
    size_t var_ir_index = getVariableIrIndex(var_symbol);

    if (stmt.initializer) {
        // Başlangıç değeri ifadesinin kodunu üret (sonucu yığına koyar)
        generateExpression(*stmt.initializer);
        // Yığındaki değeri değişkenin yerine sakla
        emit(OpCode::STORE_VAR, Operand::createVarIndex(var_ir_index));
    } else {
         // Başlangıç değeri yoksa, varsayılan değer (örn: None) ile başlatılabilir veya tanımsız kalır.
          emit(OpCode::LOAD_NONE); // Yığına None koy
          emit(OpCode::STORE_VAR, Operand::createVarIndex(var_ir_index)); // None'ı sakla
    }
}

// İfade deyiminin kodunu üretir
void CodeGenerator::generateExpressionStatement(ExpressionStmt& stmt) {
    // İfade kodunu üret (sonucu yığına koyar)
    generateExpression(*stmt.expr);
    // İfade deyimi sonucunu kullanmaz, bu yüzden sonucu yığından atarız
    emit(OpCode::POP);
}


// If deyiminin kodunu üretir
void CodeGenerator::generateIfStatement(IfStmt& stmt) {
    // If, Else ve Sonrası için temel blokları oluştur
    BasicBlock* then_block = createBasicBlock("if_then");
    BasicBlock* else_block = stmt.else_branch ? createBasicBlock("if_else") : nullptr;
    BasicBlock* after_block = createBasicBlock("if_after");

    // Koşul ifadesinin kodunu üret (sonucu yığına koyar - bool tipinde olmalı)
    generateExpression(*stmt.condition);

    // Koşula göre dallanma komutunu ekle
    // Eğer koşul doğruysa 'then' bloğuna, yanlışsa 'else' bloğuna (veya 'after' bloğuna) atla.
    emitConditionalBranch(then_block, stmt.else_branch ? else_block : after_block);

    // 'Then' bloğunun kodunu üret
    // Şu anki blok dallanma ile bitti, yeni blok zaten oluşturuldu (emitConditionalBranch içinde veya manuel)
    // Ancak eğer IR yapısı gereği emit* komutları bloğu otomatik kapatmıyorsa:
     current_block_ = then_block; // Bloğu değiştir

    emitLabel(then_block); // 'Then' bloğu etiketini yerleştir
    generateStatement(*stmt.then_branch); // 'Then' bloğu gövdesi kodunu üret

    // 'Then' bloğundan sonra 'After' bloğuna atla (Else varsa)
    if (stmt.else_branch) {
        emitUnconditionalBranch(after_block);
        // 'Else' bloğunun kodunu üret
        // current_block_ = else_block; // Bloğu değiştir
        emitLabel(else_block); // 'Else' bloğu etiketini yerleştir
        generateStatement(*stmt.else_branch); // 'Else' bloğu gövdesi kodunu üret
    }

    // 'After' bloğu etiketini yerleştir (her iki daldan veya else yoksa gelinir)
     current_block_ = after_block; // Bloğu değiştir
    emitLabel(after_block);
}


// While deyiminin kodunu üretir
void CodeGenerator::generateWhileStatement(WhileStmt& stmt) {
    // Koşul, Gövde ve Sonrası için temel blokları oluştur
    BasicBlock* condition_block = createBasicBlock("while_condition");
    BasicBlock* body_block = createBasicBlock("while_body");
    BasicBlock* after_block = createBasicBlock("while_after");

    // Break ve Continue hedeflerini yığınlara ekle (IR blok pointerları)
    break_targets_.push(after_block);
    continue_targets_.push(condition_block);

    // Döngü başına atla (koşul kontrol bloğu)
    emitUnconditionalBranch(condition_block);

    // Koşul kontrol bloğunu üret
    // current_block_ = condition_block; // Bloğu değiştir
    emitLabel(condition_block); // Koşul etiketini yerleştir

    // Koşul ifadesinin kodunu üret (sonucu yığına koyar - bool olmalı)
    generateExpression(*stmt.condition);

    // Koşula göre dallanma: Doğruysa gövdeye, yanlışsa sonraya atla
    emitConditionalBranch(body_block, after_block);

    // Döngü gövdesi bloğunu üret
    // current_block_ = body_block; // Bloğu değiştir
    emitLabel(body_block); // Gövde etiketini yerleştir
    generateStatement(*stmt.body); // Gövde kodunu üret

    // Gövde bittikten sonra koşul kontrolüne geri atla
    emitUnconditionalBranch(condition_block);

    // Döngü sonu bloğunu üret
    // current_block_ = after_block; // Bloğu değiştir
    emitLabel(after_block); // Sonu etiketini yerleştir

    // Break ve Continue hedeflerini yığınlardan çıkar
    break_targets_.pop();
    continue_targets_.pop();
}


// Match deyiminin kodunu üretir (Karmaşık)
void CodeGenerator::generateMatchStatement(MatchStmt& stmt) {
    reporter_.reportWarning({stmt.line, stmt.column}, ErrorCode::INTERNAL_ERROR, "Match statement code generation is complex and not fully implemented.");

    // 1. Eşleşecek değeri hesapla ve yığına koy
    generateExpression(*stmt.value);
    // TODO: Değeri yığından alıp bir geçici değişkene veya register'a saklamak daha iyi olabilir.

    BasicBlock* end_match_block = createBasicBlock("match_end");
    BasicBlock* next_case_check_block = nullptr;

    for (size_t i = 0; i < stmt.cases.size(); ++i) {
        auto& match_case = stmt.cases[i];

        // Bir sonraki case için kontrol bloğu etiketi
        if (i < stmt.cases.size() - 1) {
             next_case_check_block = createBasicBlock("match_case_check_" + std::to_string(i + 1));
        } else {
            // Son case ise, bir sonraki hedef end_match_block'tur
             next_case_check_block = end_match_block;
        }

        // Mevcut case'in kod gövdesi bloğu
        BasicBlock* current_case_body_block = createBasicBlock("match_case_body_" + std::to_string(i));

        // Önceki case'den veya baştan buraya atlayan etiket
        // Bu etiket, bu case'in deseninin kontrol edileceği noktadır.
        // if (i > 0) { emitLabel(previous_case_check_block); } else { // İlk case }
        // İlk case için, değer hesaplandıktan sonra buraya gelinebilir.

        // TODO: Desen eşleştirme kodunu üret. Bu, yığındaki değer ile desen ifadesini karşılaştırır.
        // generatePatternMatch(value, *match_case->pattern); // Placeholder function

        // TODO: Koşullu dallanma: Eğer desen eşleşirse current_case_body_block'a, eşleşmezse next_case_check_block'a atla.
         emitConditionalBranch(current_case_body_block, next_case_check_block);

        // Case gövdesi bloğu etiketini yerleştir ve kodunu üret
        emitLabel(current_case_body_block);
        generateStatement(*match_case->body);

        // Case gövdesi bittikten sonra end_match_block'a atla
        emitUnconditionalBranch(end_match_block);

        // Sonraki iterasyon için previous_case_check_block'u güncelle
         previous_case_check_block = next_case_check_block;
    }

    // Tüm case'ler bittikten sonraki etiket (eşleşme olmazsa veya caseler biterse buraya gelinir)
    emitLabel(end_match_block);

    // TODO: Match değeri için kullanılan geçici değişkeni/register'ı temizle.
}


// Break deyiminin kodunu üretir
void CodeGenerator::generateBreakStatement(BreakStmt& stmt) {
    if (break_targets_.empty()) {
        reporter_.reportError({stmt.line, stmt.column}, ErrorCode::INTERNAL_ERROR, "'break' statement outside of loop in code generation.");
        return; // Semantic analysis'te yakalanmalıydı
    }
    // Yığının tepesindeki BasicBlock'a (döngü sonu) koşulsuz atla
    emitUnconditionalBranch(break_targets_.top());
    // Break'ten sonraki kod ulaşılamaz, yeni blok oluşturulmalı
     current_block_ = createBasicBlock("unreachable_after_break"); // Örnek
}

// Continue deyiminin kodunu üretir
void CodeGenerator::generateContinueStatement(ContinueStmt& stmt) {
    if (continue_targets_.empty()) {
        reporter_.reportError({stmt.line, stmt.column}, ErrorCode::INTERNAL_ERROR, "'continue' statement outside of loop in code generation.");
        return; // Semantic analysis'te yakalanmalıydı
    }
    // Yığının tepesindeki BasicBlock'a (döngü koşul kontrolü) koşulsuz atla
    emitUnconditionalBranch(continue_targets_.top());
     // Continue'dan sonraki kod ulaşılamaz, yeni blok oluşturulmalı
      current_block_ = createBasicBlock("unreachable_after_continue"); // Örnek
}

// Return deyiminin kodunu üretir
void CodeGenerator::generateReturnStatement(ReturnStmt& stmt) {
    if (stmt.value) {
        // Dönüş değeri ifadesinin kodunu üret (sonucu yığına koyar)
        generateExpression(*stmt.value);
    } else {
        // Değer döndürmeyen return (yığına None veya Void koyulabilir)
         emit(OpCode::LOAD_NONE); // Eğer void None ile temsil ediliyorsa
    }
    // Dönüş komutunu ekle
    emit(OpCode::RETURN);
    // Return'den sonraki kod ulaşılamaz, yeni blok oluşturulmalı
     current_block_ = createBasicBlock("unreachable_after_return"); // Örnek
}

// Fonksiyon tanımının kodunu üretir
void CodeGenerator::generateFunctionDefinition(DefStmt& stmt) {
    // Fonksiyon sembolünü al (Semantik Analiz aşamasında oluşturulmuş)
    std::shared_ptr<Symbol> func_symbol = symbol_table_.resolveSymbol(stmt.name_token.lexeme);
     if (!func_symbol || func_symbol->kind != SymbolKind::FUNCTION) {
          reporter_.reportError({stmt.name_token.line, stmt.name_token.column}, ErrorCode::INTERNAL_ERROR, "Function symbol not found or invalid during code generation.");
          return;
     }

    // Yeni bir fonksiyon yapısı oluştur ve modüle ekle
    current_function_ = new Function();
    current_function_->symbol = func_symbol;
    generated_module_.functions.push_back(std::unique_ptr<Function>(current_function_)); // Sahipliği aktar

    // Fonksiyonun giriş temel bloğunu oluştur ve mevcut blok olarak ayarla
    current_block_ = createBasicBlock("entry");

    // TODO: Parametrelerin işlenmesi (çağrıdan gelen argümanları yığından almak/register'lara koymak)
    // Bu, fonksiyonun giriş bloğuna eklenecek komutlarla yapılır.

    // Fonksiyon gövdesinin kodunu üret (BlockStmt)
    generateBlockStatement(*stmt.body);

    // Fonksiyon gövdesinin sonu bir dallanma veya return değilse, örtülü bir return ekle
     if (current_block_ && (current_block_->instructions.empty() ||
                            (current_block_->instructions.back().opcode != OpCode::JUMP &&
                             current_block_->instructions.back().opcode != OpCode::BRANCH_IF &&
                             current_block_->instructions.back().opcode != OpCode::RETURN))) {
          emit(OpCode::RETURN); // Void return varsayımı
     }


    // Fonksiyon üretimi bitti, context'i sıfırla
    current_function_ = nullptr;
    current_block_ = nullptr;
}

// Sınıf tanımının kodunu üretir
void CodeGenerator::generateClassDefinition(ClassDeclStmt& stmt) {
    reporter_.reportWarning({stmt.name_token.line, stmt.name_token.column}, ErrorCode::INTERNAL_ERROR, "Class definition code generation is complex and not fully implemented.");
    // TODO: Sınıf yapısının IR'daki temsili (vtables, alan offsetleri vb.) üretilmelidir.
    // Metot tanımları generateFunctionDefinition gibi işlenir ancak sınıf kapsamına bağlıdır.
}


// --- İfade Kod Üretimi ---

// Literal ifade kodunu üretir (sonucu yığına koyar)
void CodeGenerator::generateLiteralExpr(LiteralExpr& expr) {
    // Literal tipine göre uygun LOAD komutunu ekle
    switch (expr.value_token.type) {
        case TokenType::INTEGER:
             // TODO: Token lexeme'ini integer'a çevir (std::stoi)
            try {
                 int value = std::stoi(expr.value_token.lexeme);
                 emit(OpCode::LOAD_INT, Operand::createInt(value));
            } catch (const std::exception& e) {
                 reporter_.reportError({expr.line, expr.column}, ErrorCode::INTERNAL_ERROR, "Failed to parse integer literal: " + std::string(e.what()));
                 emit(OpCode::LOAD_INT, Operand::createInt(0)); // Hata durumunda varsayılan değer
            }
            break;
        case TokenType::FLOAT:
            // TODO: Token lexeme'ini float'a çevir (std::stod)
            try {
                 double value = std::stod(expr.value_token.lexeme);
                 emit(OpCode::LOAD_FLOAT, Operand::createFloat(value));
            } catch (const std::exception& e) {
                 reporter_.reportError({expr.line, expr.column}, ErrorCode::INTERNAL_ERROR, "Failed to parse float literal: " + std::string(e.what()));
                 emit(OpCode::LOAD_FLOAT, Operand::createFloat(0.0)); // Hata durumunda varsayılan değer
            }
            break;
        case TokenType::STRING:
            // String literal'i havuza ekle ve indexini yükle
            {
                 size_t index = addStringLiteral(expr.value_token.lexeme); // Lexeme zaten tırnaklar olmadan işlenmiş olmalıydı Lexer'da
                 emit(OpCode::LOAD_STRING, Operand::createStringIndex(index));
            }
            break;
        case TokenType::TRUE_KW:
             emit(OpCode::LOAD_BOOL, Operand::createBool(true));
             break;
        case TokenType::FALSE_KW:
             emit(OpCode::LOAD_BOOL, Operand::createBool(false));
             break;
        case TokenType::NONE_KW:
             emit(OpCode::LOAD_NONE);
             break;
        default:
            reporter_.reportError({expr.line, expr.column}, ErrorCode::INTERNAL_ERROR, "Unhandled literal type during code generation.");
             emit(OpCode::LOAD_NONE); // Bilinmeyen literal için None yükle
            break;
    }
}

// Değişken ifadesi kodunu üretir (değişkenin değerini yığına yükler)
void CodeGenerator::generateVariableExpr(VariableExpr& expr) {
    // Sembolü çözümle (Semantik Analiz aşamasında çözülmüş olmalıydı)
    std::shared_ptr<Symbol> var_symbol = symbol_table_.resolveSymbol(expr.name_token.lexeme); // Veya AST düğümünde işaretçi tutulur

    if (!var_symbol) {
        reporter_.reportError({expr.line, expr.column}, ErrorCode::INTERNAL_ERROR, "Variable symbol not found during code generation.");
        // Hata durumunda yığına ne konulacağı IR'a bağlı. None basmak yaygın.
        emit(OpCode::LOAD_NONE);
        return;
    }

    // Değişkenin IR'daki yerini al (yığın offseti, global adres vb.)
    size_t var_ir_index = getVariableIrIndex(var_symbol);

    // Değişkenin değerini yığına yükle komutunu ekle
    emit(OpCode::LOAD_VAR, Operand::createVarIndex(var_ir_index));
}


// İkili operatör ifadesi kodunu üretir
void CodeGenerator::generateBinaryExpr(BinaryExpr& expr) {
    // Sol ve sağ işlenenlerin kodunu üret (sonuçları yığına koyar)
    generateExpression(*expr.left);
    generateExpression(*expr.right);

    // Semantik Analiz sırasında belirlenen çözümlenmiş tipleri al
    Type* left_type = expr.left->resolved_type.get(); // AST düğümünde Type* tutulduğu varsayımıyla
    Type* right_type = expr.right->resolved_type.get(); // AST düğümünde Type* tutulduğu varsayımıyla

    // Operatör tokenı tipine ve işlenen tiplerine göre uygun IR komutunu seç
    OpCode op_code = getBinaryOpCode(expr.operator_token.type, left_type, right_type);

    if (op_code != OpCode::LOAD_INT) { // LOAD_INT varsayılan hata kodu döndürüyor, daha iyi bir kontrol gerek
         emit(op_code); // Seçilen ikili operatör komutunu ekle (yığındaki 2 değeri işler, sonucu yığına koyar)
    } else {
         // getBinaryOpCode hata durumu döndürdüyse, burada hata raporlanmış olmalı.
         // Yığına bir hata değeri veya None basmak gerekebilir.
         emit(OpCode::LOAD_NONE); // Hata durumunda yığına None koy
    }
}

// Birli operatör ifadesi kodunu üretir
void CodeGenerator::generateUnaryExpr(UnaryExpr& expr) {
    // İşlenenin kodunu üret (sonucu yığına koyar)
    generateExpression(*expr.operand);

    // Semantik Analiz sırasında belirlenen çözümlenmiş tipi al
    Type* operand_type = expr.operand->resolved_type.get(); // AST düğümünde Type* tutulduğu varsayımıyla

    // Operatör tokenı tipine ve işlenen tipine göre uygun IR komutunu seç
    OpCode op_code = getUnaryOpCode(expr.operator_token.type, operand_type);

     if (op_code != OpCode::LOAD_INT) { // LOAD_INT varsayılan hata kodu döndürüyor
         emit(op_code); // Seçilen birli operatör komutunu ekle (yığındaki 1 değeri işler, sonucu yığına koyar)
     } else {
          emit(OpCode::LOAD_NONE); // Hata durumunda yığına None koy
     }
}

// Fonksiyon çağrısı ifadesi kodunu üretir
void CodeGenerator::generateCallExpr(CallExpr& expr) {
    // Çağrılan ifade (callee) kodunu üret (fonksiyon/metot referansı yığına konabilir veya doğrudan biliniyorsa kullanılmaz)
    // Eğer callee bir VariableExpr ise, sembolünü çözümleyip fonksiyon sembolünü al.
    // Eğer callee bir GetExpr (obj.method) ise, objenin ve metod sembolünün kodunu üret.

    // Basitlik için, callee'nin doğrudan bir fonksiyon sembolüne çözümlendiğini varsayalım
    // (Bu genellikle SemanticAnalyzer tarafından yapılır ve AST düğümüne eklenir)
     std::shared_ptr<Symbol> func_symbol = expr.callee->resolved_symbol; // AST düğümünde resolved_symbol olduğu varsayımı

    // Sembolü çözümle (geçici olarak isimle arama)
    std::shared_ptr<Symbol> func_symbol = nullptr;
     if (auto var_expr = dynamic_cast<VariableExpr*>(expr.callee.get())) {
         func_symbol = symbol_table_.resolveSymbol(var_expr->name_token.lexeme);
     }
     // TODO: Metot çağrıları için GetExpr durumunu ele al

    if (!func_symbol || func_symbol->kind != SymbolKind::FUNCTION) {
        reporter_.reportError({expr.line, expr.column}, ErrorCode::INTERNAL_ERROR, "Function symbol not resolved or invalid during code generation.");
        // Hata durumunda yığına hata değeri veya None basılabilir.
        // Yine de argümanları üretip yığından atmak gerekebilir.
        for (auto& arg : expr.arguments) generateExpression(*arg); // Argümanları üret
        emit(OpCode::POP, Operand::createInt(expr.arguments.size())); // Argümanları yığından at
        emit(OpCode::LOAD_NONE); // Sonuç olarak None bas
        return;
    }

    // Argüman ifadelerinin kodunu üret (yığına sıralı olarak konulur)
    for (auto& arg : expr.arguments) {
        generateExpression(*arg);
    }

    // Fonksiyon çağrısı komutunu ekle
    // İşlenenler: Fonksiyon sembolünün indeksi, argüman sayısı
    // IR, argümanları yığından alıp işleyecek şekilde tasarlanmalı.
    // return değeri yığına konulacak.
    emit(OpCode::CALL, Operand::createSymbolIndex(0), Operand::createInt(expr.arguments.size())); // Symbol indexi placeholder (0)


    // TODO: generateGetExpr, generateSetExpr, generateIndexExpr implementasyonları
}


// --- Yardımcı Fonksiyonlar Implementasyonu ---

// String literal'i havuza ekler ve indexini döndürür. Zaten varsa mevcut indexi döndürür.
size_t CodeGenerator::addStringLiteral(const std::string& str) {
    if (string_literal_map_.count(str)) {
        return string_literal_map_[str];
    }
    size_t index = generated_module_.string_literals.size();
    generated_module_.string_literals.push_back(str);
    string_literal_map_[str] = index;
    return index;
}

// Sembolün IR'daki karşılığını (örn: yığın offseti, global adres) belirler ve döndürür.
// Bu basit implementasyonda sadece ardışık indeksler atıyoruz.
size_t CodeGenerator::getVariableIrIndex(std::shared_ptr<Symbol> var_symbol) {
    // TODO: Gerçek implementasyonda, sembolün kapsam seviyesine ve türüne (local, global, parameter, field)
    // bakarak yığın offseti, register numarası veya global veri adresi belirlenir.
    // Sembol yapısına IR'a özgü bir konum bilgisi üyesi eklenebilir.

    // Basitlik için, sembolü hashleyip bir indeks döndürelim veya sembol tablosunda bir indeks tutalım.
    // Veya SemanticAnalyzer sembole IR konum bilgisi ekleyebilir.

    // Şimdilik, sembol adresini (shared_ptr) kullanarak benzersiz bir indeks atayalım.
    // Bu, gerçek bir adresleme şeması değildir!

    // Statik bir harita kullanarak shared_ptr adresini bir indekse eşleyelim.
    static std::unordered_map<std::shared_ptr<Symbol>, size_t> symbol_ir_index_map;
    static size_t next_index = 0;

    if (symbol_ir_index_map.count(var_symbol)) {
        return symbol_ir_index_map[var_symbol];
    }

    size_t index = next_index++;
    symbol_ir_index_map[var_symbol] = index;
     reporter_.reportWarning(var_symbol->declaration_location, ErrorCode::INTERNAL_ERROR, "Assigned dummy IR index " + std::to_string(index) + " to variable '" + var_symbol->name + "'.");
    return index;
}


// Bir BasicBlock pointerını onun CompileModule içindeki indexine eşler
// BasicBlock'lar Function içinde tutuluyor. Bu fonksiyon, bir BasicBlock'un
// ait olduğu fonksiyon içindeki indeksini bulur.
int CodeGenerator::getBasicBlockIndex(BasicBlock* block) {
     if (!block || !current_function_) return -1;
     for (size_t i = 0; i < current_function_->basic_blocks.size(); ++i) {
         if (current_function_->basic_blocks[i].get() == block) {
             return static_cast<int>(i);
         }
     }
     return -1; // Bulunamadı (olmamalı)
}


// İşlem sonucunun tipine göre uygun ikili operatör opcode'unu seçer
OpCode CodeGenerator::getBinaryOpCode(TokenType op_token_type, Type* left_type, Type* right_type) {
    // Semantic Analysis aşamasında tiplerin uyumlu olduğu varsayılır.
    // Burada sadece runtime tiplerine göre doğru IR opcode'u seçilir.
    // IR'da her tip/operatör kombinasyonu için ayrı opcode olabilir veya tek opcode farklı runtime dispatch yapar.
    // Basitlik için ayrı opcode'lar varsayalım.

    if (!left_type || !right_type) return OpCode::LOAD_INT; // Hata kodu

    switch (op_token_type) {
        case TokenType::PLUS:
            if (left_type->id == TypeId::INTEGER && right_type->id == TypeId::INTEGER) return OpCode::ADD_INT;
            if (left_type->id == TypeId::FLOAT && right_type->id == TypeId::FLOAT) return OpCode::ADD_FLOAT;
             if (left_type->id == TypeId::STRING && right_type->id == TypeId::STRING) return OpCode::ADD_STRING;
            // TODO: Diğer + overloadi (örn: int+float, list+list)
            break;
        case TokenType::MINUS:
            if (left_type->id == TypeId::INTEGER && right_type->id == TypeId::INTEGER) return OpCode::SUB_INT;
            if (left_type->id == TypeId::FLOAT && right_type->id == TypeId::FLOAT) return OpCode::SUB_FLOAT;
            break;
        // TODO: Diğer operatörler için opcode eşlemeleri
        case TokenType::STAR: // *
             if (left_type->id == TypeId::INTEGER && right_type->id == TypeId::INTEGER) return OpCode::MUL_INT;
             if (left_type->id == TypeId::FLOAT && right_type->id == TypeId::FLOAT) return OpCode::MUL_FLOAT;
             break;
        case TokenType::SLASH: // /
             if (left_type->id == TypeId::INTEGER && right_type->id == TypeId::INTEGER) return OpCode::DIV_INT; // Integer division?
             if (left_type->id == TypeId::FLOAT && right_type->id == TypeId::FLOAT) return OpCode::DIV_FLOAT;
             break;
        case TokenType::EQ: // ==
             if (left_type->id == TypeId::INTEGER && right_type->id == TypeId::INTEGER) return OpCode::EQ_INT;
             if (left_type->id == TypeId::FLOAT && right_type->id == TypeId::FLOAT) return OpCode::EQ_FLOAT;
             if (left_type->id == TypeId::BOOL && right_type->id == TypeId::BOOL) return OpCode::EQ_BOOL;
             // TODO: Diğer eşitlik karşılaştırmaları
             break;

        // ... diğer operatörler ...
    }

     // Eşleşme yoksa veya desteklenmiyorsa hata kodu döndür
    reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "Failed to get binary opcode for types " + left_type->toString() + ", " + right_type->toString() + " and operator " + tokenTypeToString(op_token_type) + ".");
    return OpCode::LOAD_INT; // Hata olduğunu belirten geçici bir dönüş
}


// İşlem sonucunun tipine göre uygun birli operatör opcode'unu seçer
OpCode CodeGenerator::getUnaryOpCode(TokenType op_token_type, Type* operand_type) {
     if (!operand_type) return OpCode::LOAD_INT; // Hata kodu

     switch (op_token_type) {
         case TokenType::MINUS: // -
             if (operand_type->id == TypeId::INTEGER) return OpCode::NEG_INT;
             if (operand_type->id == TypeId::FLOAT) return OpCode::NEG_FLOAT;
             break;
         case TokenType::NOT: // not
             if (operand_type->id == TypeId::BOOLEAN) return OpCode::NOT_BOOL;
             break;
         // ... diğer birli operatörler ...
     }

     reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "Failed to get unary opcode for type " + operand_type->toString() + " and operator " + tokenTypeToString(op_token_type) + ".");
     return OpCode::LOAD_INT; // Hata
}

// Bir BasicBlock pointerını onun CompileModule içindeki indexine eşler
int CodeGenerator::getBasicBlockIndex(BasicBlock* block) {
    if (!block || !current_function_) return -1; // Geçersiz blok veya fonksiyon yok

     // BasicBlock pointerını, ait olduğu fonksiyonun basic_blocks vektöründeki indexine dönüştür.
     // Bu, IR içinde dallanmalar için blok indexlerini kullanmamızı sağlar.
      std::distance kullanılabilir.
     auto it = std::find_if(current_function_->basic_blocks.begin(), current_function_->basic_blocks.end(),
                            [block](const std::unique_ptr<BasicBlock>& uptr){ return uptr.get() == block; });

     if (it != current_function_->basic_blocks.end()) {
         return static_cast<int>(std::distance(current_function_->basic_blocks.begin(), it));
     }

    return -1; // Bulunamadı (olmamalı)
}

// TODO: getVariableIrIndex implementasyonu (Sembolün yığın/register/global adresini belirleme)
size_t CodeGenerator::getVariableIrIndex(std::shared_ptr<Symbol> var_symbol) {
    reporter_.reportWarning(var_symbol->declaration_location, ErrorCode::INTERNAL_ERROR, "Placeholder getVariableIrIndex used for '" + var_symbol->name + "'. Returns dummy index.");
    // Gerçek implementasyon, sembolün kapsamına ve türüne (yerel, parametre, global, sınıf üyesi) bakar.
    // Yerel değişkenler ve parametreler için yığın çerçevesi içinde bir offset hesaplar.
    // Global değişkenler için global veri segmentinde bir adres veya index atar.
    // Sınıf üyeleri (alanlar) için nesne yapısı içinde bir offset hesaplar.

    // Şimdilik Sembol adresine dayalı basit bir haritalama kullanalım (gerçek adresleme değil!)
     static std::unordered_map<std::shared_ptr<Symbol>, size_t> symbol_ir_index_map;
     static size_t next_index = 0; // Tüm değişkenler için global sayaç (basit)

     if (symbol_ir_index_map.find(var_symbol) == symbol_ir_index_map.end()) {
         symbol_ir_index_map[var_symbol] = next_index++;
     }

    return symbol_ir_index_map[var_symbol];
}


// TODO: Diğer generate* fonksiyonları ve yardımcı fonksiyonların implementasyonları buraya gelecek.


} namespace CCube