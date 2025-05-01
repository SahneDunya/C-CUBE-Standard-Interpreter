#ifndef CUBE_DATA_TYPES_TYPE_HPP
#define CUBE_DATA_TYPES_TYPE_HPP

#include <string>
#include <vector>
#include <memory> // unique_ptr için
#include <unordered_map> // İleride sınıf üyeleri için

namespace CCube {

// İleri bildirim (forward declaration)
struct Type;

// C-CUBE dilindeki temel ve karmaşık tiplerin kimlikleri
enum class TypeId {
    UNKNOWN, VOID, INTEGER, FLOAT, STRING, BOOLEAN, NONE,
    CLASS, FUNCTION, LIST, DICTIONARY,
    // Gelecekte eklenecek tipler: ARRAY, TUPLE, CUSTOM_STRUCT vb.
};

// C-CUBE dilindeki bir tipi temsil eden temel yapı
struct Type {
    TypeId id;
    std::string name; // Kullanıcı dostu isim (örn: "int", "MyClass")

    Type(TypeId id, const std::string& name) : id(id), name(name) {}
    virtual ~Type() = default; // Sanal yıkıcı

    // Tipler arası atama uyumluluğunu kontrol eder
    // Örn: int'e float atanabilir mi? (hayır veya evet, dile bağlı)
    virtual bool isAssignableTo(const Type& target_type) const {
        // Varsayılan: Sadece aynı tipse atanabilir (ileride genişletilecek)
        return this->id == target_type.id;
    }

    // Tipler arası örtülü/açık dönüşüm uyumluluğunu kontrol eder
    // Örn: int float'a dönüştürülebilir mi?
    virtual bool isConvertibleTo(const Type& target_type) const {
         // Varsayılan: Sadece aynı tipse dönüştürülebilir (ileride genişletilecek)
         return this->id == target_type.id;
    }

    // Tiplerin eşitliğini kontrol eder
    bool operator==(const Type& other) const { return id == other.id; }
    bool operator!=(const Type& other) const { return !(*this == other); }

    // Hata ayıklama için tipin string gösterimi
    std::string toString() const { return name; }

     // AST düğümlerine eklemek için unique_ptr factory fonksiyonları
     static std::unique_ptr<Type> createUnknown() { return std::make_unique<Type>(TypeId::UNKNOWN, "unknown"); }
     static std::unique_ptr<Type> createVoid() { return std::make_unique<Type>(TypeId::VOID, "void"); }
     static std::unique_ptr<Type> createInteger() { return std::make_unique<Type>(TypeId::INTEGER, "int"); }
     static std::unique_ptr<Type> createFloat() { return std::make_unique<Type>(TypeId::FLOAT, "float"); }
     static std::unique_ptr<Type> createString() { return std::make_unique<Type>(TypeId::STRING, "string"); }
     static std::unique_ptr<Type> createBoolean() { return std::make_unique<Type>(TypeId::BOOLEAN, "bool"); }
     static std::unique_ptr<Type> createNone() { return std::make_unique<Type>(TypeId::NONE, "None"); }

     // Daha karmaşık tipler için factory fonksiyonları eklenecek
      static std::unique_ptr<Type> createClass(...)
      static std::unique_ptr<Type> createFunction(...)

};

// Fonksiyon tipi (parametre tipleri ve dönüş tipi içerir)
struct FunctionType : public Type {
    std::unique_ptr<Type> return_type;
    std::vector<std::unique_ptr<Type>> parameter_types;

    FunctionType(std::unique_ptr<Type> return_type, std::vector<std::unique_ptr<Type>> param_types)
        : Type(TypeId::FUNCTION, "function"), // İsim daha detaylı olabilir (örn: "(int, float) -> bool")
          return_type(std::move(return_type)),
          parameter_types(std::move(param_types)) {}

     // Fonksiyon tipleri için atama/dönüşüm kuralları (örn: imzaların uyuşması) burada override edilir.
      bool isAssignableTo(...) override;
      bool isConvertibleTo(...) override;
};

 // Sınıf tipi (Üyeleri, metotları, kalıtımı içerir)
struct ClassType : public Type {
    // Sınıf üyeleri (alanlar ve metotlar) için sembol tablosuna referans veya kopya
     std::shared_ptr<Scope> class_scope; // Kendi kapsamı olabilir
     std::vector<std::unique_ptr<ClassType>> superclasses; // Kalıtım
    std::string class_name; // Sınıf adı

    ClassType(const std::string& name) : Type(TypeId::CLASS, name), class_name(name) {}

     // Sınıf tipleri için atama/dönüşüm kuralları (örn: kalıtım) burada override edilir.
      bool isAssignableTo(...) override;
      bool isConvertibleTo(...) override;
};


// Daha karmaşık tipler için factory fonksiyonları implementasyonları type.cpp'de olacaktır.
// Bunların .hpp'de sadece deklarasyonları veya basit implementasyonları bulunabilir.


}  namespace CCube

#endif // CUBE_DATA_TYPES_TYPE_HPP