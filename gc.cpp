#include "gc.h"
#include "interpreter.h"     // Interpreter'ın ortamına, call stack'ine erişim için
#include "environment.h"     // Ortam nesneleri için
#include "value.h"           // Value'ların GcObject'lere işaretçi tuttuğu varsayılır
#include "object.h"          // C_CUBE_Object (GcObject'ten türemişse)
#include "function.h"        // C_CUBE_Function (GcObject'ten türemişse)
#include "class.h"           // C_CUBE_Class (GcObject'ten türemişse)
#include "c_cube_module.h"   // C_CUBE_Module (GcObject'ten türemişse)

#include <iostream>
#include <algorithm>         // std::remove_if için

// GarbageCollector Constructor
GarbageCollector::GarbageCollector(Interpreter& interpreter)
    : interpreter(interpreter) {
    // Başlangıç eşiği ayarlanabilir
}

// Yeni bir GcObject'i tahsis eder ve çöp toplayıcıya kaydeder
template<typename T, typename... Args>
std::shared_ptr<T> GarbageCollector::allocate(Args&&... args) {
    static_assert(std::is_base_of<GcObject, T>::value, "T must derive from GcObject");

    std::shared_ptr<T> obj = std::make_shared<T>(std::forward<Args>(args)...);
    heap.push_back(obj); // Tüm tahsis edilmiş nesneleri takip et

    // İsteğe bağlı: Tahsis edilen belleği izle ve eşiğe ulaşırsa GC'yi tetikle
     currentAllocatedBytes += sizeof(T); // Basit bir tahsis izleme (objenin gerçek boyutu değildir)
     if (currentAllocatedBytes >= allocationThreshold) {
         collectGarbage();
     }

    return obj;
}

// Bir GcObject'i işaretlemek ve kuyruğa eklemek için dışarıdan (markChildren'dan) çağrılır.
void GarbageCollector::enqueueForMarking(GcPtr obj) {
    if (obj == nullptr || obj->marked) {
        return; // Nesne zaten nullptr veya işaretlenmişse işlemi atla
    }
    obj->marked = true;           // Nesneyi işaretle
    markedObjects.insert(obj);    // İşaretli nesneler kümesine ekle
    markQueue.push_back(obj);     // Kuyruğa ekle, böylece çocukları da işlenecek
}

// İşaretleme aşaması
void GarbageCollector::mark() {
    markedObjects.clear(); // Her döngüde temizle
    markQueue.clear();     // Kuyruğu temizle

    // --- Kökleri İşaretle ---
    // 1. Interpreter'ın global ortamındaki tüm değerler
    // (Environment::forEachValue metodu olduğunu varsayıyoruz)
    interpreter.globals->forEachValue([this](ValuePtr val) {
        // ValuePtr'ın içinde GcObject var mı kontrol et
        if (auto gcObj = val->getGcObject()) { // Value sınıfında getGcObject() metodu olmalı
            enqueueForMarking(gcObj);
        }
    });

    // 2. Interpreter'ın mevcut ortam zincirindeki tüm değerler
    EnvironmentPtr currentEnv = interpreter.environment;
    while (currentEnv) {
        // Not: Eğer Environment'ın kendisi bir GcObject ise, onu da işaretlemelisiniz.
         if (auto envAsGc = std::dynamic_pointer_cast<GcObject>(currentEnv)) {
             enqueueForMarking(envAsGc);
         }
        currentEnv->forEachValue([this](ValuePtr val) {
            if (auto gcObj = val->getGcObject()) {
                enqueueForMarking(gcObj);
            }
        });
        currentEnv = currentEnv->enclosing; // Üst ortama geç
    }

    // 3. Çağrı yığınındaki (call stack) değerler
    // (Interpreter sınıfında getCallStackValues veya benzeri bir metot olduğunu varsayıyoruz)
    // For example:
    
    for (const auto& frameValue : interpreter.getCallStackValues()) {
        if (auto gcObj = frameValue->getGcObject()) {
            enqueueForMarking(gcObj);
        }
    }
    
    // veya doğrudan Interpreter'dan stack frame'lerine erişiliyorsa
     for (const auto& frame : interpreter.callStack) { // CallStack üyesi varsa
         frame.environment->forEachValue([this](ValuePtr val) {
             if (auto gcObj = val->getGcObject()) { enqueueForMarking(gcObj); }
         });
    //     // Frame'deki diğer geçici değerler veya değişkenler doğrudan ValuePtr ise
          frame.getTemporaryValues().forEach([this](ValuePtr val) { ... });
     }

    // --- İşaretlenmiş her canlı nesnenin çocuklarını rekürsif olarak işaretle (BFS) ---
    size_t i = 0;
    while (i < markQueue.size()) {
        GcPtr current = markQueue[i++];
         current->marked zaten true. Şimdi çocuklarını işaretle.
        current->markChildren(*this); // Her GcObject kendi çocuklarını GC'ye bildirecek
    }
}

// Süpürme aşaması
void GarbageCollector::sweep() {
    // İşaretlenmemiş nesneleri 'heap' listesinden kaldır.
    heap.erase(std::remove_if(heap.begin(), heap.end(),
                              [this](const GcPtr& obj) {
                                  if (!obj->marked) {
                                      // Nesne işaretlenmemiş, yani ölü. Bellekten temizlenecek.
                                       currentAllocatedBytes -= sizeOf(obj); // Bellek takibi için (sizeof(GcObject) doğru değil)
                                      return true; // Kaldırılacak
                                  } else {
                                      // Nesne canlı, işaretini sıfırla bir sonraki döngü için
                                      obj->marked = false;
                                      return false; // Bırakılacak
                                  }
                              }),
               heap.end());
}

// Çöp toplama döngüsünü tetikler.
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

// Bellek serbest bırakma bildirimi (shared_ptr otomatik yapar, ama manuel izleme için)
void GarbageCollector::notifyDeallocation(size_t bytes) {
     currentAllocatedBytes -= bytes; // shared_ptr kullandığımızdan bu genellikle gerekmez
}

// Explicit template instantiation for allocate if needed elsewhere
template std::shared_ptr<C_CUBE_Object> GarbageCollector::allocate<C_CUBE_Object>(std::shared_ptr<C_CUBE_Class>);
template std::shared_ptr<C_CUBE_Function> GarbageCollector::allocate<C_CUBE_Function>(const std::vector<Token>&, StmtPtr, EnvironmentPtr);
template std::shared_ptr<C_CUBE_Module> GarbageCollector::allocate<C_CUBE_Module>(const std::string&, std::vector<StmtPtr>, EnvironmentPtr);
