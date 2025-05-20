#include "interpreter.h"
#include "ast.h"
#include "value.h"
#include "environment.h"
#include "token.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cmath> // pow için

// Hata raporlama fonksiyonu (main.cpp'de olmalı veya ayrı bir modül)
 extern bool hadRuntimeError; // Global hata flag'i (basitlik için)
 void runtimeError(const Token& token, const std::string& message); // Hata raporlama fonksiyonu

// Interpreter sınıfı implementasyonu

// Interpreter'ın hem StmtVisitor hem de ExprVisitor arayüzlerini implement ettiğini varsayalım
 class Interpreter : public StmtVisitor, public ExprVisitor { ... } şeklinde tanımlandıysa

Interpreter::Interpreter() {
    // Global ortamı başlat ve standart kütüphane fonksiyonlarını ekle
    globals = std::make_shared<Environment>();
    environment = globals; // Başlangıçta mevcut ortam global ortamdır

    // Örnek: 'print' global fonksiyonunu ekle
    // Built-in fonksiyonlar için özel bir Value tipi veya sınıfı gerekebilir
    // Şimdilik print'i direk C++ kodu olarak burada işleyeceğiz.
    // Eğer C-CUBE'dan çağrılacak bir fonksiyon olarak eklemek istiyorsanız:
    
    class BuiltinPrint : public C_CUBE_Callable { // C_CUBE_Callable özel bir sınıf
    public:
        int arity() const override { return 1; } // 1 argüman alır
        ValuePtr call(Interpreter& interpreter, const std::vector<ValuePtr>& arguments) override {
            // arguments[0]'ı yazdır
            std::cout << valueToString(arguments[0]) << std::endl;
            return std::make_shared<Value>(); // None döndür
        }
    };
    globals->define("print", std::make_shared<Value>(std::make_shared<BuiltinPrint>()));
}

void Interpreter::interpret(const std::vector<StmtPtr>& statements) {
    try {
        for (const auto& stmt : statements) {
            execute(stmt);
        }
    } catch (const std::runtime_error& error) {
         runtimeError'ı yakala ve raporla
         std::cerr << error.what() << std::endl;
         hadRuntimeError = true;
    }
}

// Deyim çalıştırma ana metodu (Visitor desenine göre accept çağırır)
void Interpreter::execute(StmtPtr stmt) {
    if (stmt) {
        stmt->accept(*this); // İlgili ziyaretçi metodunu çağır
    }
}

// Blok deyimi çalıştırma (Yeni bir skop oluşturur)
void Interpreter::executeBlock(const std::vector<StmtPtr>& statements, EnvironmentPtr environment) {
    // Şu anki ortamı kaydet
    EnvironmentPtr previous = this->environment;

    try {
        this->environment = environment; // Ortamı yeni blok ortamına ayarla
        for (const auto& stmt : statements) {
            execute(stmt); // Blok içindeki deyimleri çalıştır
        }
    } catch (...) {
        // İstisnaları yakala ve ortamı geri yüklemeden yeniden fırlat
        this->environment = previous; // Ortamı geri yükle
        throw;
    }
    // Ortamı geri yükle
    this->environment = previous;
}

// İfade değerlendirme ana metodu (Visitor desenine göre accept çağırır)
ValuePtr Interpreter::evaluate(ExprPtr expr) {
    if (expr) {
       return expr->accept(*this); // İlgili ziyaretçi metodunu çağır ve değeri döndür
    }
    // Geçici olarak nullptr döndür, gerçekte hata fırlatılmalı veya None döndürülmeli
    return nullptr;
}


// StmtVisitor Metodları Implementasyonu (ast.h'daki accept metodları bunları çağıracak)

void Interpreter::visitExpressionStmt(const ExpressionStmt& stmt) {
    evaluate(stmt.expression); // İfadeyi değerlendir, sonucu kullanma
}

