#ifndef CUBE_MEMORY_MANAGEMENT_GC_HPP
#define CUBE_MEMORY_MANAGEMENT_GC_HPP

#include <vector>
#include <string>
#include <memory>
#include <unordered_set> // Kökleri saklamak için
#include <cstddef> // size_t için
#include <cstdint> // uint8_t için

#include "Data Types/object.hpp"        // Object türü için (GC onunla çalışır)
#include "Data Types/type.hpp"          // Type bilgisi için (Object oluştururken)
#include "ErrorHandling/error_reporter.hpp" // Hata raporlama için


namespace CCube {
namespace MemoryManagement {

// C-CUBE Çalışma Zamanı Çöp Toplayıcısı (Basit Mark-and-Sweep Örneği)
// Bu bir Singleton olacaktır.
class GarbageCollector {
public:
    // Singleton instance'ına erişim metodu
    static GarbageCollector& getInstance();

    // Singleton kopyalanamaz ve atama yapılamaz
    GarbageCollector(const GarbageCollector&) = delete;
    GarbageCollector& operator=(const GarbageCollector&) = delete;

    // Heap'ten yeni bir C-CUBE Object'i için bellek ayırır.
    // Bu, C++ new operatörünün yerine kullanılır.
    // size: Nesne için toplam byte boyutu (Object başlığı dahil)
    // type: Nesnenin çalışma zamanı Type'ı
    // Döndürülen Object* GC tarafından yönetilir.
    Object* allocate(size_t size, Type* type, SourceLocation loc);

    // Çöp toplama döngüsünü tetikler.
    void collect();

    // GC kökü olarak bir pointer'ın adresini kaydeder.
    // GC, bu pointerlardan ulaşılabilen tüm objeleri canlı kabul eder.
    // 'root' parametresi, Object* pointer'ının ADRESİDİR (Object**).
    // Çünkü root pointer'ın değeri (yani işaret ettiği Object) çalışma zamanında değişebilir.
    void addRoot(Object** root);

    // Daha önce eklenmiş bir GC kökünü kaldırır.
    void removeRoot(Object** root);

    // Bir Object'i GC işaretleme aşamasında ziyaret eder (GC'nin dahili kullanımı için).
    // Object::visitPointers metodunda kullanılır.
    void markObject(Object* obj);


private:
    // Özel kurucu (Singleton)
    GarbageCollector(ErrorReporter& reporter);

    ErrorReporter& reporter_;

    // --- Heap Yönetimi (Basit Yığın Alanı) ---
    std::vector<uint8_t> heap_; // Tüm GC'li nesnelerin bulunduğu bellek alanı
    size_t heap_size_ = 1024 * 1024; // Varsayılan heap boyutu (1MB)
    // TODO: Serbest bellek yönetimi için Free List veya başka bir yapı

    // --- GC Durumu ---
    // Kökler listesi (çalışma zamanında erişilebilir Object** pointerları)
     std::unordered_set<Object**> roots_; // Set arama O(logN) veya O(1)
    // Alternatif: vector, daha basit ama remove yavaş olabilir.
    std::vector<Object**> roots_;


    // --- Mark-and-Sweep Aşama Metotları ---
    void mark_phase(); // İşaretleme aşamasını başlatır
    // markObject(Object* obj) yukarıda public arayüzde tanımlandı, Object::visitPointers onu çağırır.
    void sweep_phase(); // Süpürme aşamasını başlatır

    // TODO: Ayrılmış objeleri takip etmek için bir liste veya başka bir yapı
    // Heap üzerinde tüm objeleri nasıl gezileceği GC implementasyonuna bağlıdır.
    // Örneğin: Tüm objelerin bir linked list'ini tutmak veya heap'i taramak.
    // Basitlik için, heap'in başından sonuna objeleri gezdiğimizi varsayalım.
};

} namespace MemoryManagement
} namespace CCube

#endif // CUBE_MEMORY_MANAGEMENT_GC_HPP