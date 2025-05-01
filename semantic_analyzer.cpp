#include "semantic_analyzer.hpp"
#include <typeinfo> // dynamic_cast ile tip kontrolü için
#include <iostream> // Hata ayıklama çıktıları için

namespace CCube {

// SymbolTable metotlarının implementasyonları (ayrı .cpp dosyasında olmalı, burada yer tutucu)
SymbolTable::SymbolTable(ErrorReporter& reporter)
    : reporter_(reporter), current_scope_(nullptr), current_scope_level_(0) {
    enterScope(); // Global kapsamı oluştur
}

void SymbolTable::enterScope() {
    // Yeni kapsamı oluştur ve mevcut kapsamın çocuğu yap
    scopes_.push_back(std::make_unique<Scope>(current_scope_));
    current_scope_ = scopes_.back().get(); // current_scope'u yeni kapsama ayarla
    current_scope_level_++;
      std::cout << "Entered scope level " << current_scope_level_ << std::endl; // Hata ayıklama
}

void SymbolTable::exitScope() {
    if (current_scope_ && current_scope_->parent_scope) {
        // Üst kapsama geri dön
          std::cout << "Exiting scope level " << current_scope_level_ << std::endl; // Hata ayıklama
        current_scope_ = current_scope_->parent_scope;
        scopes_.pop_back(); // unique_ptr'ı ve Scope nesnesini siler
        current_scope_level_--;
    } else if (current_scope_) {
         // Global kapsamdan çıkılıyor
          std::cout << "Exiting global scope level " << current_scope_level_ << std::endl; // Hata ayıklama
         current_scope_ = nullptr;
         scopes_.pop_back();
         current_scope_level_--; // Should be 0
         // Artık kapsam yığını boş olmalı
    } else {
        // Hata: Zaten bir kapsamda değilken çıkış yapılmaya çalışıldı
         reporter_.reportError({0, 0}, "Internal Error: Tried to exit scope when not in a scope.");
    }
}

bool SymbolTable::declareSymbol(const std::string& name, SymbolKind kind, std::unique_ptr<Type> type, SourceLocation loc) {
    if (!current_scope_) {
        reporter_.reportError(loc, "Internal Error: Trying to declare symbol outside any scope.");
        return false;
    }

    if (current_scope_->symbols.count(name)) {
        reporter_.reportError(loc, "Redeclaration of '" + name + "' in the same scope.");
        // TODO: Önceki tanımlamanın konumunu da bulup raporda göstermek faydalı olur.
        return false;
    }

    // Sembolü oluştur ve mevcut kapsama ekle
    auto symbol = std::make_shared<Symbol>(name, kind, std::move(type), loc, current_scope_level_);
    current_scope_->symbols[name] = symbol;
     std::cout << "Declared symbol: " << symbol->toString() << " in scope " << current_scope_level_ << std::endl; // Hata ayıklama
    return true;
}

std::shared_ptr<Symbol> SymbolTable::resolveSymbol(const std::string& name) {
    Scope* scope = current_scope_;
    while (scope != nullptr) {
        if (scope->symbols.count(name)) {
            return scope->symbols[name]; // Sembol bulundu
        }
        scope = scope->parent_scope; // Üst kapsama geç
    }
    // Sembol bulunamadı
    return nullptr;
}


// SemanticAnalyzer Kurucu
SemanticAnalyzer::SemanticAnalyzer(Program& ast, ErrorReporter& reporter)
    : ast_(ast),
      reporter_(reporter),
      symbol_table_(reporter), // Sembol tablosu oluşturulurken hata raporlayıcı verilir
      in_loop_(false) // Başlangıçta döngü içinde değiliz
       current_function_return_type_ = nullptr // Başlangıçta fonksiyon içinde değiliz
{
    // TODO: TypeSystem'ı başlat
     type_system_ = TypeSystem();
}

// Semantik analiz işlemini başlatır
void SemanticAnalyzer::analyze() {
    // Program seviyesindeki analizi başlat
    analyzeProgram(ast_);

    // Analiz bittiğinde global kapsamdan çıkış yapılmalı (SymbolTable yıkıcısı bunu yapabilir veya burada exitScope() çağrılır)
     symbol_table_.exitScope(); // Global kapsam constructor'da girildiği için burada çıkılır.
}

// Program seviyesini analiz eder
void SemanticAnalyzer::analyzeProgram(Program& program) {
    // Üst düzey deyimleri analiz et
    for (auto& stmt : program.statements) {
        analyzeStatement(*stmt);
    }
}

// Genel bir deyimi analiz eder
void SemanticAnalyzer::analyzeStatement(Statement& stmt) {
    // Deyim türüne göre ilgili analiz fonksiyonunu çağır
    // dynamic_cast kullanıyoruz, alternatif olarak Visitor deseni kullanabilirsiniz.
    if (auto* s = dynamic_cast<ExpressionStmt*>(&stmt)) {
        analyzeExpressionStatement(*s);
    } else if (auto* s = dynamic_cast<VarDeclStmt*>(&stmt)) {
        analyzeVarDeclaration(*s);
    } else if (auto* s = dynamic_cast<BlockStmt*>(&stmt)) {
        analyzeBlockStatement(*s);
    } else if (auto* s = dynamic_cast<ImportStmt*>(&stmt)) {
        analyzeImportStatement(*s);
    } else if (auto* s = dynamic_cast<MatchStmt*>(&stmt)) {
        analyzeMatchStatement(*s);
    } else if (auto* s = dynamic_cast<IfStmt*>(&stmt)) {
        analyzeIfStatement(*s);
    } else if (auto* s = dynamic_cast<WhileStmt*>(&stmt)) {
        analyzeWhileStatement(*s);
    } else if (auto* s = dynamic_cast<DefStmt*>(&stmt)) {
        analyzeFunctionDefinition(*s);
    } else if (auto* s = dynamic_cast<ClassDeclStmt*>(&stmt)) {
        analyzeClassDefinition(*s);
    } else if (auto* s = dynamic_cast<ReturnStmt*>(&stmt)) {
        analyzeReturnStatement(*s);
    } else if (auto* s = dynamic_cast<BreakStmt*>(&stmt)) {
        analyzeBreakStatement(*s);
    } else if (auto* s = dynamic_cast<ContinueStmt*>(&stmt)) {
        analyzeContinueStatement(*s);
    }
    // TODO: Diğer deyim türleri (ForStmt vb.) buraya eklenecek
    else {
        // Bilinmeyen veya desteklenmeyen deyim türü
         reporter_.reportError({stmt.line, stmt.column}, "Internal Error: Unknown statement type encountered during semantic analysis.");
    }
}

// İfade deyimini analiz eder
void SemanticAnalyzer::analyzeExpressionStatement(ExpressionStmt& stmt) {
    // İfadeyi analiz et, dönüş değerini kullanmıyoruz
    analyzeExpression(*stmt.expr);
}

// Değişken bildirimi deyimini analiz eder
void SemanticAnalyzer::analyzeVarDeclaration(VarDeclStmt& stmt) {
    // Başlangıç değeri ifadesini analiz et ve tipini al
    std::unique_ptr<Type> initializer_type = Type::createUnknown(); // Varsayılan olarak bilinmeyen tip
    if (stmt.initializer) {
        initializer_type = analyzeExpression(*stmt.initializer);
    }

    // Sembol tablosuna değişkeni bildir
    // Atama hedefi her zaman değişken bildirimi değilse, bu mantık değişebilir.
    // Python'da ilk atama değişken bildirimidir.
     SourceLocation loc = {stmt.name_token.line, stmt.name_token.column};
     symbol_table_.declareSymbol(stmt.name_token.lexeme, SymbolKind::VARIABLE, std::move(initializer_type), loc);

    // AST düğümüne çözümlenmiş tip bilgisini ekleyebilirsiniz (isteğe bağlı)
     stmt.resolved_type = std::move(initializer_type); // VarDeclStmt Statement olduğu için resolved_type'ı yok

    // Eğer VarDeclStmt'in initializer'ı varsa, bu aslında bir atama işlemidir.
    // Atamanın anlamsal kontrolü (örn. tipe atanabilirlik) burada veya ayrı bir atama düğümü
    // (AssignmentExpr) analiz edilirken yapılabilir. Şimdilik sadece değişkeni bildiriyoruz.
}

// Blok deyimini analiz eder
void SemanticAnalyzer::analyzeBlockStatement(BlockStmt& stmt) {
    // Blok için yeni bir kapsam başlat
    symbol_table_.enterScope();

    // Blok içindeki her deyimi analiz et
    for (auto& child_stmt : stmt.statements) {
        analyzeStatement(*child_stmt);
    }

    // Bloktan çıkarken kapsamı sonlandır
    symbol_table_.exitScope();
}

// Import deyimini analiz eder
void SemanticAnalyzer::analyzeImportStatement(ImportStmt& stmt) {
    // TODO: Modül çözümleme mantığı buraya eklenecek.
    // İthal edilen modüllerin/isimlerin sembol tablosuna eklenmesi gerekebilir.
    // Bu karmaşık bir konu olabilir (dosya sistemi erişimi, modül arama yolları vb.).
    // Basitlik için şimdilik sadece var olduğunu varsayıp devam edelim.
    // Örneğin: from std.graphics import Vulkan -> Vulkan ismini sembol tablosuna FUNCTION/CLASS olarak ekle
    for (const auto& name : stmt.module_names) {
         reporter_.reportWarning({stmt.line, stmt.column}, "Import statement analysis is not fully implemented. Module: " + name);
         // Geçici olarak sembol tablosuna ekleyelim (doğru tip ve tür ile eklenmeli)
          symbol_table_.declareSymbol(name, SymbolKind::UNKNOWN, Type::createUnknown(), {stmt.line, stmt.column});
    }
}

// Match deyimini analiz eder
void SemanticAnalyzer::analyzeMatchStatement(MatchStmt& stmt) {
    // Eşleşecek değer ifadesini analiz et ve tipini al
    std::unique_ptr<Type> value_type = analyzeExpression(*stmt.value);

    // Her case'i analiz et
    for (auto& match_case : stmt.cases) {
        analyzeMatchCase(*match_case);

        // TODO: Pattern tipinin value_type ile uyumlu olup olmadığını kontrol et.
         reporter_.reportWarning({match_case->line, match_case->column}, "Match case type compatibility check not fully implemented.");
    }

    // TODO: Default case var mı kontrolü? Case'lerin kapsamı?
}

// Match Case'i analiz eder
void SemanticAnalyzer::analyzeMatchCase(MatchCase& match_case) {
    // Pattern ifadesini analiz et ve tipini al
    std::unique_ptr<Type> pattern_type = analyzeExpression(*match_case.pattern);

    // Case gövdesini analiz et
    analyzeStatement(*match_case.body);

     match_case.pattern->resolved_type = std::move(pattern_type); // Pattern Expression olduğu için tipi buraya eklenebilir
}


// If deyimini analiz eder
void SemanticAnalyzer::analyzeIfStatement(IfStmt& stmt) {
    // Koşul ifadesini analiz et
    std::unique_ptr<Type> condition_type = analyzeExpression(*stmt.condition);

    // TODO: Koşul ifadesinin Boolean tipinde olup olmadığını kontrol et.
     if (condition_type->id != TypeId::BOOLEAN) {
         reporter_.reportError({stmt.condition->line, stmt.condition->column}, "If condition must be a boolean expression.");
     }

    // If ve Else (varsa) dallarını analiz et
    analyzeStatement(*stmt.then_branch);
    if (stmt.else_branch) {
        analyzeStatement(*stmt.else_branch);
    }

    // TODO: If/Elif/Else dallarından dönen tiplerin birleştirilmesi (eğer ifade olarak kullanılıyorsa)
}

// While deyimini analiz eder
void SemanticAnalyzer::analyzeWhileStatement(WhileStmt& stmt) {
    // Döngü içine girildiğini işaretle (break/continue kontrolü için)
    bool old_in_loop = in_loop_;
    in_loop_ = true;

    // Koşul ifadesini analiz et
    std::unique_ptr<Type> condition_type = analyzeExpression(*stmt.condition);

    // TODO: Koşul ifadesinin Boolean tipinde olup olmadığını kontrol et.
      if (condition_type->id != TypeId::BOOLEAN) {
          reporter_.reportError({stmt.condition->line, stmt.condition->column}, "While condition must be a boolean expression.");
      }

    // Döngü gövdesini analiz et
    analyzeStatement(*stmt.body);

    // Döngüden çıkıldığını işaretle
    in_loop_ = old_in_loop;
}

// Fonksiyon tanımını analiz eder
void SemanticAnalyzer::analyzeFunctionDefinition(DefStmt& stmt) {
    // TODO: Fonksiyonun tipini belirle (parametre tipleri ve dönüş tipi)
    // Şimdilik parametre tiplerini UNKNOWN varsayalım, dönüş tipini VOID veya UNKNOWN.
    std::vector<std::unique_ptr<Type>> param_types;
    for (size_t i = 0; i < stmt.parameters.size(); ++i) {
         param_types.push_back(Type::createUnknown()); // Parametre tipi UNKNOWN (şimdilik)
    }
    std::unique_ptr<Type> return_type = Type::createVoid(); // Varsayılan dönüş tipi VOID

    // Fonksiyon tipini oluştur
    auto function_type = std::make_unique<FunctionType>(std::move(return_type), std::move(param_types));

    // Fonksiyon adını mevcut kapsamda bildir
    SourceLocation loc = {stmt.name_token.line, stmt.name_token.column};
    symbol_table_.declareSymbol(stmt.name_token.lexeme, SymbolKind::FUNCTION, std::move(function_type), loc);

    // Fonksiyon gövdesi için yeni bir kapsam başlat
    symbol_table_.enterScope();

    // Parametreleri yeni kapsamda bildir
    // Gerçek tipleri, argüman analizinden veya tip çıkarımından sonra belirlenecek
    for (size_t i = 0; i < stmt.parameters.size(); ++i) {
         SourceLocation param_loc = {stmt.parameters[i].line, stmt.parameters[i].column};
         // Parametre tipini kullanacağız (yukarıda oluşturulan param_types'tan)
         // Ancak SymbolTable'a eklerken unique_ptr taşımalıyız, bu yapılandırma değişebilir.
         // Şimdilik parametre tipini UNKNOWN olarak ekleyelim.
         symbol_table_.declareSymbol(stmt.parameters[i].lexeme, SymbolKind::PARAMETER, Type::createUnknown(), param_loc);
    }


    // TODO: Şu anki fonksiyonun dönüş tipini kaydet (return kontrolü için)
    // Type* old_function_return_type = current_function_return_type_;
     current_function_return_type_ = resolved_function_return_type_ptr; // Gerçek dönüş tipine işaret etmeli

    // Fonksiyon gövdesini analiz et (bir BlockStmt olmalı)
    analyzeBlockStatement(*stmt.body);

    // Fonksiyon kapsamından çık
    symbol_table_.exitScope();

    // TODO: Şu anki fonksiyonun dönüş tipini geri yükle
     current_function_return_type_ = old_function_return_type;

    // TODO: Fonksiyon gövdesindeki return deyimlerine bakarak dönüş tipini çıkar (eğer belirtilmemişse)
    // ve declareSymbol çağrısındaki FunctionType'ı güncelle.
}

// Sınıf tanımını analiz eder
void SemanticAnalyzer::analyzeClassDefinition(ClassDeclStmt& stmt) {
    // Sınıf adını mevcut kapsamda bildir
    SourceLocation loc = {stmt.name_token.line, stmt.name_token.column};
    // TODO: Sınıf tipi nesnesi oluşturulurken üyeleri, metotları, kalıtım bilgisi tutulmalı
    auto class_type = std::make_unique<ClassType>(stmt.name_token.lexeme);
    symbol_table_.declareSymbol(stmt.name_token.lexeme, SymbolKind::CLASS, std::move(class_type), loc);

    // Sınıf gövdesi için yeni bir kapsam başlat (sınıf üyeleri bu kapsamda olacak)
    symbol_table_.enterScope();

    // TODO: Kalıtım varsa üst sınıfları çözümle ve kapsam zincirine ekle

    // Sınıf gövdesindeki her deyimi (genellikle metotlar ve alanlar) analiz et
    for (auto& member_stmt : stmt.body) {
        // Metotlar analyzeFunctionDefinition ile, alanlar analyzeVarDeclaration ile analiz edilebilir.
        // AST'deki ClassDeclStmt body'si Statement listesi olduğu için analyzeStatement çağırılabilir.
        analyzeStatement(*member_stmt);

        // TODO: Analiz edilen üyeleri sınıf tipine ekle (class_type nesnesini güncelle)
    }

    // Sınıf kapsamından çık
    symbol_table_.exitScope();
}

// Return deyimini analiz eder
void SemanticAnalyzer::analyzeReturnStatement(ReturnStmt& stmt) {
    // Return deyiminin bir fonksiyon içinde olup olmadığını kontrol et
     current_function_return_type_ != nullptr kontrolü yapılabilir
     if (!current_function_return_type_) {
         reporter_.reportError({stmt.line, stmt.column}, "'return' statement outside of function body.");
     }

    // Dönüş değeri ifadesi varsa analiz et
    std::unique_ptr<Type> return_value_type = Type::createVoid(); // Dönüş değeri yoksa VOID
    if (stmt.value) {
        return_value_type = analyzeExpression(*stmt.value);
        // TODO: Dönüş değeri tipinin mevcut fonksiyonun dönüş tipiyle uyumlu olup olmadığını kontrol et
         if (!return_value_type->isAssignableTo(*current_function_return_type_)) {
              reporter_.reportError({stmt.value->line, stmt.value->column}, "Cannot convert return value type '" + return_value_type->toString() + "' to function return type '" + current_function_return_type_->toString() + "'.");
         }
    } else {
        // Dönüş değeri yoksa, fonksiyonun dönüş tipi VOID değilse hata
         if (current_function_return_type_->id != TypeId::VOID && current_function_return_type_->id != TypeId::UNKNOWN) {
              reporter_.reportError({stmt.line, stmt.column}, "Function expected return type '" + current_function_return_type_->toString() + "' but 'return' statement has no value.");
         }
    }
}

// Break deyimini analiz eder
void SemanticAnalyzer::analyzeBreakStatement(BreakStmt& stmt) {
    // Break deyiminin bir döngü içinde olup olmadığını kontrol et
    if (!in_loop_) {
        reporter_.reportError({stmt.line, stmt.column}, "'break' statement outside of loop body.");
    }
}

// Continue deyimini analiz eder
SemanticAnalyzer::analyzeContinueStatement(ContinueStmt& stmt) {
    // Continue deyiminin bir döngü içinde olup olmadığını kontrol et
    if (!in_loop_) {
        reporter_.reportError({stmt.line, stmt.column}, "'continue' statement outside of loop body.");
    }
}


// --- İfade Analiz Fonksiyonları ---

// Genel ifadeyi analiz eder ve tipini belirler
std::unique_ptr<Type> SemanticAnalyzer::analyzeExpression(Expression& expr) {
    // İfade türüne göre ilgili analiz fonksiyonunu çağır
    std::unique_ptr<Type> resolved_type = Type::createUnknown(); // Varsayılan tip

    if (auto* e = dynamic_cast<LiteralExpr*>(&expr)) {
        resolved_type = analyzeLiteralExpr(*e);
    } else if (auto* e = dynamic_cast<VariableExpr*>(&expr)) {
        resolved_type = analyzeVariableExpr(*e);
    } else if (auto* e = dynamic_cast<BinaryExpr*>(&expr)) {
        resolved_type = analyzeBinaryExpr(*e);
    } else if (auto* e = dynamic_cast<UnaryExpr*>(&expr)) {
         resolved_type = analyzeUnaryExpr(*e);
    } else if (auto* e = dynamic_cast<CallExpr*>(&expr)) {
         resolved_type = analyzeCallExpr(*e);
    }
    // TODO: Diğer ifade türleri (GetExpr, SetExpr, IndexExpr vb.) buraya eklenecek
    else {
        reporter_.reportError({expr.line, expr.column}, "Internal Error: Unknown expression type encountered during semantic analysis.");
        resolved_type = Type::createUnknown(); // Hata durumunda bilinmeyen tip döndür
    }

    // AST düğümüne çözümlenmiş tip bilgisini ekle
    expr.resolved_type = std::move(resolved_type);
    return expr.resolved_type->createUnknown(); // unique_ptr taşındığı için burası ne dönecek düşünülmeli.
                                             // Belki de analyzeExpression void olmalı ve sadece resolved_type'ı set etmeli.
                                             // Veya unique_ptr taşımak yerine Type*'a işaret etmeli.
                                             // Şu anki implementasyon unique_ptr'ı taşıyor, bu yüzden geri döndürürken yeni bir kopya (UNKNOWN) oluşturuyor.
                                             // Daha iyi yaklaşım: Fonksiyonlar Type* dönsün ve TypeSystem'da singleton temel tipler olsun.

    // Gelişmiş yaklaşım: analyzeExpression void olsun, içindeki alt fonksiyonlar Type* dönsün ve resolved_type'ı set etsin.
    // Veya analyzeExpression, analiz ettiği ifadenin *adresine* yazsın tipi.
     auto resolved_type_ptr = analyzeExpressionInternal(expr);
     expr.resolved_type = std::unique_ptr<Type>(resolved_type_ptr); // Ownership ataması
     return resolved_type_ptr; // Raw pointer döndür (dikkatli kullanılmalı)

    // Basitlik için, analyzeExpression Type* dönsün ve TypeSystem'da singleton temel tipler olsun.
    // Bu durumda Type nesneleri unique_ptr ile yönetilmez, TypeSystem kendisi yönetir.
    // AST düğümlerindeki resolved_type Type* olur.

     // --- Gelişmiş Tasarım İçin Güncelleme (resolved_type = Type*) ---
     
     // İfade türüne göre ilgili analiz fonksiyonunu çağır
     Type* resolved_type_ptr = nullptr; // Varsayılan olarak nullptr (UNKNOWN gibi)

     if (auto* e = dynamic_cast<LiteralExpr*>(&expr)) {
         resolved_type_ptr = analyzeLiteralExpr(*e);
     } // ... diğer türler
      else {
          reporter_.reportError({expr.line, expr.column}, "Internal Error: Unknown expression type encountered during semantic analysis.");
          resolved_type_ptr = TypeSystem::getInstance().getUnknownType(); // Singleton Unknown Type
      }

     // AST düğümüne çözümlenmiş tip bilgisini işaretçi olarak ekle
     expr.resolved_type_ptr = resolved_type_ptr; // resolved_type_ptr Type* olsun AST düğümünde
     return resolved_type_ptr; // İşaretçiyi döndür
     
      // Şimdilik, unique_ptr taşıma/kopyalama karmaşıklığından kaçınmak için
      // analyzeExpression'ın Type* döndürmesi ve TypeSystem'ın singleton tipler sağlaması daha mantıklı.
      // Ancak mevcut Type tanımı unique_ptr kullanıyor. Bu kısım yeniden tasarlanmalı.
      // Şimdilik mevcut unique_ptr'lı yapıyı koruyalım ama döndürülen değerin UNKNOWN olacağını bilelim.
      // analyzeExpression void olsun ve resolved_type'ı set etsin.

     // Void versiyonu:
     
      void SemanticAnalyzer::analyzeExpression(Expression& expr) {
         // ... logic ...
         if (auto* e = dynamic_cast<LiteralExpr*>(&expr)) {
             expr.resolved_type = analyzeLiteralExpr(*e); // analyzeLiteralExpr unique_ptr<Type> döndürür
         } // ... diğer türler
          else {
              reporter_.reportError({expr.line, expr.column}, "Internal Error: Unknown expression type encountered during semantic analysis.");
              expr.resolved_type = Type::createUnknown();
          }
      }
      std::unique_ptr<Type> SemanticAnalyzer::analyzeLiteralExpr(LiteralExpr& expr) {
          // ... logic ... return Type::createInteger();
      }
     
     // Bu yaklaşım daha iyi. analyzeExpression void olsun.

    // Revize edilmiş analyzeExpression (void versiyonu)
    // ... (Bu fonksiyonun implementasyonu aşağıda revize edilmiştir)

    // Şimdilik orijinal planla devam edelim, analyzeExpression unique_ptr döndürsün ama kullanımı dikkatli olsun.
    // Veya en iyisi AST düğümündeki resolved_type'ı Type* yapmak ve TypeSystem'ı singleton yapmak.
    // Bu kod setinde TypeSystem singleton ve resolved_type Type* varsayılarak devam edelim.
    // Bunun için ast.hpp ve type.hpp/cpp dosyalarını güncellemek gerekir.
    // Bu kodlar, TypeSystem singleton ve resolved_type Type* yapısını yansıtmayacaktır tam olarak,
    // ancak semantik analiz mantığını göstermeyi amaçlar.

    // Mevcut unique_ptr'lı yapı ile:
     Type* type_ptr = resolved_type.get(); // Point to the created type object
     expr.resolved_type = std::move(resolved_type); // Transfer ownership to the AST node
     return std::unique_ptr<Type>(type_ptr); // DİKKAT: Bu doğru değil! Pointer'ı unique_ptr'a sarıp transfer etmek ownership karmaşası yaratır.

     // En basit ve güvenli yol şudur: analyzeExpression'ın görevi sadece resolved_type'ı set etmek olsun. Geriye bir şey döndürmesin.
     // Alt fonksiyonlar da unique_ptr<Type> döndürsün.
     // analyzeExpression void olmalı.
     // Alt fonksiyonları unique_ptr<Type> döndürmeli.
     // analyzeExpression bu değeri alıp expr.resolved_type'a atamalı.
     // Bu şekilde devam edelim.

    // void analyzeExpression(Expression& expr) fonksiyonunun implementasyonu buraya gelecek.
    // Aşağıdaki Type* döndüren implementasyonlar bu void fonksiyona taşınacak ve unique_ptr döndürecek şekilde değiştirilecek.

    // Aşağıdaki fonksiyon implementasyonları, analyzeExpression'ın void olduğu ve kendilerine
    // gelen AST düğümünün resolved_type üyesine unique_ptr<Type> atadığı varsayımıyla yazılmıştır.
}

// Revize edilmiş analyzeExpression (void versiyonu)
void SemanticAnalyzer::analyzeExpression(Expression& expr) {
     // İfade türüne göre ilgili analiz fonksiyonunu çağır
     if (auto* e = dynamic_cast<LiteralExpr*>(&expr)) {
         expr.resolved_type = analyzeLiteralExpr(*e);
     } else if (auto* e = dynamic_cast<VariableExpr*>(&expr)) {
         expr.resolved_type = analyzeVariableExpr(*e);
     } else if (auto* e = dynamic_cast<BinaryExpr*>(&expr)) {
         expr.resolved_type = analyzeBinaryExpr(*e);
     } else if (auto* e = dynamic_cast<UnaryExpr*>(&expr)) {
         expr.resolved_type = analyzeUnaryExpr(*e);
     } else if (auto* e = dynamic_cast<CallExpr*>(&expr)) {
         expr.resolved_type = analyzeCallExpr(*e);
     }
     // TODO: Diğer ifade türleri
     else {
         reporter_.reportError({expr.line, expr.column}, "Internal Error: Unknown expression type encountered during semantic analysis.");
         expr.resolved_type = Type::createUnknown();
     }
}


// Literal ifadeyi analiz eder
std::unique_ptr<Type> SemanticAnalyzer::analyzeLiteralExpr(LiteralExpr& expr) {
    // Token tipine göre C-CUBE tipini belirle
    switch (expr.value_token.type) {
        case TokenType::INTEGER: return Type::createInteger();
        case TokenType::FLOAT:   return Type::createFloat();
        case TokenType::STRING:  return Type::createString();
        case TokenType::TRUE_KW:
        case TokenType::FALSE_KW: return Type::createBoolean();
        case TokenType::NONE_KW: return Type::createNone();
        default:
            reporter_.reportError({expr.line, expr.column}, "Internal Error: Unexpected token type for literal.");
            return Type::createUnknown();
    }
}

// Değişken ifadesini analiz eder (isim çözümleme)
std::unique_ptr<Type> SemanticAnalyzer::analyzeVariableExpr(VariableExpr& expr) {
    // Sembol tablosunda değişken adını çözümle
    std::shared_ptr<Symbol> symbol = symbol_table_.resolveSymbol(expr.name_token.lexeme);

    if (!symbol) {
        // Sembol bulunamadıysa hata
        reporter_.reportError({expr.line, expr.column}, "Undefined variable '" + expr.name_token.lexeme + "'.");
        return Type::createUnknown(); // Hata durumunda bilinmeyen tip
    }

    // TODO: Sembolün VARIABLE veya PARAMETER olduğundan emin ol.
      if (symbol->kind != SymbolKind::VARIABLE && symbol->kind != SymbolKind::PARAMETER) {
          reporter_.reportError({expr.line, expr.column}, "'" + expr.name_token.lexeme + "' is not a variable.");
           return Type::createUnknown();
     }

    // AST düğümüne çözümlenmiş sembolü kaydedebilirsiniz (kod üretimi için faydalı olabilir)
     expr.resolved_symbol = symbol; // VariableExpr yapısına resolved_symbol shared_ptr eklenebilir

    // Değişkenin tipini döndür (sembol tablosundan)
    // Sembolün tipini kopyalayarak unique_ptr oluşturmak gerekebilir.
    // Veya TypeSystem singleton ise, Type* döndürülebilir.
    // Mevcut Type yapısı unique_ptr kullandığı için burada dikkatli olmak gerek.
    // Semboldeki type unique_ptr, buraya kopyası lazım.
    // En iyisi TypeSystem singleton ve Type* resolved_type.
    // Şimdilik, semboldeki type'ın bir kopyasını oluşturup döndürelim (eğer Type copy/clone yapabiliyorsa).
    // Veya Symbol'deki type Unique_ptr<Type> değil, shared_ptr<Type> olabilir.
    // Symbol'deki type unique_ptr ve AST'deki resolved_type shared_ptr<Type> olsa?

    // Varsayılan olarak Sembol'deki type'ın unique_ptr olduğunu ve bir kopyasının döndürüleceğini varsayalım:
     return std::make_unique<Type>(*symbol->type); // Type copy constructor veya clone metodu olmalı

    // TypeSystem singleton ve resolved_type Type* varsayımıyla:
     return symbol->type; // Symbol'deki Type* pointerını döndür

    // Mevcut unique_ptr'lı Type yapısı ve void analyzeExpression varsayımıyla:
    if (symbol->type) {
         // Symbol'deki unique_ptr<Type>'ın bir kopyasını döndürmek (clone gerektirir)
         // Veya semboldeki shared_ptr<Type> ise:
          return symbol->type; // Symbol shared_ptr<Type> tutuyorsa
         // Eğer Symbol unique_ptr<Type> tutuyorsa ve resolved_type da unique_ptr<Type> ise...
         // semantic_analyzer'ın analyzeVariableExpr metodu unique_ptr<Type> döndürmeli.
          returned unique_ptr should be a *new* unique_ptr owning a copy of the type.
          Needs Type::clone() virtual method.

          Type::clone() olduğunu varsayarsak:
          return symbol->type->clone();

         // TypeSystem singleton ve resolved_type Type* varsayımıyla devam edelim.
         // Symbol'deki type da Type* olmalı.
          return symbol->type; // Symbol::type Type* ise

         // Geri dönüp mevcut unique_ptr'lı Type yapısına uygun bir dönüş yapısı düşünelim.
         // analyzeVariableExpr unique_ptr<Type> döndürmeli.
         // Döndürülen unique_ptr, sembolün tipinin bir kopyasına sahip olmalı.
         // Bu durumda Type sınıfına sanal bir `clone` metodu eklenmesi gerekir.

          Type::clone() implemente edildiğini varsayarak:
          return symbol->type->clone();

    }
     // Eğer sembolün tipi atanmamışsa (olmamalı)
    return Type::createUnknown();
}


// İkili operatör ifadesini analiz eder
std::unique_ptr<Type> SemanticAnalyzer::analyzeBinaryExpr(BinaryExpr& expr) {
    // Sol ve sağ ifadeleri analiz et ve tiplerini al
    analyzeExpression(*expr.left); // Bu çağrı left->resolved_type'ı set eder
    analyzeExpression(*expr.right); // Bu çağrı right->resolved_type'ı set eder

    Type* left_type = expr.left->resolved_type.get(); // AST düğümüne set edilen tipi al
    Type* right_type = expr.right->resolved_type.get(); // AST düğümüne set edilen tipi al

    // TODO: Tip sistemini kullanarak operatör ve işlenen tiplerine göre sonuç tipini belirle
    // ve tip uyumluluğunu kontrol et.
    Type* result_type = type_system_.getBinaryResultType(left_type, expr.operator_token.type, right_type);

    // Geçici olarak basit kurallar:
    TokenType op_type = expr.operator_token.type;
    if (op_type == TokenType::PLUS && left_type->id == TypeId::STRING && right_type->id == TypeId::STRING) {
         // String birleştirme
          return Type::createString(); // TypeSystem singleton olsaydı type_system_.getStringType()
          return Type::createString()->clone(); // unique_ptr/clone varsayımıyla

    } else if ((op_type == TokenType::PLUS || op_type == TokenType::MINUS || op_type == TokenType::STAR || op_type == TokenType::SLASH || op_type == TokenType::PERCENT) &&
               ((left_type->id == TypeId::INTEGER || left_type->id == TypeId::FLOAT) && (right_type->id == TypeId::INTEGER || right_type->id == TypeId::FLOAT))) {
        // Sayısal işlemler (int/float)
        if (left_type->id == TypeId::FLOAT || right_type->id == TypeId::FLOAT) {
             return Type::createFloat(); // TypeSystem singleton olsaydı
             return Type::createFloat()->clone(); // unique_ptr/clone varsayımıyla
        } else {
             return Type::createInteger(); // TypeSystem singleton olsaydı
             return Type::createInteger()->clone(); // unique_ptr/clone varsayımıyla
        }
    } else if ((op_type == TokenType::EQ || op_type == TokenType::NEQ || op_type == TokenType::LT || op_type == TokenType::GT || op_type == TokenType::LTE || op_type == TokenType::GTE) &&
               left_type->isConvertibleTo(*right_type) && right_type->isConvertibleTo(*left_type)) {
        // Karşılaştırma işlemleri (uyumlu tipler arasında)
         return Type::createBoolean(); // TypeSystem singleton olsaydı
         return Type::createBoolean()->clone(); // unique_ptr/clone varsayımıyla

    } else if ((op_type == TokenType::AND || op_type == TokenType::OR) &&
               left_type->id == TypeId::BOOLEAN && right_type->id == TypeId::BOOLEAN) {
        // Mantıksal işlemler
         return Type::createBoolean(); // TypeSystem singleton olsaydı
         return Type::createBoolean()->clone(); // unique_ptr/clone varsayımıyla

    } else {
        // Desteklenmeyen operatör veya tip uyuşmazlığı hatası
        reporter_.reportError({expr.operator_token.line, expr.operator_token.column},
                             "Unsupported operand types '" + left_type->toString() + "' and '" + right_type->toString() + "' for operator '" + expr.operator_token.lexeme + "'.");
        return Type::createUnknown();
    }
}

// Birli operatör ifadesini analiz eder
std::unique_ptr<Type> SemanticAnalyzer::analyzeUnaryExpr(UnaryExpr& expr) {
    // İşleneni analiz et ve tipini al
    analyzeExpression(*expr.operand);
    Type* operand_type = expr.operand->resolved_type.get();

    // TODO: Tip sistemini kullanarak operatör ve işlenen tipine göre sonuç tipini belirle
    // ve tip uyumluluğunu kontrol et.
    Type* result_type = type_system_.getUnaryResultType(expr.operator_token.type, operand_type);

    // Geçici olarak basit kurallar:
    TokenType op_type = expr.operator_token.type;
    if (op_type == TokenType::MINUS && (operand_type->id == TypeId::INTEGER || operand_type->id == TypeId::FLOAT)) {
         // Negatif işareti
         // return (operand_type->id == TypeId::INTEGER) ? Type::createInteger() : Type::createFloat();
         if (operand_type->id == TypeId::INTEGER) return Type::createInteger()->clone();
         else return Type::createFloat()->clone();

    } else if (op_type == TokenType::NOT && operand_type->id == TypeId::BOOLEAN) {
        // Mantıksal NOT
         return Type::createBoolean();
        return Type::createBoolean()->clone();

    } else {
        // Desteklenmeyen operatör veya tip uyuşmazlığı hatası
        reporter_.reportError({expr.operator_token.line, expr.operator_token.column},
                             "Unsupported operand type '" + operand_type->toString() + "' for unary operator '" + expr.operator_token.lexeme + "'.");
        return Type::createUnknown();
    }
}

// Fonksiyon/metot çağrısı ifadesini analiz eder
std::unique_ptr<Type> SemanticAnalyzer::analyzeCallExpr(CallExpr& expr) {
    // Çağrılan ifadeyi analiz et (bu bir VariableExpr (fonksiyon adı) veya GetExpr (metot) olabilir)
    analyzeExpression(*expr.callee);
    Type* callee_type = expr.callee->resolved_type.get();

    // TODO: Çağrılan ifadenin FunctionType veya ClassType (constructor çağrısı için) olduğundan emin ol.
     if (callee_type->id != TypeId::FUNCTION && callee_type->id != TypeId::CLASS) {
          reporter_.reportError({expr.callee->line, expr.callee->column}, "Expression is not callable.");
          // Argümanları yine de analiz etmeye devam et
          for (auto& arg : expr.arguments) analyzeExpression(*arg);
          return Type::createUnknown();
     }

    // Argüman ifadelerini analiz et
    std::vector<std::unique_ptr<Type>> argument_types;
    for (auto& arg : expr.arguments) {
        analyzeExpression(*arg);
        argument_types.push_back(arg->resolved_type->clone()); // Argüman tipini kopyala
    }

    // TODO: Çağrılan ifadenin tipine (callee_type - FunctionType veya ClassType) göre
    // argüman sayısının ve tiplerinin uyuştuğunu kontrol et.
    // Eğer FunctionType ise: callee_type->parameter_types ile argument_types'ı karşılaştır.
    // Eğer ClassType ise: Constructor'ı bul (varsa) ve onun parametreleri ile karşılaştır.

    // TODO: Fonksiyon/metot/constructor'ın dönüş tipini belirle ve döndür.
    // Eğer FunctionType ise: callee_type->return_type'ı döndür.
    // Eğer ClassType ise: Sınıfın kendisinin tipini döndür.

    // Geçici olarak bilinmeyen tip döndürelim ve kontrol uyarıları ekleyelim.
    reporter_.reportWarning({expr.line, expr.column}, "Call expression analysis (argument/parameter matching, return type) not fully implemented.");

    // Çağrı ifadesinin tipini belirle (Fonksiyonun dönüş tipi veya sınıf tipi)
    // Basitlik için şimdilik bilinmeyen tip döndürelim.
    return Type::createUnknown();
}


// TODO: Diğer ifade türleri için analiz fonksiyonları buraya eklenecek:
// analyzeGetExpr, analyzeSetExpr, analyzeIndexExpr vb.

} namespace CCube