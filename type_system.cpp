#include "type_system.hpp"
#include <iostream> // Hata ayıklama için

namespace CCube {

// Singleton instance'ını döndüren metot
TypeSystem& TypeSystem::getInstance() {
    static TypeSystem instance; // Lazy initialization, thread-safe in C++11+
    return instance;
}

// Özel Kurucu: Temel tip singletonlarını oluşturur
TypeSystem::TypeSystem()
    : unknown_type_(std::make_unique<Type>(TypeId::UNKNOWN, "unknown")),
      void_type_(std::make_unique<Type>(TypeId::VOID, "void")),
      integer_type_(std::make_unique<IntegerType>()), // IntegerType::IntegerType()
      float_type_(std::make_unique<FloatType>()),     // FloatType::FloatType()
      string_type_(std::make_unique<StringType>()),   // StringType::StringType()
      boolean_type_(std::make_unique<BooleanType>()), // BooleanType::BooleanType()
      none_type_(std::make_unique<NoneType>())         // NoneType::NoneType()
{
     std::cout << "[DEBUG] TypeSystem initialized." << std::endl; // Hata ayıklama
}


// İkili operatörler için sonuç tipini belirler ve tip uyumluluğunu kontrol eder.
// Temel C-CUBE tip kuralları burada tanımlanır.
Type* TypeSystem::getBinaryResultType(Type* left, TokenType op, Type* right) {
    if (!left || !right) return getUnknownType(); // İşlenenlerden biri bilinmeyen tipte

    // Her operatör için tip kurallarını yaz
    switch (op) {
        case TokenType::PLUS: // +
            if (left->id == TypeId::STRING && right->id == TypeId::STRING) return getStringType(); // String birleştirme
            if ((left->id == TypeId::INTEGER || left->id == TypeId::FLOAT) && (right->id == TypeId::INTEGER || right->id == TypeId::FLOAT)) {
                // Sayısal toplama (int + int = int, float + x = float)
                return (left->id == TypeId::FLOAT || right->id == TypeId::FLOAT) ? getFloatType() : getIntegerType();
            }
            break; // Uyumsuz tip

        case TokenType::MINUS: // -
        case TokenType::STAR:  // *
        case TokenType::SLASH: // /
        case TokenType::PERCENT: // %
            if ((left->id == TypeId::INTEGER || left->id == TypeId::FLOAT) && (right->id == TypeId::INTEGER || right->id == TypeId::FLOAT)) {
                // Sayısal işlemler (int + int = int, float + x = float)
                return (left->id == TypeId::FLOAT || right->id == TypeId::FLOAT) ? getFloatType() : getIntegerType();
            }
            break; // Uyumsuz tip

        case TokenType::EQ:  // ==
        case TokenType::NEQ: // !=
            // Eşitlik/Eşitsizlik (Temel tipler ve referanslar arasında olabilir)
            // Genellikle tipler dönüştürülebiliyorsa karşılaştırılabilir.
            if (isConvertible(left, right) || isConvertible(right, left)) {
                 return getBooleanType();
            }
            break; // Uyumsuz tip

        case TokenType::LT:  // <
        case TokenType::GT:  // >
        case TokenType::LTE: // <=
        case TokenType::GTE: // >=
             // Karşılaştırma (Sayısal tipler, stringler, booleanlar, None arasında olabilir)
             // Genellikle tipler dönüştürülebiliyorsa karşılaştırılabilir.
            if ((left->id == TypeId::INTEGER || left->id == TypeId::FLOAT) && (right->id == TypeId::INTEGER || right->id == TypeId::FLOAT)) {
                 return getBooleanType(); // Sayısal karşılaştırma
            }
            // TODO: String karşılaştırma, Boolean karşılaştırma, None karşılaştırma kuralları eklenecek.
            break; // Uyumsuz tip

        case TokenType::AND: // and
        case TokenType::OR:  // or
            // Mantıksal AND/OR (Boolean tipleri arasında)
            if (left->id == TypeId::BOOLEAN && right->id == TypeId::BOOLEAN) {
                return getBooleanType();
            }
            break; // Uyumsuz tip

        // TODO: Diğer ikili operatörler (örn: bitwise operators &, |, ^, kaydırma >>, <<)

        default:
            // Bilinmeyen operatör
             // Hata raporlama burada yapılmamalı, SemanticAnalyzer yapmalı.
            break;
    }

     // Kurala uymayan tip kombinasyonu veya bilinmeyen operatör
    return getUnknownType();
}

// Birli operatörler için sonuç tipini belirler ve tip uyumluluğunu kontrol eder.
Type* TypeSystem::getUnaryResultType(TokenType op, Type* operand) {
    if (!operand) return getUnknownType();

    switch (op) {
        case TokenType::MINUS: // -
            // Negatif işareti (Sayısal tiplerde)
            if (operand->id == TypeId::INTEGER) return getIntegerType();
            if (operand->id == TypeId::FLOAT) return getFloatType();
            break; // Uyumsuz tip

        case TokenType::NOT: // not
            // Mantıksal NOT (Boolean tipinde)
            if (operand->id == TypeId::BOOLEAN) return getBooleanType();
             // Python'da her şey boolean'a dönüştürülebilir, C-CUBE da öyleyse kural değişir.
            break; // Uyumsuz tip

        // TODO: Diğer birli operatörler (örn: bitwise NOT ~, dereference *, address-of &)

        default:
            // Bilinmeyen operatör
            break;
    }

    return getUnknownType();
}

// source tipinin target tipe atanabilir olup olmadığını kontrol eder.
// Burası, dilin atama kurallarının merkezi.
bool TypeSystem::isAssignable(Type* target, Type* value) const {
    if (!target || !value) return false;

    // Aynı tip her zaman atanabilir
    if (target->id == value->id) return true;

    // Özel atama kuralları
    // Örnek: Integer float'a atanabilir mi? (C-CUBE'un Python'a yakınlığı göz önüne alınırsa belki hayır, belki evet)
    // Şimdilik basit tutalım: Sadece exact match veya açıkça tanımlanmış kurallar.
     if (target->id == TypeId::FLOAT && value->id == TypeId::INTEGER) return true; // Örnek kural: int -> float ataması geçerli

    // TODO: Daha karmaşık kurallar:
    // - None herhangi bir tipe atanabilir mi? (Python'da genellikle hayır, None tipi özeldir)
    // - Kalıtım: Türetilmiş sınıfın nesnesi baz sınıf değişkenine atanabilir mi?
    // - Fonksiyon tipleri arası atama (parametre/dönüş tipi uyumluluğu)
    // - List/Dict atamaları (element tipleri uyumlu mu?)

    // Varsayılan: Açıkça izin verilmeyen atamalar geçersizdir.
    return false;
}

// source tipinin target tipe dönüştürülebilir olup olmadığını kontrol eder.
// Burası, dilin tip dönüşüm kurallarının merkezi.
bool TypeSystem::isConvertible(Type* source, Type* target) const {
     if (!source || !target) return false;

     // Aynı tip her zaman dönüştürülebilir (kendine dönüşüm)
     if (source->id == target->id) return true;

     // Özel dönüşüm kuralları
     // Örnek: Integer float'a dönüştürülebilir
     if (source->id == TypeId::INTEGER && target->id == TypeId::FLOAT) return true;
     // Örnek: Float integer'a dönüştürülebilir (veri kaybıyla)
     if (source->id == TypeId::FLOAT && target->id == TypeId::INTEGER) return true;
     // Örnek: Boolean integer'a dönüştürülebilir (true=1, false=0)
     if (source->id == TypeId::BOOLEAN && target->id == TypeId::INTEGER) return true;
     // Örnek: Herhangi bir tip string'e dönüştürülebilir (toString gibi)
      if (target->id == TypeId::STRING) return true;

     // TODO: Daha karmaşık kurallar:
     // - None'un diğer tiplere dönüşümü
     // - Obje dönüşümleri (type casting)
     // - Koleksiyon dönüşümleri

     // Varsayılan: Açıkça izin verilmeyen dönüşümler geçersizdir.
     return false;
}


} namespace CCube