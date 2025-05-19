#include "gc.h"
#include "interpreter.h" // Interpreter'ın ortamına, call stack'ine erişim için
#include "environment.h" // Ortam nesneleri için
#include "value.h"       // Value'ların GcObject'lere işaretçi tuttuğu varsayılır
#include "object.h"      // C_CUBE_Object (GcObject'ten türemişse)
#include "function.h"    // C_CUBE_Function (GcObject'ten türemişse)
#include "class.h"       // C_CUBE_Class (GcObject'ten türemişse)

#include <iostream>
#include <algorithm> // std::remove_if için

// GarbageCollector Constructor
GarbageCollector::GarbageCollector(Interpreter& interpreter)
    : interpreter(interpreter) {
    // Başlangıç eşiği ayarlanabilir
}

// Yeni bir GcObject'i tahsis eder ve çöp toplayıcıya kaydeder
template<typename T, typename... Args>
std::shared_ptr<T> GarbageCollector::allocate(Args&&... args) {
    // T'nin GcObject'ten türediğini derleme zamanında kontrol et
    static_assert(std::is_base_of<GcObject, T>::value, "T must derive from GcObject");

    std::shared_ptr<T> obj = std::make_shared<T>(std::forward<Args>(args)...);
    heap.push_back(obj); // Tüm tahsis edilmiş nesneleri takip et

    // İsteğe bağlı: Tahsis edilen belleği izle ve eşiğe ulaşırsa GC'yi tetikle
     currentAllocatedBytes += sizeof(T); // Basit bir tahsis izleme
     if (currentAllocatedBytes >= allocationThreshold) {
         collectGarbage();
     }

    return obj;
}

// İşaretleme aşaması
void GarbageCollector::mark() {
    markedObjects.clear(); // Her döngüde temizle

    // Kökleri işaretle:
    // 1. Interpreter'ın global ortamındaki tüm değerler
    // 2. Interpreter'ın mevcut ortamlarındaki tüm değerler
    // 3. Çağrı yığınındaki (call stack) tüm değerler
    // 4. Built-in fonksiyonlar (eğer onlar da GcObject ise)

    // Örnek: Global ortamdaki değerleri işaretleme
    // Environment sınıfında bir 'getValues()' veya 'forEachValue()' metodu olmalı
     interpreter.globals->markValues(markedObjects);

    // Mevcut ortam zincirindeki değerleri işaretle
     EnvironmentPtr currentEnv = interpreter.environment;
     while (currentEnv) {
         currentEnv->markValues(markedObjects);
         currentEnv = currentEnv->enclosing; // Üst ortama geç
     }

    // Çağrı yığınındaki (call stack) değerleri işaretle
    // Interpreter'ın call stack'ine erişim olmalı
     for (const auto& frame : interpreter.callStack) {
         frame.environment->markValues(markedObjects);
    //     // Frame'deki diğer geçici değerler veya değişkenler
     }

    // İşaretlenmiş her canlı nesnenin çocuklarını rekürsif olarak işaretle
    std::vector<GcPtr> toMark;
    for (const auto& obj : markedObjects) {
        toMark.push_back(obj); // Kökleri başlangıç için toMark listesine ekle
    }

    size_t i = 0;
    while (i < toMark.size()) {
        GcPtr current = toMark[i++];
        if (!current->marked) { // Zaten işaretlenmemişse
            current->marked = true;
            markedObjects.insert(current);
            // Çocuklarını işaretle (bu, çocukları toMark listesine ekler)
             current->markChildren(); // Bu metodun çocukları markedObjects'e eklemesi gerekiyor
                                    // Veya markChildren metodu toMark listesine eklemeli
            // Basit bir yaklaşım: markChildren() metodu, çocuk GcObject'leri döndürsün
            // veya GarbageCollector'ın işaretleme metodunu doğrudan çağırsın
        }
    }
    // NOTE: `markChildren` metodu `this` objesinin içinde diğer `GcObject`'leri bulup onların
    // `marked` flag'ini `true` yapmalı ve `markedObjects` kümesine eklemelidir.
    // Bu, `Value` nesnelerinizin `GcObject`'lere işaretçi tuttuğunu varsayar.
    // Example: value.h içerisinde `GcObject`'e işaretçi tutan bir `ValuePtr` tipi var.
    // C_CUBE_Object::markChildren -> fields'daki ValuePtr'ları işaretle
    // C_CUBE_Function::markChildren -> closure ortamı, gövde AST'sindeki GcObject'ler
    // C_CUBE_Class::markChildren -> superclass, metotlardaki C_CUBE_Function'ları işaretle
    // Environment::markChildren -> değişken map'indeki ValuePtr'ları işaretle
}

