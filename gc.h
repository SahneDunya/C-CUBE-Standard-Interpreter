#ifndef C_CUBE_GC_H
#define C_CUBE_GC_H

#include <vector>
#include <memory>
#include <unordered_set> // İşaretli nesneleri tutmak için
#include <deque>         // İşaretleme kuyruğu için (BFS)

// İleri bildirimler
class Environment;
class Interpreter;
class Value; // Value sınıfına ihtiyaç duyulacak (ValuePtr'dan GcPtr'ı çekmek için)

// GcObject: Çöp toplayıcı tarafından yönetilecek temel sınıf
class GcObject {
public:
    // Bu nesnenin erişilebilir olup olmadığını belirten işaret
    bool marked = false;

    // Sanal yıkıcı: Türemiş sınıfların doğru şekilde temizlenebilmesi için önemlidir.
    virtual ~GcObject() = default;

    // Sanal işaretleme metodu: Bu nesnenin erişilebilir çocuklarını işaretler.
    // Artık GcObject'lerin çocuklarını işaretlemek için GarbageCollector'ı kullanacaklar.
    // Her türemiş sınıf kendi çocuklarını (başka GcObject'lere olan referansları) gc.enqueueForMarking() ile kuyruğa eklemelidir.
    virtual void markChildren(GarbageCollector& gc) = 0;
};

// GcPtr: GcObject'lere işaretçi alias'ı
// Çöp toplayıcı sistemi kullanılıyorsa, ValuePtr'lar GcPtr'ları tutar.
using GcPtr = std::shared_ptr<GcObject>;

// GarbageCollector: Çöp toplayıcının kendisi
class GarbageCollector {
private:
    // Tüm tahsis edilmiş GcObject'lerin listesi (heap)
    std::vector<GcPtr> heap;

    // İşaretleme aşamasında canlı olduğu bulunan nesnelerin kümesi (tekrar işaretlemeyi önlemek için)
    std::unordered_set<GcPtr> markedObjects;

    // İşaretleme aşamasında kullanılacak BFS (Breadth-First Search) kuyruğu
    std::deque<GcPtr> markQueue;

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

    // Bir GcObject'i işaretlemek ve kuyruğa eklemek için dışarıdan (markChildren'dan) çağrılır.
    void enqueueForMarking(GcPtr obj);

    // Ayrılan belleği izlemek için kullanılır (isteğe bağlı)
    void notifyAllocation(size_t bytes);
    void notifyDeallocation(size_t bytes);
};

#endif // C_CUBE_GC_H
