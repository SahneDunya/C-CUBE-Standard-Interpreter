#ifndef C_CUBE_OBJECT_H
#define C_CUBE_OBJECT_H

#include "value.h" // ValuePtr için
#include "token.h" // Hata mesajları için Token
#include <unordered_map> // Alanları saklamak için map
#include <string>
#include <memory> // std::shared_ptr için
#include "gc.h"

// İleri bildirim: Object sınıfı Class sınıfını referans alacak.
// Dairesel bağımlılığı önlemek için Class sınıfının tam tanımına şimdilik ihtiyacımız yok.
class C_CUBE_Class;

// C_CUBE_Class nesnesine akıllı işaretçi alias'ı (Class sınıfı tanımlandığında kullanılacak)
using C_CUBE_ClassPtr = std::shared_ptr<C_CUBE_Class>;


// Kullanıcı tanımlı C-CUBE sınıfının bir örneğini (instance) temsil eden sınıf
class C_CUBE_Object : public GcObject {
    // ...
    void markChildren(GarbageCollector& gc) override { /* ... */ }
};

private:
    // Bu nesnenin ait olduğu sınıfın referansı
    C_CUBE_ClassPtr klass;

    // Nesnenin alanlarını (properties) saklayan map
    // Key: Alan adı (string), Value: Alan değeri (ValuePtr)
    std::unordered_map<std::string, ValuePtr> fields;

public:
    // Constructor: Nesneyi oluştururken ait olduğu sınıfı alır.
    C_CUBE_Object(C_CUBE_ClassPtr klass);

    // Bir alanın değerini döndürür. Eğer alan yoksa, metodunu arar.
    // Eğer ne alan ne de metot bulunursa runtime hatası verir.
    // name: Erişilmek istenen alanın/metodun adı (Token).
    ValuePtr get(const Token& name);

    // Bir alanın değerini ayarlar veya yeni bir alan ekler.
    // name: Ayarlanacak alanın adı (Token).
    // value: Alanın yeni değeri (ValuePtr).
    void set(const Token& name, ValuePtr value);

    // Nesnenin string temsilini döndürür (örneğin, "<instance of SınıfAdı>").
    std::string toString() const;
};

// C_CUBE_Object nesnesine akıllı işaretçi alias'ı
using C_CUBE_ObjectPtr = std::shared_ptr<C_CUBE_Object>;

#endif // C_CUBE_OBJECT_H