void Interpreter::visitPrintStmt(const PrintStmt& stmt) {
    ValuePtr value = evaluate(stmt.expression); // İfadeyi değerlendir
     std::cout << valueToString(value) << std::endl; // Değeri string'e çevirip yazdır
    // Basitlik için variant içindekini yazdırma (daha robust bir valueToString yazılmalı)
    if (value) {
         if (std::holds_alternative<std::string>(*value)) {
             std::cout << std::get<std::string>(*value) << std::endl;
         } else if (std::holds_alternative<double>(*value)) {
             std::cout << std::get<double>(*value) << std::endl;
         } else if (std::holds_alternative<bool>(*value)) {
              std::cout << std::get<bool>(*value) << std::endl;
         } else if (std::holds_alternative<std::monostate>(*value)) {
              std::cout << "none" << std::endl; // None değerini yazdırma
         } else {
              std::cout << "[object or unknown type]" << std::endl; // Diğer tipler için yer tutucu
         }
    } else {
         std::cout << "nullptr value" << std::endl; // evaluate null döndürürse
    }
}

void Interpreter::visitVarDeclStmt(const VarDeclStmt& stmt) {
    ValuePtr value = nullptr; // Varsayılan değer none
    if (stmt.initializer) {
        value = evaluate(stmt.initializer); // Başlangıç değeri varsa değerlendir
    } else {
        value = std::make_shared<Value>(); // Yoksa default none
    }
    environment->define(stmt.name.lexeme, value); // Ortama tanımla
}

void Interpreter::visitBlockStmt(const BlockStmt& stmt) {
    // Yeni bir ortam oluştur ve bloğu çalıştır
    executeBlock(stmt.statements, std::make_shared<Environment>(this->environment));
}

void Interpreter::visitIfStmt(const IfStmt& stmt) {
    ValuePtr condition = evaluate(stmt.condition); // Koşulu değerlendir

    // isTruthy fonksiyonu ValuePtr'ı bool'a çevirmeli (Python gibi)
     bool isTruthy(ValuePtr value); // Value.h'da implement edilebilir

    // Geçici isTruthy implementasyonu:
    bool condition_is_true = false;
    if (condition) {
        if (std::holds_alternative<bool>(*condition)) {
            condition_is_true = std::get<bool>(*condition);
        } else if (std::holds_alternative<std::monostate>(*condition)) {
            condition_is_true = false; // None false'tur
        } else if (std::holds_alternative<double>(*condition)) {
             condition_is_true = std::get<double>(*condition) != 0; // Sayı 0 değilse true
        } else if (std::holds_alternative<std::string>(*condition)) {
             condition_is_true = !std::get<std::string>(*condition).empty(); // String boş değilse true
        } // Diğer tipler için varsayılan false olabilir
    }


    if (condition_is_true) {
        execute(stmt.thenBranch); // if bloğunu çalıştır
    } else if (stmt.elseBranch) {
        execute(stmt.elseBranch); // else bloğunu çalıştır (varsa)
    }
}

void Interpreter::visitWhileStmt(const WhileStmt& stmt) {
     // Geçici isTruthy kullanılıyor
    while (true) {
         ValuePtr condition = evaluate(stmt.condition);
         bool condition_is_true = false;
         if (condition) {
             if (std::holds_alternative<bool>(*condition)) {
                 condition_is_true = std::get<bool>(*condition);
             } else if (std::holds_alternative<std::monostate>(*condition)) {
                 condition_is_true = false;
             } else if (std::holds_alternative<double>(*condition)) {
                  condition_is_true = std::get<double>(*condition) != 0;
             } else if (std::holds_alternative<std::string>(*condition)) {
                  condition_is_true = !std::get<std::string>(*condition).empty();
             }
         }

         if (!condition_is_true) break; // Koşul false ise döngüden çık

         execute(stmt.body); // Döngü gövdesini çalıştır
    }
}

// Fonksiyon, Return, Sınıf, Import, Match deyimleri daha sonra implement edilecek...
void Interpreter::visitFunDeclStmt(const FunDeclStmt& stmt) {
     // Fonksiyon tanımlama implementasyonu (fonksiyon objesi oluşturulmalı)
     std::cerr << "Warning: Function declaration not fully implemented." << std::endl;
}

