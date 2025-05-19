#ifndef C_CUBE_GC_H
#define C_CUBE_GC_H

#include <vector>
#include <memory>
#include <unordered_set> // İşaretli nesneleri tutmak için

// İleri bildirimler
class Environment;
class Interpreter;

// GcObject: Çöp toplayıcı tarafından yönetilecek temel sınıf
class GcObject {
public:
    // Bu nesnenin erişilebilir olup olmadığını belirten işaret
    bool marked = false;

    // Sanal yıkıcı: Türemiş sınıfların doğru şekilde temizlenebilmesi için önemlidir.
    virtual ~GcObject() = default;

    // Sanal işaretleme metodu: Bu nesnenin erişilebilir çocuklarını işaretler.
    // Her türemiş sınıf kendi çocuklarını (başka GcObject'lere olan referansları) işaretlemelidir.
    virtual void markChildren() = 0;
};

// GcPtr: GcObject'lere işaretçi alias'ı
// Çöp toplayıcı sistemi kullanılıyorsa, ValuePtr'lar GcPtr'ları tutar.
using GcPtr = std::shared_ptr<GcObject>;

// GarbageCollector: Çöp toplayıcının kendisi
class GarbageCollector {
private:
    // Tüm tahsis edilmiş GcObject'lerin listesi
    // Bu listeyi çöp toplayıcı "köklerden" başlatır ve işaretler.
    std::vector<GcPtr> heap;

    // İşaretleme aşamasında erişilebilir (canlı) olduğu bulunan nesnelerin kümesi
    std::unordered_set<GcPtr> markedObjects;

    // Çöp toplayıcıyı tetiklemek için eşik değeri
    size_t allocationThreshold = 1024 * 1024; // Örneğin 1MB
    size_t currentAllocatedBytes = 0; // Mevcut tahsis edilen bellek

    // Yorumlayıcıya referans (global ortamları, call stack'i vb. kök olarak kullanmak için)
    Interpreter& interpreter;

    // İşaretleme aşamasını gerçekleştirir
    void mark();

    // Süpürme aşamasını gerçekleştirir
    void sweep();

public:
    // Constructor: Çöp toplayıcının hangi yorumlayıcıyla çalışacağını belirtir.
    GarbageCollector(Interpreter& interpreter);

    // Yeni bir GcObject'i yönetmek için kaydeder.
    // T tipi GcObject'ten türemelidir.
    template<typename T, typename... Args>
    std::shared_ptr<T> allocate(Args&&... args);

    // Çöp toplama döngüsünü tetikler.
    void collectGarbage();

    // Ayrılan belleği izlemek için kullanılır (isteğe bağlı)
    void notifyAllocation(size_t bytes);
    void notifyDeallocation(size_t bytes);
};

// C_CUBE_Function, C_CUBE_Object, C_CUBE_Class gibi tüm çalışma zamanı nesneleriniz
// GcObject'ten türemeli ve markChildren metodunu implement etmelidir.
// Örneğin:
/*
// function.h içinde:
class C_CUBE_Function : public Callable, public GcObject {
    // ...
    void markChildren() override {
        // Fonksiyonun gövdesindeki AST düğümlerini işaretle (eğer onlar da GcObject ise)
        // Eğer closure bir GcObject ise, onu da işaretle
        // Örneğin: body->markChildren(); // Eğer StmtPtr de GcObject'lere referans tutuyorsa
         if (closure) closure->markChildren(); // Ortamlar da GcObject olabilir
    }
    // ...
};


#endif // C_CUBE_GC_H
