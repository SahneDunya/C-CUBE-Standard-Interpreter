#include "method_dispatcher.hpp"
#include <iostream>
#include <stdexcept>

// TODO: Çalışma zamanı somut Object tiplerini dahil etmeniz GEREKİYOR.
// Argüman tiplerini kontrol ederken ve Object*'lardan değerleri çıkarırken bunlara ihtiyaç duyulur.
#include "Data Types/IntegerObject.hpp"
#include "Data Types/StringObject.hpp"
#include "Data Types/BooleanObject.hpp"
#include "Data Types/NoneObject.hpp"
#include "Data Types/ListObject.hpp"
#include "Data Types/ClassInstance.hpp" // Kullanıcı tanımlı sınıf örnekleri


// TODO: Eğer metot entry point'leri C++ fonksiyon pointerları ise, bu fonksiyonların deklarasyonları burada veya başka bir başlıkta olmalı.
// Eğer IR block indexleri ise, VM veya Runtime, bu indexleri nasıl çağıracağını bilmeli.

namespace CCube {
namespace Runtime {

// Singleton instance'ını döndüren metot
MethodDispatcher& MethodDispatcher::getInstance() {
    // ErrorReporter'a erişim için Singleton GC örneğindeki gibi bir mekanizma gerekir.
    // GC singleton'ında kullanılan geçici ErrorReporter yaklaşımını burada da kullanalım.
    static ErrorReporter* reporter_instance = nullptr; // Main/Compiler set edecek
    if (!reporter_instance) {
         std::cerr << "FATAL ERROR: MethodDispatcher called before ErrorReporter is set or Dispatcher initialized!" << std::endl;
         static ErrorReporter fallback_reporter;
         reporter_instance = &fallback_reporter;
    }

    static MethodDispatcher instance(*reporter_instance);
    return instance;
}

// Özel Kurucu
MethodDispatcher::MethodDispatcher(ErrorReporter& reporter)
    : reporter_(reporter) {
    // Metot kayıt defteri (method_registry_) burada boş olarak başlar.
    // Runtime başlatılırken veya CodeGen tarafından doldurulur.
    std::cout << "MethodDispatcher created." << std::endl; // Hata ayıklama
}

// Metot bilgisini arar (placeholder)

MethodInfo* MethodDispatcher::findMethodInfo(Type* runtime_type, int method_id) {
    // TODO: method_registry_ haritasında runtime_type'ı ara.
    // Eğer bulunursa, içindeki haritada method_id'yi ara.
    // Cache (dispatch_cache_) önce kontrol edilebilir.

     if (dispatch_cache_.count({runtime_type, method_id})) {
        return get_method_info_from_entry_point(dispatch_cache_[{runtime_type, method_id}]); // entry_point'tan MethodInfo'yu geri alma karmaşık!
     }

    // Gerçek arama:
     auto type_it = method_registry_.find(runtime_type);
     if (type_it != method_registry_.end()) {
         auto method_it = type_it->second.find(method_id);
         if (method_it != type_it->second.end()) {
             // Metot bulundu
              dispatch_cache_[{runtime_type, method_id}] = method_it->second->entry_point; // Cache'e ekle
             return method_it->second.get(); // unique_ptr'dan raw pointer döndür
         }
     }

    // TODO: Kalıtım zincirinde yukarı doğru arama yap (base sınıflarda)

    return nullptr; // Metot bulunamadı
}


// Bir nesne üzerindeki metot çağrısını çalışma zamanında çözümler ve çalıştırır.
Object* MethodDispatcher::dispatch(Object* instance, int method_id, const std::vector<Object*>& args, ErrorReporter& reporter, SourceLocation loc) {
    if (!instance) {
        reporter.reportError(loc, ErrorCode::RUNTIME_NULL_POINTER_DEREFERENCE, "Attempted to call method on null object.");
        return nullptr; // veya hata nesnesi
    }

    Type* runtime_type = instance->getRuntimeType();
    if (!runtime_type || runtime_type->id != TypeId::CLASS) {
        // Metotlar sadece ClassType nesnelerinde çağrılabilir (veya özel built-in tiplerde)
        reporter.reportError(loc, ErrorCode::RUNTIME_CALL_NON_CALLABLE, "Attempted to call method on non-class object type '" + runtime_type->toString() + "'.");
        return nullptr; // veya hata nesnesi
    }

    // TODO: runtime_type'ı ClassType*'a cast et (güvenli cast yapın!)
     ClassType* class_type = static_cast<ClassType*>(runtime_type); // dynamic_cast daha güvenli

    // Metot bilgisini bul
    // TODO: findMethodInfo(runtime_type, method_id) çağrısı yapın
     MethodInfo* method_info = findMethodInfo(runtime_type, method_id);

    MethodInfo* method_info = nullptr; // Yer tutucu
     reporter.reportWarning(loc, ErrorCode::INTERNAL_ERROR, "Method lookup logic is a placeholder.");


    if (!method_info) {
        // Metot bulunamadı
        // TODO: method_id'den metot adını bulmaya çalış (debugging/hata mesajı için)
        std::string method_name = "unknown_method_" + std::to_string(method_id); // Yer tutucu isim
        reporter.reportError(loc, ErrorCode::RUNTIME_METHOD_NOT_FOUND, "Method '" + method_name + "' not found on object of type '" + runtime_type->toString() + "'.");
        return nullptr; // veya hata nesnesi
    }

    // Argüman sayısını ve tiplerini kontrol et
    // TODO: Argüman sayısı method_info->signature->parameter_types.size() ile args.size()'ı karşılaştır.
    // TODO: Her argümanın tipi (args[i]->getRuntimeType()) method_info->signature->parameter_types[i] ile uyumlu mu kontrol et (isAssignable veya isConvertible kullan).
    reporter.reportWarning(loc, ErrorCode::INTERNAL_ERROR, "Method argument checking is a placeholder.");


    // Metodu Çalıştır (Invocation)
    // TODO: method_info->entry_point'u kullanarak metodu çağır.
    // Çağırma şekli entry_point'un ne olduğuna bağlıdır (C++ fonksiyon pointerı, IR adresi vb.).
    // Argümanlar (instance dahil) doğru şekilde hazırlanıp paslanmalıdır.
    // Dönüş değeri Object* olarak alınmalıdır.

    Object* result = nullptr; // Yer tutucu sonuç
    reporter.reportWarning(loc, ErrorCode::INTERNAL_ERROR, "Method invocation logic is a placeholder.");

    
    // Örnek C++ fonksiyon çağrısı (basitleştirilmiş, argüman paslama mekanizması gerektirir):
    typedef Object* (*NativeMethodPtr)(Object* self, const std::vector<Object*>& args, ErrorReporter& reporter, SourceLocation loc);
    NativeMethodPtr native_func = reinterpret_cast<NativeMethodPtr>(method_info->entry_point);
    result = native_func(instance, args, reporter, loc);

    
    // Örnek IR/VM çağrısı:
    // Get the VM instance (placeholder)
     VirtualMachine& vm = VirtualMachine::getInstance();
     result = vm.callMethod(method_info->entry_point, instance, args); // VM'in çağrı arayüzü


    // Dönüş değerini döndür
    return result;
}

// TODO: registerMethod implementasyonu (method_registry_'yi doldurur)

void MethodDispatcher::registerMethod(Type* class_type, int method_id, std::unique_ptr<MethodInfo> info) {
    if (!class_type || class_type->id != TypeId::CLASS) {
        // Hata: Sınıf olmayan bir tipe metot kaydedilmeye çalışıldı.
        reporter_.reportError({0,0}, ErrorCode::INTERNAL_ERROR, "Attempted to register method to a non-class type.");
        return;
    }
     // TODO: class_type'ı ClassType*'a cast et.
      class_type_ptr = static_cast<ClassType*>(class_type);

     method_registry_ haritasına ekle
     method_registry_[class_type_ptr][method_id] = std::move(info);

    // TODO: Kalıtım varsa VTable güncelleme mantığı burada veya ClassType içinde.
}


} namespace Runtime
} namespace CCube