void Interpreter::visitReturnStmt(const ReturnStmt& stmt) {
     // Return deyimi implementasyonu (istisna fırlatmak yaygın bir yöntemdir)
      std::cerr << "Warning: Return statement not fully implemented." << std::endl;
       ValuePtr value = nullptr;
       if (stmt.value) value = evaluate(stmt.value);
       throw ReturnException(value); // Özel bir istisna sınıfı
}

void Interpreter::visitClassDeclStmt(const ClassDeclStmt& stmt) {
      // Sınıf tanımı implementasyonu (sınıf objesi oluşturulmalı)
       std::cerr << "Warning: Class declaration not fully implemented." << std::endl;
}

void Interpreter::visitImportStmt(const ImportStmt& stmt) {
    // Import deyimi implementasyonu (dosyayı bul, parse et, çalıştır)
    // Çok dilli import çok daha sonra implement edilecek
     std::cerr << "Warning: Import statement not fully implemented." << std::endl;
}

void Interpreter::visitMatchStmt(const MatchStmt& stmt) {
    // Match deyimi implementasyonu
     std::cerr << "Warning: Match statement not fully implemented." << std::endl;
     // Evaluate the subject
     // Iterate through cases
     // Evaluate pattern of each case
     // Check if subject matches pattern
     // If match, execute case body and break
}


// ExprVisitor Metodları Implementasyonu

ValuePtr Interpreter::visitAssignExpr(const AssignExpr& expr) {
    ValuePtr value = evaluate(expr.value); // Atanacak değeri değerlendir
    // Değişkeni ortamda ara ve değeri ata
    environment->assign(expr.name, value);
    return value; // Atanan değeri döndür (Python gibi)
}

