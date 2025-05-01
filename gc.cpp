#include "gc.hpp"
#include <iostream>
#include <algorithm> // std::remove, std::find
#include <cstdlib> // malloc, free veya sbrk gibi sistem çağrıları için (gerçek heap yönetimi)
#include <cstring> // memcpy için

// TODO: Çalışma zamanı somut Object tiplerini dahil etmeniz GEREKİYOR.
// visitPointers metotları bu tiplerde implemente edilmelidir.
// Ayrıca allocate metodu için bu tiplerin boyutlarına (sizeof) ihtiyacınız olacak.
#include "Data Types/IntegerObject.hpp"
#include "Data Types/StringObject.hpp"
// ... diğerleri ...


namespace CCube {
namespace MemoryManagement {

// Singleton instance'ını döndüren metot
GarbageCollector& GarbageCollector::getInstance() {
    // ErrorReporter'a erişim için static bir instance lazım
    // Global veya Compiler'da oluşturulmuş bir ErrorReporter* pointer'ını burada kullanabiliriz.
    // Veya GC ErrorReporter* pointerını constructor'da alır ve bu static fonksiyonda ilk kez o pointer'ı kullanır.
    // Basitlik için, ErrorReporter'ın zaten bir başka singleton veya global olarak erişilebilir bir yerde olduğunu varsayalım.
    // Veya... GC constructor'ı ErrorReporter alır, getInstance ilk çağrıda ErrorReporter'ı bir yerden (pluggable) alır.
    // En temiz yol, Compiler'ın GC'yi oluşturup yönetmesi ve referansını geçmesidir, ama GC genellikle singleton tasarlanır.
    // Basit Singleton yaklaşımıyla devam edelim ve ErrorReporter'a erişim varsayalım.

    // Geçici çözüm: ErrorReporter'ın singleton olduğunu varsayalım veya global bir instance'a erişelim.
    // Eğer ErrorReporter singleton ise:
     static GarbageCollector instance(ErrorHandling::ErrorReporter::getInstance());

    // Veya basit bir global ErrorReporter pointer'ı kullan (iyi uygulama değil):
     extern ErrorHandling::ErrorReporter* globalErrorReporter; // Başka bir yerde tanımlanmalı
     static GarbageCollector instance(*globalErrorReporter);

    // Veya GC'nin constructor'ı çağrılırken ErrorReporter referansı ilk kez verilir.
     static GarbageCollector instance(Compiler::getGlobalErrorReporter()); // Compiler'da böyle bir getter olduğunu varsayalım.

    // En basit (ama dikkatli kullanılması gereken): Geçici bir ErrorReporter nesnesi oluşturmak (iyi değil) veya nullptr geçmek.
    // Gerçek implementasyonda, ErrorReporter'ın singleton olması veya GC'nin ömrünün Compiler'a bağlı olması ve referansın geçilmesi gerekir.

    // Basitlik için, ilk çağrıda ErrorReporter'ın bir şekilde erişilebilir olduğunu varsayalım.
    // Örneğin, main/Compiler'dan GC'yi başlatırken ErrorReporter'ı singleton'a set etmek gibi.
    static ErrorReporter* reporter_instance = nullptr; // Main/Compiler set edecek
    if (!reporter_instance) {
         // Hata: GC başlatılmadan çağrıldı veya ErrorReporter set edilmedi.
         std::cerr << "FATAL ERROR: GarbageCollector called before ErrorReporter is set or GC initialized!" << std::endl;
         // Acil durum reporterı oluştur veya direkt stderr'a yaz
         static ErrorReporter fallback_reporter;
         reporter_instance = &fallback_reporter;
          throw std::runtime_error("GarbageCollector called before initialization.");
    }

    static GarbageCollector instance(*reporter_instance); // İlk çağrıda ErrorReporter ile başlat
    return instance;
}

// Özel Kurucu
GarbageCollector::GarbageCollector(ErrorReporter& reporter)
    : reporter_(reporter) {
    // Heap alanını ayır
    heap_.resize(heap_size_);
    // TODO: Free list'i başlat (tüm heap tek büyük serbest blok olarak başlar)
    std::cout << "GarbageCollector initialized with heap size " << heap_size_ << " bytes." << std::endl; // Hata ayıklama
     heap_start_ = heap_.data();
     heap_end_ = heap_.data() + heap_size_;
     free_list_ = heap_start_; // Örnek: tek bir serbest blok
}

// Heap'ten yeni bir Object için bellek ayırır.
Object* GarbageCollector::allocate(size_t size, Type* type, SourceLocation loc) {
    // TODO: Free list'ten veya heap'i genişleterek uygun boyutta serbest bellek bul.
    // Bu basit örnekte sadece heap'in başından ayırdığımızı varsayalım (gerçekçi değil).
     size_t required_size = size; // Object başlığı + türetilmiş sınıf üyeleri boyutu

    // Basit yer tutucu ayırma: Her çağrıda yeni bellek blokları ayır (GC yapmıyor!)
    // Gerçek implementasyon Free List kullanır.
     Object* obj_memory = reinterpret_cast<Object*>(std::malloc(size)); // Heap'ten değil, sistemden ayırıyor!

    // Gerçek GC heap'inden ayırma mantığı (yer tutucu):
     Object* obj_memory = reinterpret_cast<Object*>(find_free_block_in_heap(size)); // placeholder

    // Eğer bellek ayrılamazsa hata raporla
     if (!obj_memory) {
          reporter_.reportError(loc, ErrorCode::RUNTIME_NULL_POINTER_DEREFERENCE, "Out of memory during object allocation."); // Daha çok OutOfMemory hatası
          return nullptr; // veya hata nesnesi döndür
     }

    // Ayrılan bellek alanını kullanarak Object'i yerinde oluştur (placement new)
     Object* obj = new (obj_memory) Object(type); // Object constructor çağrılır
    // Ancak burada türetilmiş sınıf oluşturulmalı (IntegerObject, StringObject vb.)

    // Gerçek implementasyonda:
     Object* obj = reinterpret_cast<Object*>(find_free_block_in_heap(size)); // Hafızayı bul
     if (!obj) { reporter_.reportError(loc, ErrorCode::RUNTIME_NULL_POINTER_DEREFERENCE, "Out of memory."); return nullptr; }
    // Construct the derived object in this memory (requires knowing which derived type to construct, usually passed by compiler)
    // Example for IntegerObject: IntegerObject* int_obj = new (obj) IntegerObject(value, type);
    // This 'allocate' method signature needs to be more complex to know WHICH derived type and its constructor parameters.

    // Alternatif: allocate metodu sadece ham bellek ayırır, Compiler/Runtime ayırdığı belleğe objeyi yerleştirir.
     void* raw_memory = find_free_block_in_heap(size); return raw_memory;
    // Ve Compiler kodu üretir: raw_mem = GC.allocate(size); obj_ptr = new (raw_mem) IntegerObject(value, type);

    // En basit, ama GC için iyi değil: Sadece new kullanıp GC'ye bildirmek.
     Object* obj = new Object(type); // Türetilmiş sınıf nesnesi olmalıydı
     add_allocated_object_to_list(obj); // GC'nin takip listesine ekle

    // Şimdilik, sadece bir yer tutucu mesaj ve nullptr döndürelim.
     reporter_.reportWarning(loc, ErrorCode::INTERNAL_ERROR, "GC allocate is a placeholder and returns nullptr.");
     return nullptr;
}

// Çöp toplama döngüsünü tetikler.
void GarbageCollector::collect() {
    std::cout << "Starting GC collection..." << std::endl; // Hata ayıklama

    // 1. İşaretleme aşaması
    mark_phase();

    // 2. Süpürme aşaması
    sweep_phase();

    std::cout << "GC collection finished." << std::endl; // Hata ayıklama
}

// GC kökü ekler
void GarbageCollector::addRoot(Object** root) {
    if (root) {
         roots_.insert(root); // unordered_set için
        // Vector için: Var mı diye kontrol etmeden ekleyelim basitlik için
        roots_.push_back(root);
        std::cout << "GC root added." << std::endl; // Hata ayıklama
    }
}

// GC kökü kaldırır
void GarbageCollector::removeRoot(Object** root) {
    roots_.erase(root); // unordered_set için

    // Vector için: Elemanı bul ve sil
    auto it = std::find(roots_.begin(), roots_.end(), root);
    if (it != roots_.end()) {
        roots_.erase(it);
        std::cout << "GC root removed." << std::endl; // Hata ayıklama
    }
}

// Bir Object'i GC işaretleme aşamasında ziyaret eder.
// mark_phase ve Object::visitPointers tarafından çağrılır.
void GarbageCollector::markObject(Object* obj) {
    // Null pointer veya zaten işaretlenmişse bir şey yapma
    if (!obj || obj->gc_marked_) {
        return;
    }

    // Objeyi işaretle
    obj->gc_marked_ = true;
    std::cout << "Marked object at " << obj << " (Type: " << (obj->getRuntimeType() ? obj->getRuntimeType()->toString() : "null") << ")" << std::endl; // Hata ayıklama

    // Objeyi canlı tutan diğer objelere olan pointerları ziyaret et
    obj->visitPointers(*this); // Sanal çağrı ile türetilmiş sınıfa geçer
}


// --- Mark-and-Sweep Aşama Metotları Implementasyonu ---

void GarbageCollector::mark_phase() {
    std::cout << "GC Mark Phase..." << std::endl; // Hata ayıklama
    // Köklerden başlayarak ulaşılabilen tüm objeleri işaretle
    for (Object** root : roots_) {
        if (root && *root) { // Kök pointer'ı ve işaret ettiği obje geçerliyse
            markObject(*root); // Kök objeyi işaretle
        }
    }

    // TODO: Diğer root kaynakları (Stack, Registerlar) da burada işlenmelidir.
    // Compiler/Runtime, stack ve registerlardaki Object* pointerlarını GC'ye bildirmelidir.
}

void GarbageCollector::sweep_phase() {
    std::cout << "GC Sweep Phase..." << std::endl; // Hata ayıklama

    // TODO: Heap alanındaki tüm ayrılmış objeleri gez (nasıl gezileceği heap yönetimine bağlı)
    // Veya ayrılmış objelerin bir listesi tutuluyorsa o liste gezilir.

    size_t swept_count = 0;
    size_t freed_bytes = 0;

    // Basit yer tutucu süpürme mantığı (gerçek heap taramıyor!)
    // Bu sadece Object* objelerin bir listesi tutulduğunu varsayıyor (listemiz yok!).
    // Gerçek implementasyonda, heap'in başından başlayıp ayrılan nesne boyutlarına göre ilerlenir.

    
    // Örnek (Eğer GC tüm ayrılan objelerin listesini tutuyorsa):
    auto& allocated_objects = get_list_of_allocated_objects(); // Placeholder

    auto it = allocated_objects.begin();
    while (it != allocated_objects.end()) {
        Object* obj = *it;
        if (obj->gc_marked_) {
            // Canlı obje, işareti kaldır ve bir sonraki iterasyona geç
            obj->gc_marked_ = false;
            ++it;
        } else {
            // Çöp obje, belleği serbest bırak ve listeden sil
            size_t obj_size = getSizeOfObject(obj); // Object tipine göre boyutu bul
             obj->~Object(); // Yıkıcıyı çağır
            free_memory_block(obj); // Belleği serbest bırak ve free list'e ekle
            freed_bytes += obj_size;
            swept_count++;
            it = allocated_objects.erase(it); // Listeden sil
            std::cout << "Swept object at " << obj << std::endl; // Hata ayıklama
        }
    }

     reporter_.reportWarning({0,0}, ErrorCode::INTERNAL_ERROR, "GC Sweep Phase is a placeholder and doesn't free memory.");

    // TODO: Süpürülen byte miktarını veya obje sayısını kaydet/raporla.
    std::cout << "GC Swept " << swept_count << " objects, freed " << freed_bytes << " bytes (placeholder)." << std::endl;
}

// TODO: Diğer GC metotları ve yardımcı fonksiyonlar implemente edilecek.
// Örneğin: find_free_block_in_heap, getSizeOfObject vb.


} namespace MemoryManagement
} namespace CCube