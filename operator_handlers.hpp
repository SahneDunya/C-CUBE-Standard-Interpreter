#ifndef CUBE_OPERATORS_OPERATOR_HANDLERS_HPP
#define CUBE_OPERATORS_OPERATOR_HANDLERS_HPP

#include <memory> // Belki sonuç nesneleri için shared_ptr
#include "Syntax/token.hpp"    // TokenType için
#include "Data Types/object.hpp" // Object türü için
#include "Data Types/type_system.hpp" // Runtime tip kontrolü için TypeSystem
#include "ErrorHandling/error_reporter.hpp" // Runtime hata raporlama için

// TODO: Çalışma zamanı somut Object tiplerini dahil etmeniz gerekecek (örn: IntegerObject, FloatObject, StringObject)
// Bunlar Data Types klasöründe Object'ten türetilmiş sınıflar olacaktır.
#include "Data Types/integer_object.hpp"
#include "Data Types/float_object.hpp"
#include "Data Types/string_object.hpp"
// ... diğer tipler ...


namespace CCube {

// Çalışma zamanında operatör işlemlerini gerçekleştiren işleyiciler sınıfı.
// Bu sınıf statik metotlar içerebilir.
class OperatorHandlers {
public:
    // Çalışma zamanında ikili operatör işlemini gerçekleştirir.
    // Operandlar Object* pointerlarıdır. Sonuç yeni bir Object* pointerıdır.
    // İşlem geçersizse runtime hatası raporlar ve nullptr veya özel bir hata nesnesi döndürebilir.
    static Object* handleBinary(TokenType op, Object* left, Object* right, ErrorReporter& reporter, SourceLocation loc);

    // Çalışma zamanında birli operatör işlemini gerçekleştirir.
    // Operand Object* pointerıdır. Sonuç yeni bir Object* pointerıdır.
    // İşlem geçersizse runtime hatası raporlar.
    static Object* handleUnary(TokenType op, Object* operand, ErrorReporter& reporter, SourceLocation loc);

    // TODO: Diğer operatör türleri için işleyiciler (örn: atama, nokta erişimi, indexleme vb.)
    // Atama genellikle ayrı bir deyim olarak ele alınır ancak operatör gibi syntax'ı varsa burada da olabilir.
     static void handleAssignment(Object* target, Object* value, ErrorReporter& reporter, SourceLocation loc);
     static Object* handleGet(Object* obj, const std::string& member_name, ErrorReporter& reporter, SourceLocation loc);

private:
    // Kurucu gizli (statik metotlar için instance'a gerek yok)
    OperatorHandlers() = delete;
};

} namespace CCube

#endif // CUBE_OPERATORS_OPERATOR_HANDLERS_HPP