ValuePtr Interpreter::visitBinaryExpr(const BinaryExpr& expr) {
    ValuePtr left = evaluate(expr.left);   // Sol operandı değerlendir
    ValuePtr right = evaluate(expr.right); // Sağ operandı değerlendir

    // İkili operatör mantığı (Tip kontrolü ve hata yönetimi çok önemli!)
    // Şu an sadece sayısal operatörler ve string birleştirme örneği

    switch (expr.op.type) {
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL: {
            // Karşılaştırma operatörleri
            // Operandların aynı tipte ve karşılaştırılabilir olduğunu varsayalım
            if (std::holds_alternative<double>(*left) && std::holds_alternative<double>(*right)) {
                double l = std::get<double>(*left);
                double r = std::get<double>(*right);
                switch (expr.op.type) {
                    case TokenType::GREATER:       return std::make_shared<Value>(l > r);
                    case TokenType::GREATER_EQUAL: return std::make_shared<Value>(l >= r);
                    case TokenType::LESS:          return std::make_shared<Value>(l < r);
                    case TokenType::LESS_EQUAL:    return std::make_shared<Value>(l <= r);
                    case TokenType::EQUAL_EQUAL:   return std::make_shared<Value>(l == r);
                    case TokenType::BANG_EQUAL:    return std::make_shared<Value>(l != r);
                    default: break; // Ulaşılmamalı
                }
            } else if (std::holds_alternative<std::string>(*left) && std::holds_alternative<std::string>(*right)) {
                 std::string l = std::get<std::string>(*left);
                 std::string r = std::get<std::string>(*right);
                 switch (expr.op.type) {
                    case TokenType::EQUAL_EQUAL: return std::make_shared<Value>(l == r);
                    case TokenType::BANG_EQUAL:  return std::make_shared<Value>(l != r);
                    default:
                         // Stringler için sadece eşitlik/eşitsizlik tanımlı olsun
                         std::cerr << "[Line " << expr.op.line << "] Runtime Error: Invalid operator for strings." << std::endl;
                         / throw std::runtime_error("Invalid operator for strings.");
                         return nullptr; // Hata durumunda
                 }
            }
            // Diğer tip karşılaştırmaları (bool, None vb.) buraya eklenebilir
             else if (std::holds_alternative<bool>(*left) && std::holds_alternative<bool>(*right)) {
                bool l = std::get<bool>(*left);
                bool r = std::get<bool>(*right);
                 switch (expr.op.type) {
                    case TokenType::EQUAL_EQUAL: return std::make_shared<Value>(l == r);
                    case TokenType::BANG_EQUAL:  return std::make_shared<Value>(l != r);
                     default:
                        std::cerr << "[Line " << expr.op.line << "] Runtime Error: Invalid operator for booleans." << std::endl;
                         return nullptr;
                 }
            } else {
                // Uyumsuz tipler hatası
                 runtimeError(expr.op, "Operands must be numbers for arithmetic operations.");
                 std::cerr << "[Line " << expr.op.line << "] Runtime Error: Incompatible types for comparison." << std::endl;
                 throw std::runtime_error("Incompatible types for comparison.");
                return nullptr; // Hata durumunda
            }
            break;
        }
        case TokenType::MINUS:
        case TokenType::SLASH:
        case TokenType::STAR: {
             // Aritmetik operatörler (Sadece sayıları desteklesin)
            if (std::holds_alternative<double>(*left) && std::holds_alternative<double>(*right)) {
                 double l = std::get<double>(*left);
                 double r = std::get<double>(*right);
                 switch (expr.op.type) {
                     case TokenType::MINUS: return std::make_shared<Value>(l - r);
                     case TokenType::SLASH:
                         if (r == 0) {
                             runtimeError(expr.op, "Division by zero.");
                            std::cerr << "[Line " << expr.op.line << "] Runtime Error: Division by zero." << std::endl;
                             throw std::runtime_error("Division by zero.");
                            return nullptr; // Hata durumunda
                         }
                         return std::make_shared<Value>(l / r);
                     case TokenType::STAR: return std::make_shared<Value>(l * r);
                     default: break; // Ulaşılmamalı
                 }
            } else {
                // Uyumsuz tipler hatası
                 runtimeError(expr.op, "Operands must be numbers for arithmetic operations.");
                 std::cerr << "[Line " << expr.op.line << "] Runtime Error: Operands must be numbers for arithmetic operations." << std::endl;
                 throw std::runtime_error("Operands must be numbers for arithmetic operations.");
                return nullptr; // Hata durumunda
            }
            break;
        }
         case TokenType::PLUS: {
              // Toplama operatörü (Sayısal toplama veya string birleştirme)
             if (std::holds_alternative<double>(*left) && std::holds_alternative<double>(*right)) {
                 return std::make_shared<Value>(std::get<double>(*left) + std::get<double>(*right));
             } else if (std::holds_alternative<std::string>(*left) && std::holds_alternative<std::string>(*right)) {
                 return std::make_shared<Value>(std::get<std::string>(*left) + std::get<std::string>(*right));
             } else {
                 // Uyumsuz tipler hatası
                 std::cerr << "[Line " << expr.op.line << "] Runtime Error: Operands must be two numbers or two strings." << std::endl;
                 return nullptr; // Hata durumunda
             }
             break;
         }
        default: break; // Diğer operatörler (mantıksal vb.) burada ele alınacak
    }

    // Ulaşılmamalı
    return nullptr;
}

ValuePtr Interpreter::visitCallExpr(const CallExpr& expr) {
     // Fonksiyon/Metot çağrısı implementasyonu
      std::cerr << "Warning: Function call not fully implemented." << std::endl;
    // Evaluate the callee (the expression that produces the function/object)
    // Evaluate the arguments
    // Check if the callee is callable (is a function object)
    // Call the callable object with evaluated arguments
     return nullptr;
}

ValuePtr Interpreter::visitGetExpr(const GetExpr& expr) {
    // Obje özelliği alma implementasyonu
     std::cerr << "Warning: Object property get not fully implemented." << std::endl;
    // Evaluate the object expression
    // Check if the result is an object
    // Get the property from the object
     return nullptr;
}

ValuePtr Interpreter::visitGroupingExpr(const GroupingExpr& expr) {
    return evaluate(expr.expression); // Parantez içindeki ifadeyi değerlendir
}

