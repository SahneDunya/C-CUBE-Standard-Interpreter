#ifndef CUBE_RUNTIME_METHOD_DISPATCHER_HPP
#define CUBE_RUNTIME_METHOD_DISPATCHER_HPP

#include <vector>
#include <string>
#include <memory>
#include <unordered_map> // Metot lookup cache veya tanımlar için

#include "Data Types/object.hpp"        // Object türü için (metot çağrılan nesne)
#include "Data Types/type.hpp"          // Type bilgisi için (Object::getRuntimeType())
#include "ErrorHandling/error_reporter.hpp" // Runtime hata raporlama için

// TODO: Çalışma zamanı somut Object tiplerini dahil etmeniz gerekebilir
//Argüman tiplerini kontrol ederken veya sonuç objelerini işlerken.
#include "Data Types/IntegerObject.hpp"
#include "Data Types/StringObject.hpp"
// ...


namespace CCube {
namespace Runtime { // Çalışma zamanı bileşenleri için namespace

// Metot çağırma bilgisini temsil eden yapı (dahili kullanım için)
struct MethodInfo {
    std::string name;       // Metot adı
    std::unique_ptr<FunctionType> signature; // Metot imzası (parametre ve dönüş tipleri)
    // Metot implementasyonunun çalışma zamanı entry point'i.
    // Bu bir C++ fonksiyon pointer'ı, IR block index'i, veya VM opcode'u olabilir.
    void* entry_point = nullptr;

    // TODO: VTable indexi gibi diğer bilgiler eklenebilir
};

// Çalışma zamanında metot çağrılarını çözümleyen ve yönlendiren (dispatch) sınıfı
// Bu genellikle bir Singleton olacaktır.
class MethodDispatcher {
public:
    // Singleton instance'ına erişim metodu
    static MethodDispatcher& getInstance();

    // Singleton kopyalanamaz ve atama yapılamaz
    MethodDispatcher(const MethodDispatcher&) = delete;
    MethodDispatcher& operator=(const MethodDispatcher&) = delete;

    // Bir nesne üzerindeki metot çağrısını çalışma zamanında çözümler ve çalıştırır.
    // instance: Metodun çağrıldığı nesne (Object* this/self)
    // method_id: Çağrılacak metodun ID'si (Derleme zamanında belirlenir, string lookup'tan daha hızlıdır)
    // args: Metoda geçirilen argümanlar (Object* listesi)
    // reporter: Runtime hatalarını raporlamak için
    // loc: Çağrının kaynak kod konumu (runtime hataları için)
    // Başarılı olursa metodun döndürdüğü Object* pointerını, aksi halde nullptr veya hata nesnesi döndürür.
    Object* dispatch(Object* instance, int method_id, const std::vector<Object*>& args, ErrorReporter& reporter, SourceLocation loc);

    // TODO: Metot adını çalışma zamanı ID'sine çeviren bir mekanizma gerekebilir (daha çok CodeGen/VM kullanır)
     int getMethodId(const std::string& name);

    // TODO: Derleme aşamasında veya Runtime başlatılırken metot bilgilerini (MethodInfo) kaydetme metotları
     void registerMethod(Type* class_type, int method_id, std::unique_ptr<MethodInfo> info);


private:
    // Özel kurucu (Singleton)
    MethodDispatcher(ErrorReporter& reporter);

    ErrorReporter& reporter_;

    // Runtime'da erişilebilen tüm metot bilgileri (Type* ve Method ID'ye göre haritalanabilir)
    // Bu harita, derleme aşamasında veya Runtime başlatılırken doldurulur.
    // Basit örnek: Type* ve Method ID'ye göre MethodInfo* pointerı.
     std::unordered_map<Type*, std::unordered_map<int, std::unique_ptr<MethodInfo>>> method_registry_;

    // Alternatif: Type* ve Method ID'ye göre sadece entry_point pointerı tutan daha hızlı lookup.
     std::unordered_map<Type*, std::unordered_map<int, void*>> method_entry_points_;

    // Lookup hızlandırmak için bir cache
     std::unordered_map<std::pair<Type*, int>, void*> dispatch_cache_; // pair hash'leme gerektirir.


    // Metot bilgisini (MethodInfo) runtime tipine ve ID'ye göre arar
     MethodInfo* findMethodInfo(Type* runtime_type, int method_id); // Implementasyonu .cpp'de
};

} namespace Runtime
} namespace CCube

#endif // CUBE_RUNTIME_METHOD_DISPATCHER_HPP