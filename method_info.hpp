#ifndef CUBE_RUNTIME_METHOD_INFO_HPP
#define CUBE_RUNTIME_METHOD_INFO_HPP

#include <string>
#include <memory> // unique_ptr için
#include "Data Types/type.hpp" // FunctionType tanımı için


namespace CCube {
namespace Runtime {

// Metot çağırma bilgisini temsil eden yapı (Çalışma zamanı ve Derleme zamanı kullanımı için)
// Bir sınıf metodunun adını, imzasını ve çalışma zamanındaki giriş noktasını (entry point) saklar.
struct MethodInfo {
    std::string name;       // Metot adı
    // Metot imzası (parametre ve dönüş tipleri)
     std::unique_ptr kullanmak, sahipliğin MethodInfo nesnesinde olmasını sağlar.
    std::unique_ptr<FunctionType> signature;

    // Metot implementasyonunun çalışma zamanı giriş noktası.
    // Bu, derlenmiş kodun veya VM'in metodu çağırmak için kullanacağı adrestir/tanımlayıcıdır.
    // Tipi, derleyicinin kod üretimi ve çalışma zamanı mimarisine bağlıdır:
    // - C++ fonksiyon pointer'ı (eğer standart kütüphane/yerel metotsa)
    // - IR Basic Block indexi/pointer'ı (eğer C-CUBE koduysa ve VM kullanılıyorsa)
    // - Makine kodu adresi
    void* entry_point = nullptr; // Genel void* kullanıyoruz, gerçek tipi CodeGen/Runtime belirler.

    // TODO: VTable indexi, metot ID'si, erişim belirteci (public, private) gibi diğer bilgiler eklenebilir.
     int method_id = -1;
     AccessSpecifier access = AccessSpecifier::PUBLIC; // Eğer dil destekliyorsa

    // Kurucu
    MethodInfo(std::string name, std::unique_ptr<FunctionType> signature, void* entry_point = nullptr)
        : name(std::move(name)), signature(std::move(signature)), entry_point(entry_point) {}

    // Varsayılan kopyalama/atama ve yıkıcı yeterli (unique_ptr taşınır)
    // Eğer MethodInfo'lar karmaşık yönetim gerektirirse kopyalama yasaklanabilir.
    MethodInfo(const MethodInfo&) = default;
    MethodInfo& operator=(const MethodInfo&) = default;
    ~MethodInfo() = default;

    // Hata ayıklama için string gösterimi
    std::string toString() const;
};

} namespace Runtime
} namespace CCube

#endif // CUBE_RUNTIME_METHOD_INFO_HPP