ValuePtr Interpreter::visitLiteralExpr(const LiteralExpr& expr) {
    // Literal değeri döndür (Token'daki lexeme'den ValuePtr oluştur)
    // Token'da literal saklıyorsak onu direk döndürürüz
    // Şimdilik lexeme'den ValuePtr oluşturuyoruz:
    if (expr.value.type == TokenType::NUMBER) {
        // Lexeme'yi double'a çevir
        try {
            return std::make_shared<Value>(std::stod(expr.value.lexeme));
        } catch (const std::exception& e) {
            // Sayı çevirme hatası (lexical analizde yakalanmalıydı aslında)
            std::cerr << "[Line " << expr.value.line << "] Runtime Error: Invalid number literal '" << expr.value.lexeme << "'." << std::endl;
             return nullptr;
        }
    } else if (expr.value.type == TokenType::STRING) {
        // Lexeme zaten string değeri
        return std::make_shared<Value>(expr.value.lexeme);
    } else if (expr.value.type == TokenType::TRUE) {
        return std::make_shared<Value>(true);
    } else if (expr.value.type == TokenType::FALSE) {
        return std::make_shared<Value>(false);
    } else if (expr.value.type == TokenType::NONE) {
        return std::make_shared<Value>(); // std::monostate içeren Value (None)
    }

    // Diğer literal tipleri (ileride eklenecekler)
    return nullptr; // Bilinmeyen literal tipi
}

ValuePtr Interpreter::visitLogicalExpr(const LogicalExpr& expr) {
    ValuePtr left = evaluate(expr.left); // Sol operandı değerlendir

    // Mantıksal 'or' veya 'and' kısa devre mantığı
    if (expr.op.type == TokenType::OR) {
        // Eğer sol operand 'truthy' ise, sağ operandı değerlendirmeye gerek yok
        if (isTruthy(left)) return left;
    } else { // TokenType::AND
        // Eğer sol operand 'falsy' ise, sağ operandı değerlendirmeye gerek yok
        if (!isTruthy(left)) return left;
    }

    // Kısa devre olmadıysa, sağ operandı değerlendir
    return evaluate(expr.right);
}

ValuePtr Interpreter::visitSetExpr(const SetExpr& expr) {
     // Obje özelliği atama implementasyonu
      std::cerr << "Warning: Object property set not fully implemented." << std::endl;
    // Evaluate the object expression
    // Check if the result is an object
    // Evaluate the value to be assigned
    // Set the property on the object
     return nullptr;
}

ValuePtr Interpreter::visitSuperExpr(const SuperExpr& expr) {
     // Super anahtar kelimesi implementasyonu
      std::cerr << "Warning: 'super' not fully implemented." << std::endl;
     return nullptr;
}

ValuePtr Interpreter::visitThisExpr(const ThisExpr& expr) {
     // this anahtar kelimesi implementasyonu
      std::cerr << "Warning: 'this' not fully implemented." << std::endl;
    // Resolve 'this' to the current instance object
     return nullptr;
}

ValuePtr Interpreter::visitUnaryExpr(const UnaryExpr& expr) {
    ValuePtr right = evaluate(expr.right); // Sağ operandı değerlendir

    switch (expr.op.type) {
        case TokenType::BANG: // Mantıksal NOT (!)
              return std::make_shared<Value>(!isTruthy(right));
             if (right) return std::make_shared<Value>(!isTruthy(right));
             break;
        case TokenType::MINUS: // Negatif sayı (-)
             // Sadece sayıları desteklesin
             if (right && std::holds_alternative<double>(*right)) {
                 return std::make_shared<Value>(-std::get<double>(*right));
             } else {
                 // Uyumsuz tip hatası
                  std::cerr << "[Line " << expr.op.line << "] Runtime Error: Operand must be a number for unary minus." << std::endl;
                  throw std::runtime_error("Operand must be a number for unary minus.");
                 return nullptr;
             }
             break;
        default: break; // Ulaşılmamalı
    }
     // Ulaşılmamalı
    return nullptr;
}

ValuePtr Interpreter::visitVariableExpr(const VariableExpr& expr) {
    // Değişkenin değerini ortamdan al
    return environment->get(expr.name);
}

ValuePtr Interpreter::visitMatchExpr(const MatchExpr& expr) {
     // Match ifadesi (expression olarak) implementasyonu
      std::cerr << "Warning: Match expression not fully implemented." << std::endl;
     // Similar logic to visitMatchStmt, but returns the value of the matched case's expression
     return nullptr;
}


