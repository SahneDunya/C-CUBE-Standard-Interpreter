#ifndef C_CUBE_ENVIRONMENT_H
#define C_CUBE_ENVIRONMENT_H

#include <string>
#include <unordered_map>
#include <memory> // std::shared_ptr
#include <functional> // std::function için

#include "value.h" // ValuePtr tanımı için
#include "gc.h"    // GcObject tanımı için

// İleri bildirim: Interpreter sınıfı Environment'ı kullanabilir
class Interpreter;

// Environment: Değişkenleri ve onların değerlerini tutan bir ortam
class Environment : public GcObject { // Environment'ı GcObject'ten türetiyoruz!
public:
    // Bu ortamın üst ortamı. Kapanışları (closures) ve kapsam zincirini yönetir.
    // Bu shared_ptr olduğu için GcObject'ten türemişse GC tarafından izlenir.
    EnvironmentPtr enclosing; // EnvironmentPtr = std::shared_ptr<Environment> olmalıydı.
                              // EnvironmentPtr'ı aşağıda Value.h'den sonra tanımlayalım
                              // Ya da direkt shared_ptr<Environment> kullanalım şimdilik.
    std::shared_ptr<Environment> _enclosing; // Geçici olarak GcObject olarak tanımlanana kadar

    // Değişkenlerin adlarını ve değerlerini tutan harita
    std::unordered_map<std::string, ValuePtr> values;

    // Constructor: Üst ortamı alır. Global ortam için nullptr olabilir.
    Environment(std::shared_ptr<Environment> _enclosing = nullptr);

    // Bir değişken tanımlar veya günceller.
    void define(const std::string& name, ValuePtr value);

    // Bir değişkenin değerini alır. Bulamazsa hata fırlatır.
    ValuePtr get(const Token& name);

    // Bir değişkenin değerini atar. Bulamazsa hata fırlatır.
    void assign(const Token& name, ValuePtr value);

    // GcObject arayüzü: Bu ortamın çocuklarını işaretle.
    void markChildren(GarbageCollector& gc) override;

    // Ortamdaki her değeri bir lambda ile işlemek için yardımcı metot.
    // Çöp toplayıcı tarafından kökleri taramak için kullanılır.
    void forEachValue(std::function<void(ValuePtr)> callback) const;
};

// Ortam pointer'ı için alias (kendi kendisine referans vermemek için)
// Eğer Environment kendi başına bir GcObject ise, bu EnvironmentPtr da GcObject olabilir.
// Ancak döngüsel bağımlılıktan kaçınmak için şimdilik ayrı bir alias kullanmak daha güvenli.
// Yorumlayıcıda GcPtr yerine direkt std::shared_ptr<Environment> kullanmak daha doğru olur.
// O yüzden yukarıdaki GcObject için de _enclosing kullandık.
// Ortamların kendisi GcObject olduğundan, bu shared_ptr'lar da GC tarafından izlenir.
// Eğer EnvironmentPtr olarak tanımlayacaksak:
 using EnvironmentPtr = std::shared_ptr<Environment>;
// (Value.h'deki ModulePtr tanımına benzer şekilde.)

#endif // C_CUBE_ENVIRONMENT_H