// Süpürme aşaması
void GarbageCollector::sweep() {
    // İşaretlenmemiş nesneleri 'heap' listesinden kaldır.
     std::remove_if, koşulu sağlayan elementleri sona taşır ve iterator döndürür.
    // Ardından erase ile bu elementler silinir.
    heap.erase(std::remove_if(heap.begin(), heap.end(),
                              [this](const GcPtr& obj) {
                                  if (!obj->marked) {
                                      // Nesne işaretlenmemiş, yani ölü. Bellekten temizlenecek.
                                       std::cout << "Debug: Sweeping object: " << obj->toString() << std::endl; // Eğer toString varsa
                                       currentAllocatedBytes -= sizeOf(obj); // Bellek takibi için
                                      return true; // Kaldırılacak
                                  } else {
                                      // Nesne canlı, işaretini sıfırla bir sonraki döngü için
                                      obj->marked = false;
                                      return false; // Bırakılacak
                                  }
                              }),
               heap.end());
}

// Çöp toplama döngüsünü tetikler
void GarbageCollector::collectGarbage() {
    std::cout << "--- Starting garbage collection ---" << std::endl;
    size_t pre_collection_size = heap.size();

    mark();  // Canlı nesneleri işaretle
    sweep(); // İşaretlenmemiş (ölü) nesneleri temizle

    size_t post_collection_size = heap.size();
    std::cout << "--- Garbage collection finished ---" << std::endl;
    std::cout << "Objects before GC: " << pre_collection_size
              << ", Objects after GC: " << post_collection_size
              << ", Collected: " << (pre_collection_size - post_collection_size) << std::endl;

    // Eşiği dinamik olarak ayarla (isteğe bağlı)
     allocationThreshold = currentAllocatedBytes * 2; // Canlı nesnelerin iki katı kadar bellek ayır
}

// Bellek tahsisi bildirimi (isteğe bağlı)
void GarbageCollector::notifyAllocation(size_t bytes) {
    currentAllocatedBytes += bytes;
    if (currentAllocatedBytes >= allocationThreshold) {
        collectGarbage();
    }
}

// Bellek serbest bırakma bildirimi (isteğe bağlı, shared_ptr otomatik yapar)
void GarbageCollector::notifyDeallocation(size_t bytes) {
    currentAllocatedBytes -= bytes;
}

// Explicit template instantiation for common types (or define allocate in header)
// Eğer allocate metodunu sadece .cpp dosyasında tutmak istiyorsanız,
// allocate metodunun kullanılacağı tipler için explicit instantiation yapmanız gerekir.
// Örneğin:
 template std::shared_ptr<C_CUBE_Object> GarbageCollector::allocate<C_CUBE_Object>(C_CUBE_ClassPtr);
 template std::shared_ptr<C_CUBE_Function> GarbageCollector::allocate<C_CUBE_Function>(const std::vector<Token>&, StmtPtr, EnvironmentPtr);
// Veya daha kolay olan, allocate metodunun tanımını doğrudan gc.h içine taşımaktır.
// Genellikle template metodlar başlık dosyasında tanımlanır.
