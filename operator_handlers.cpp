#include "operator_handlers.hpp"
#include <cmath> // pow fonksiyonu için (üs alma operatörü varsa)
#include <limits> // Sayısal sınırlar için
#include <stdexcept> // Runtime hataları için exception kullanılabilir

// TODO: Çalışma zamanı somut Object tiplerini dahil etmeniz gerekecek
// Örnek olarak eklenmiştir, kendi dosya yollarınızla değiştirin.
 #include "Data Types/IntegerObject.hpp"
 #include "Data Types/FloatObject.hpp"
 #include "Data Types/StringObject.hpp"
 #include "Data Types/BooleanObject.hpp"
 #include "Data Types/NoneObject.hpp" // Eğer None da Object'ten türuyorsa

namespace CCube {

// Çalışma zamanında ikili operatör işlemini gerçekleştirir.
Object* OperatorHandlers::handleBinary(TokenType op, Object* left, Object* right, ErrorReporter& reporter, SourceLocation loc) {
    // Operand pointerları null ise hata
    if (!left || !right) {
        reporter.reportError(loc, "Internal Error: Null operand(s) in binary operation.");
        return nullptr; // veya özel bir hata nesnesi döndür
    }

    // Runtime tiplerini al
    Type* left_type = left->getRuntimeType();
    Type* right_type = right->getRuntimeType();

    // TODO: Tip uyumluluğunu kontrol etmek ve işlemi yapmak için TypeSystem'ı kullan
    // TypeSystem::getInstance().getBinaryResultType(left_type, op, right_type); çağrısı semantic analyzer'daydı.
    // Burada ise *runtime* tiplerine göre *gerçek işlemi* yapmalıyız.

    // Örnek Implementasyon (Sadece Temel Sayısal ve String Operatörler)
    switch (op) {
        case TokenType::PLUS: // +
            // String birleştirme
            if (left_type->id == TypeId::STRING && right_type->id == TypeId::STRING) {
                // TODO: left ve right'ı StringObject*'a cast et ve string birleştirme yap.
                 StringObject* left_str = static_cast<StringObject*>(left);
                 StringObject* right_str = static_cast<StringObject*>(right);
                 std::string result_string = left_str->getValue() + right_str->getValue();
                 return new StringObject(result_string, TypeSystem::getInstance().getStringType()); // Yeni string nesnesi oluştur
                reporter.reportWarning(loc, "String concatenation handler not fully implemented.");
                 return nullptr; // Yer tutucu
            }
            // Sayısal toplama
            if ((left_type->id == TypeId::INTEGER || left_type->id == TypeId::FLOAT) && (right_type->id == TypeId::INTEGER || right_type->id == TypeId::FLOAT)) {
                // TODO: left ve right'ı uygun sayısal tiplere cast et ve toplama yap.
                // int/float değerlerini al, işleme koy, sonuca göre yeni IntegerObject veya FloatObject oluştur.
                // Örnek (basit, cast eksik):
                 if (left_type->id == TypeId::FLOAT || right_type->id == TypeId::FLOAT) {
                     float result = getFloatValue(left) + getFloatValue(right);
                     return new FloatObject(result, TypeSystem::getInstance().getFloatType());
                 } else {
                    int result = getIntegerValue(left) + getIntegerValue(right);
                    return new IntegerObject(result, TypeSystem::getInstance().getIntegerType());
                }
                 reporter.reportWarning(loc, "Numeric addition handler not fully implemented.");
                 return nullptr; // Yer tutucu
            }
            break; // Uyumsuz tip

        case TokenType::MINUS: // -
        case TokenType::STAR:  // *
        case TokenType::SLASH: // /
        case TokenType::PERCENT: // %
             if ((left_type->id == TypeId::INTEGER || left_type->id == TypeId::FLOAT) && (right_type->id == TypeId::INTEGER || right_type->id == TypeId::FLOAT)) {
                // TODO: Sayısal çıkarma/çarpma/bölme/modül işlemlerini yap. Bölmede sıfıra bölme kontrolü yap!
                  if (op == TokenType::SLASH && (getFloatValue(right) == 0.0)) {
                     reporter.reportError(loc, "Division by zero.");
                     return nullptr; // veya hata nesnesi
                  }
                 reporter.reportWarning(loc, "Numeric subtraction/multiplication/division/modulo handler not fully implemented.");
                 return nullptr; // Yer tutucu
            }
            break; // Uyumsuz tip

        case TokenType::EQ:  // ==
        case TokenType::NEQ: // !=
            // Eşitlik/Eşitsizlik kontrolü. Object::equals metodu veya tipe özel kontrol kullanılır.
            // TODO: left->equals(right) çağrısı yap veya tiplere göre karşılaştırma mantığını yaz.
             bool result = (op == TokenType::EQ) ? left->equals(right) : !left->equals(right);
             return new BooleanObject(result, TypeSystem::getInstance().getBooleanType());
            reporter.reportWarning(loc, "Equality/Inequality handler not fully implemented.");
            return nullptr; // Yer tutucu

        case TokenType::LT:  // <
        case TokenType::GT:  // >
        case TokenType::LTE: // <=
        case TokenType::GTE: // >=
             // Karşılaştırma işlemleri. Sayısal, string vb. tipler için karşılaştırma mantığı.
             if ((left_type->id == TypeId::INTEGER || left_type->id == TypeId::FLOAT) && (right_type->id == TypeId::INTEGER || right_type->id == TypeId::FLOAT)) {
                 // TODO: Sayısal karşılaştırma yap.
                  bool result = (op == TokenType::LT) ? (getFloatValue(left) < getFloatValue(right)) : ...;
                  return new BooleanObject(result, TypeSystem::getInstance().getBooleanType());
                  reporter.reportWarning(loc, "Numeric comparison handler not fully implemented.");
                 return nullptr; // Yer tutucu
             }
            // TODO: String, Boolean, None karşılaştırması eklenecek.
            break; // Uyumsuz tip

        case TokenType::AND: // and
        case TokenType::OR:  // or
            // Mantıksal AND/OR. Boolean tiplerinde çalışır. Python'da kısa devre değerlendirme vardır.
            if (left_type->id == TypeId::BOOLEAN && right_type->id == TypeId::BOOLEAN) {
                 // TODO: Boolean değerlerini al ve işlemi yap. Kısa devre değerlendirmeyi simüle etmeniz gerekebilir.
                  bool left_val = static_cast<BooleanObject*>(left)->getValue();
                  bool right_val = static_cast<BooleanObject*>(right)->getValue();
                  bool result = (op == TokenType::AND) ? (left_val && right_val) : (left_val || right_val);
                  return new BooleanObject(result, TypeSystem::getInstance().getBooleanType());
                  reporter.reportWarning(loc, "Logical AND/OR handler not fully implemented.");
                 return nullptr; // Yer tutucu
            }
            break; // Uyumsuz tip

        // TODO: Diğer operatörler eklenecek.

        default:
            // Bilinmeyen operatör (semantic analysis aşamasında yakalanmalı ama runtime'da da kontrol)
            reporter.reportError(loc, "Internal Error: Unhandled binary operator in runtime handler.");
            return nullptr;
    }

    // Desteklenmeyen tip kombinasyonu
    reporter.reportError(loc, "Unsupported operand types '" + left_type->toString() + "' and '" + right_type->toString() + "' for operator '" + tokenTypeToString(op) + "'.");
    return nullptr; // veya özel bir hata nesnesi
}

// Çalışma zamanında birli operatör işlemini gerçekleştirir.
Object* OperatorHandlers::handleUnary(TokenType op, Object* operand, ErrorReporter& reporter, SourceLocation loc) {
     if (!operand) {
         reporter.reportError(loc, "Internal Error: Null operand in unary operation.");
         return nullptr;
     }

     Type* operand_type = operand->getRuntimeType();

     switch (op) {
         case TokenType::MINUS: // -
             // Negatif işareti
             if (operand_type->id == TypeId::INTEGER) {
                 // TODO: IntegerObject*'a cast et, değeri negatifsini al, yeni IntegerObject oluştur.
                 reporter.reportWarning(loc, "Unary minus (int) handler not fully implemented.");
                 return nullptr; // Yer tutucu
             }
             if (operand_type->id == TypeId::FLOAT) {
                 // TODO: FloatObject*'a cast et, değeri negatifsini al, yeni FloatObject oluştur.
                  reporter.reportWarning(loc, "Unary minus (float) handler not fully implemented.");
                 return nullptr; // Yer tutucu
             }
             break; // Uyumsuz tip

         case TokenType::NOT: // not
             // Mantıksal NOT
             if (operand_type->id == TypeId::BOOLEAN) {
                  // TODO: BooleanObject*'a cast et, değeri tersini al, yeni BooleanObject oluştur.
                  reporter.reportWarning(loc, "Logical NOT handler not fully implemented.");
                 return nullptr; // Yer tutucu
             }
              // Python'da her şey boolean'a dönüştürülebilir, burada da öyleyse kural değişir.
             break; // Uyumsuz tip

         // TODO: Diğer birli operatörler eklenecek.

         default:
             // Bilinmeyen operatör
             reporter.reportError(loc, "Internal Error: Unhandled unary operator in runtime handler.");
             return nullptr;
     }

     // Desteklenmeyen tip
     reporter.reportError(loc, "Unsupported operand type '" + operand_type->toString() + "' for unary operator '" + tokenTypeToString(op) + "'.");
     return nullptr; // veya özel bir hata nesnesi
}

// TODO: Diğer işleyici metotları (örn: handleAssignment, handleGet vb.) implementasyonları buraya gelecek.

} namespace CCube