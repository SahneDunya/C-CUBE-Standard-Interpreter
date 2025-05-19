#ifndef C_CUBE_CLASS_H
#define C_CUBE_CLASS_H

#include "callable.h" // Callable arayüzünü kullanıyoruz
#include "function.h" // Metotlar C_CUBE_Function nesneleridir
#include "object.h"   // Yeni nesneler C_CUBE_Object olacak

#include <unordered_map> // Metotları saklamak için map
#include <string>
#include <vector>
#include <memory> // std::shared_ptr için

// İleri bildirim: Interpreter sınıfını kullanacağımız için
class Interpreter;

// C_CUBE_Function nesnesine akıllı işaretçi alias'ı (function.h'de tanımlı)
 using C_CUBE_FunctionPtr = std::shared_ptr<C_CUBE_Function>;

// C_CUBE_Object nesnesine akıllı işaretçi alias'ı (object.h'de tanımlı)
 using C_CUBE_ObjectPtr = std::shared_ptr<C_CUBE_Object>;

// C_CUBE_Class nesnesine akıllı işaretçi alias'ı
using C_CUBE_ClassPtr = std::shared_ptr<C_CUBE_Class>;


// Kullanıcı tanımlı C-CUBE sınıf tanımını temsil eden sınıf
class C_CUBE_Class : public Callable, public std::enable_shared_from_this<C_CUBE_Class> {
     std::enable_shared_from_this, 'this' class objesine
    // shared_ptr almak için kullanılır (nesne oluştururken nesneye sınıf referansı vermek için).
private:
    // Sınıfın adı
    std::string name;

    // Üst sınıf (miras alınıyorsa), yoksa nullptr
    C_CUBE_ClassPtr superclass;

    // Sınıfa ait metotlar (isimlerine göre maplenmiş)
    std::unordered_map<std::string, C_CUBE_FunctionPtr> methods;

public:
    // Constructor:
    // name: Sınıfın adı.
    // superclass: Miras alınan sınıfın referansı (varsa).
    // methods: Sınıfın metotlarını içeren map.
    C_CUBE_Class(std::string name, C_CUBE_ClassPtr superclass,
                 std::unordered_map<std::string, C_CUBE_FunctionPtr> methods)
        : name(std::move(name)), superclass(superclass), methods(std::move(methods)) {}

    // Metot arama: Belirtilen isimde bir metodu bu sınıfta veya üst sınıflarda arar.
    // name: Aranacak metodun adı.
    // Bulursa C_CUBE_FunctionPtr, bulamazsa nullptr döndürür.
    C_CUBE_FunctionPtr findMethod(const std::string& name);

    // Sınıfın adını döndürür.
    const std::string& getName() const { return name; }


    // Callable arayüz metodlarının implementasyonu

    // Sınıf, yapıcı metodun (init) arity'si ile çağrılabilir.
    int arity() const override;

    // Sınıfı çağırır (yeni bir nesne oluşturur).
    ValuePtr call(Interpreter& interpreter, const std::vector<ValuePtr>& arguments) override;

    // Sınıfın string temsilini döndürür.
    std::string toString() const override {
        return "<class " + name + ">";
    }
};

#endif // C_CUBE_CLASS_H