// Yardımcı Fonksiyonlar Implementasyonu

// ValuePtr'ı bool'a çevirir (Python'daki truthiness kurallarına yakın)
bool Interpreter::isTruthy(ValuePtr value) {
    if (!value) return false; // Null pointer false
    if (std::holds_alternative<std::monostate>(*value)) return false; // None false
    if (std::holds_alternative<bool>(*value)) return std::get<bool>(*value); // Boolean değeri
    if (std::holds_alternative<double>(*value)) return std::get<double>(*value) != 0; // Sayı 0 değilse true
    if (std::holds_alternative<std::string>(*value)) return !std::get<std::string>(*value).empty(); // String boş değilse true

    // Diğer tipler için varsayılan true olabilir (Python'daki gibi listeler, objeler vb.)
    return true;
}

// ValuePtr'ı string'e çevirir (yazdırmak için) - daha robust implementasyon gerekebilir
 std::string Interpreter::valueToString(ValuePtr value) {
//    // Implement this based on Value variants
     if (!value) return "null";
     if (std::holds_alternative<std::monostate>(*value)) return "none";
     if (std::holds_alternative<bool>(*value)) return std::get<bool>(*value) ? "true" : "false";
     if (std::holds_alternative<double>(*value)) return std::to_string(std::get<double>(*value));
     if (std::holds_alternative<std::string>(*value)) return std::get<std::string>(*value);
//     // Diğer tipler için uygun çeviriler
     return "[object or unknown type]";
 }

// Runtime Hata Raporlama (main.cpp'de olmalı)

void runtimeError(const Token& token, const std::string& message) {
    std::cerr << "[Line " << token.line << "] Runtime Error: " << message << std::endl;
    hadRuntimeError = true;
}
Interpreter::Interpreter(ErrorReporter& errorReporter, const std::vector<std::string>& moduleSearchPaths)
    : errorReporter(errorReporter), // varsayalım ki errorReporter üyesi var
      globals(std::make_shared<Environment>()), // global ortam
      environment(globals), // başlangıçta mevcut ortam global'e eşit
      moduleLoader(*this, moduleSearchPaths) // ModuleLoader'ı başlat
{
    // Built-in fonksiyonları burada veya ayrı bir fonksiyonda tanımla
    // builtins.registerBuiltins(globals); // Eğer böyle bir metodunuz varsa
}
void Interpreter::visitImportStmt(const ImportStmt& stmt) {
    // Modül adını çözümle (örn. "game.utils" -> "game/utils")
    std::string modulePath = stmt.moduleName.lexeme; // Token'dan al

    ModulePtr importedModule = moduleLoader.loadModule(modulePath);

    if (!importedModule) {
        // Hata zaten ModuleLoader tarafından raporlanmış olmalı
        throw RuntimeException(stmt.moduleName, "Failed to import module '" + modulePath + "'.");
    }

    // C-CUBE'da modülün nasıl temsil edileceğine karar verin:
    // 1. Modülün global ortamını mı döndürüyor?
    // 2. Modülü temsil eden özel bir C_CUBE_Module nesnesi mi döndürüyor?

    // Eğer ModuleLoader'ın `readModule` metodu doğrudan bir `ValuePtr` (C_CUBE_Module objesi) döndürseydi,
    // bu daha temiz olurdu. Şimdilik varsayalım ki `ModulePtr` bir `C_CUBE_Module` objesidir.
    // Ve bu objenin "export" ettiği değerlere bir şekilde erişilebilir.

    // Modülü yerel ortamda veya global ortamda bir değişkene atayın
    // Örn: `import foo` -> `foo` değişkenine modül objesini ata
    // `import foo as bar` -> `bar` değişkenine modül objesini ata
    std::string alias = stmt.alias.has_value() ? stmt.alias.value().lexeme : stmt.moduleName.lexeme;
    environment->define(alias, std::make_shared<Value>(importedModule)); // importedModule'ın kendisi bir ValuePtr olmalı
                                                                          // veya bir C_CUBE_Module'ı Value'ye sar.
                                                                          // Burada `Value` içine `ModulePtr`'ı sarmalamak mantıklı.
}
