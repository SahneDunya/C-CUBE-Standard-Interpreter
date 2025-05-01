#ifndef CUBE_DATA_TYPES_TYPE_SYSTEM_HPP
#define CUBE_DATA_TYPES_TYPE_SYSTEM_HPP

#include <string>
#include <vector>
#include <memory> // unique_ptr için
#include <unordered_map> // İleride karmaşık tipleri yönetmek için
#include "type.hpp" // Type tanımları için
#include "Syntax/token.hpp" // TokenType için (operatörler)

namespace CCube {

// Tip Sistemi Singleton
// Tüm temel tipleri yönetir ve tipler arası uyumluluk kurallarını uygular.
class TypeSystem {
public:
    // Singleton instance'ına erişim metodu
    static TypeSystem& getInstance();

    // Singleton kopyalanamaz ve atama yapılamaz
    TypeSystem(const TypeSystem&) = delete;
    TypeSystem& operator=(const TypeSystem&) = delete;

    // Temel tip singleton'larına erişim metotları (Type* pointer döndürür)
    Type* getUnknownType() const { return unknown_type_.get(); }
    Type* getVoidType() const { return void_type_.get(); }
    Type* getIntegerType() const { return integer_type_.get(); }
    Type* getFloatType() const { return float_type_.get(); }
    Type* getStringType() const { return string_type_.get(); }
    Type* getBooleanType() const { return boolean_type_.get(); }
    Type* getNoneType() const { return none_type_.get(); }

    // İkili operatörler için sonuç tipini belirler ve tip uyumluluğunu kontrol eder.
    // Uyumsuzluk durumunda nullptr veya UnknownType* dönebilir.
    Type* getBinaryResultType(Type* left, TokenType op, Type* right);

    // Birli operatörler için sonuç tipini belirler ve tip uyumluluğunu kontrol eder.
    // Uyumsuzluk durumunda nullptr veya UnknownType* dönebilir.
    Type* getUnaryResultType(TokenType op, Type* operand);

    // source tipinin target tipe atanabilir olup olmadığını kontrol eder.
    bool isAssignable(Type* target, Type* value) const;

    // source tipinin target tipe dönüştürülebilir olup olmadığını kontrol eder.
    bool isConvertible(Type* source, Type* target) const;

    // TODO: Karmaşık tipleri (ClassType, FunctionType) yönetme metotları eklenebilir.
    // Örneğin: registerClassType, createFunctionType vb.
    // Veya bu tiplerin yönetimi SymbolTable'da kalır ve TypeSystem sadece kuralları bilir.

private:
    // Özel kurucu (sadece getInstance tarafından çağrılır)
    TypeSystem();

    // Temel tip singleton'ları (TypeSystem sahipliğinde)
    std::unique_ptr<Type> unknown_type_;
    std::unique_ptr<Type> void_type_;
    std::unique_ptr<Type> integer_type_;
    std::unique_ptr<Type> float_type_;
    std::unique_ptr<Type> string_type_;
    std::unique_ptr<Type> boolean_type_;
    std::unique_ptr<Type> none_type_;

    // TODO: Kullanıcı tanımlı tipleri (sınıflar) saklamak için bir yapı eklenebilir
     std::unordered_map<std::string, std::shared_ptr<ClassType>> defined_class_types_;
};

} namespace CCube

#endif // CUBE_DATA_TYPES_TYPE_SYSTEM_HPP