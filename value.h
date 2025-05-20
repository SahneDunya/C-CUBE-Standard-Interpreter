#ifndef C_CUBE_VALUE_H
#define C_CUBE_VALUE_H

#include <string>
#include <vector>
#include <memory>
#include <variant> // C++17 ve sonrası için
#include "c_cube_module.h" // C_CUBE_ModulePtr için

// İleri bildirimler
class GcObject;
class C_CUBE_Object;
class C_CUBE_Function;
class C_CUBE_Class;
class C_CUBE_Module; // Yeni eklenen modül sınıfı
class Callable;

// C-CUBE runtime değerlerini temsil eden sınıf/yapı
 std::variant kullanmak farklı tipleri tutmak için iyi bir yoldur
using Value = std::variant<
    std::monostate, // None / null
    bool,
    double, // Sayılar
    std::string
     std::shared_ptr<C_CUBE_Object>,
     std::shared_ptr<C_CUBE_Function>
    std::shared_ptr<C_CUBE_Object>,
    std::shared_ptr<C_CUBE_Function>,
    std::shared_ptr<C_CUBE_Class>,
    std::shared_ptr<Callable>,
    std::shared_ptr<C_CUBE_Module>
    std::shared_ptr<C_CUBE_List>,
    std::shared_ptr<C_CUBE_Dict>,
    // ... diğer tipler (listeler, dictler vb.)
>;
using ValuePtr = std::shared_ptr<Value>; // Değerlere işaretçiler

// Değerleri yazdırmak veya karşılaştırmak gibi helper fonksiyonları burada olabilir
 std::string valueToString(ValuePtr value);
 bool isTruthy(ValuePtr value); // Koşullu ifadeler için (Python'daki gibi true/false değerlendirme)
 bool isEqual(ValuePtr v1, ValuePtr v2);

using ModulePtr = std::shared_ptr<C_CUBE_Module>; // veya sadece shared_ptr<Module>

using ValueType = std::variant<
    std::monostate, // Represents 'none'
    bool,
    double,
    std::string,
    std::shared_ptr<C_CUBE_Object>,
    std::shared_ptr<C_CUBE_Function>,
    std::shared_ptr<C_CUBE_Class>,
    std::shared_ptr<Callable>, 
    std::shared_ptr<C_CUBE_Module>
    std::shared_ptr<C_CUBE_List>,
    std::shared_ptr<C_CUBE_Dict>,
    // ... diğer tipler (List, Dict, vb.)
    ModulePtr // Yeni eklenen modül tipi!
>;

// ValuePtr tanımınız da bu ValueType'ı kullanmalı.
using ValuePtr = std::shared_ptr<ValueType>;

class Value {
public:
    ValueType data;

    // Constructor'lar
    Value() : data(std::monostate{}) {} // Varsayılan olarak 'none'
    Value(bool val) : data(val) {}
    Value(double val) : data(val) {}
    Value(const std::string& val) : data(val) {}
    Value(std::string&& val) : data(std::move(val)) {}
    // shared_ptr tabanlı tipler için constructor'lar
    Value(std::shared_ptr<C_CUBE_Object> obj) : data(obj) {}
    Value(std::shared_ptr<C_CUBE_Function> func) : data(func) {}
    Value(std::shared_ptr<C_CUBE_Class> cls) : data(cls) {}
    Value(std::shared_ptr<Callable> callable) : data(callable) {}
    Value(ModulePtr module) : data(module) {} // Yeni ModulePtr constructor'ı

    // GcObject'ten türemiş bir shared_ptr'ı güvenli bir şekilde GcPtr olarak döndürür.
    // Çöp toplayıcı tarafından çağrılacaktır.
    GcPtr getGcObject() const;

    // Type checking helpers (isteğe bağlı, Utils.h'de de olabilir)
    bool isNone() const { return std::holds_alternative<std::monostate>(data); }
    bool isBool() const { return std::holds_alternative<bool>(data); }
    bool isNumber() const { return std::holds_alternative<double>(data); }
    bool isString() const { return std::holds_alternative<std::string>(data); }
    bool isObject() const { return std::holds_alternative<std::shared_ptr<C_CUBE_Object>>(data); }
    bool isFunction() const { return std::holds_alternative<std::shared_ptr<C_CUBE_Function>>(data); }
    bool isClass() const { return std::holds_alternative<std::shared_ptr<C_CUBE_Class>>(data); }
    bool isCallable() const { return std::holds_alternative<std::shared_ptr<Callable>>(data); }
    bool isModule() const { return std::holds_alternative<ModulePtr>(data); }


    // Type casting helpers (std::get kullanarak)
    template<typename T>
    const T& as() const { return std::get<T>(data); }

    template<typename T>
    T& as() { return std::get<T>(data); }
};

// Yorumlayıcı içinde Value nesnelerine işaret etmek için kullanılacak shared_ptr
using ValuePtr = std::shared_ptr<Value>;

#endif // C_CUBE_VALUE_H
