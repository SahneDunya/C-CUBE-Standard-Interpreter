#ifndef CUBE_STANDARD_LIBRARY_GRAPHICS_STD_GRAPHICS_HPP
#define CUBE_STANDARD_LIBRARY_GRAPHICS_STD_GRAPHICS_HPP

#include <vector>
#include <string>
#include <memory>

#include "Data Types/object.hpp"        // Object türü için
#include "ErrorHandling/error_reporter.hpp" // Runtime hata raporlama için
#include "Data Types/type_system.hpp"   // Tip sistemine erişim için (nesne oluştururken tip lazım)
#include "Target Information/target_info.hpp" // Hedef platform bilgisi için


// TODO: Çalışma zamanı somut Object tiplerini dahil etmeniz gerekebilir
#include "Data Types/IntegerObject.hpp"
#include "Data Types/StringObject.hpp"
// ...


namespace CCube {
namespace StandardLibrary {
namespace Graphics { // Grafik modülü için iç içe namespace

// Kullanılacak grafik backend'i
enum class Backend {
    AUTO,        // Otomatik seç (varsayılan)
    VULKAN,
    OPENGL,
    OPENGL_ES,
    // TODO: Platforma özel backend'ler eklenebilir (örn: NINTENDO_SWITCH_NVN)
    NONE         // Grafik yok (hata ayıklama vb.)
};

// Grafik modülünün ana yöneticisi veya bağlam sınıfı
// Backend seçimi ve temel grafik kaynaklarının yönetimi (pencereler, cihazlar)
class GraphicsManager {
public:
    // Kurucu (Backend seçimi ve başlatma burada yapılabilir)
    GraphicsManager(ErrorReporter& reporter);

    // Yıkıcı (Grafik kaynaklarını temizler)
    ~GraphicsManager();

    // Grafik sistemi başlatma (C-CUBE kodundan çağrılabilir)
    // Gerekli parametreleri Object* olarak alır (örn: backend seçimi string olarak)
    // Başarılı olursa TrueObject*, aksi halde FalseObject* veya hata nesnesi döndürür.
    Object* initialize(const std::vector<Object*>& args, ErrorReporter& reporter, SourceLocation loc);

    // Pencere oluşturma (C-CUBE kodundan çağrılabilir)
    // Object* title, Object* width, Object* height gibi parametreler alır.
    // Başarılı olursa bir Window nesnesine işaret eden Object*, aksi halde NoneObject* veya hata nesnesi döndürür.
    Object* createWindow(const std::vector<Object*>& args, ErrorReporter& reporter, SourceLocation loc);

    // TODO: Diğer temel yöneticilik fonksiyonları

private:
    ErrorReporter& reporter_;
    Backend current_backend_ = Backend::NONE;
    // TODO: Seçilen backend'e ait bir implementasyon sınıfına pointer (VulkanManager*, OpenGLManager*)
     void* backend_impl_ = nullptr;

    // Kopya oluşturmayı ve atamayı yasakla
    GraphicsManager(const GraphicsManager&) = delete;
    GraphicsManager& operator=(const GraphicsManager&) = delete;
};


// TODO: Grafik API'lerine özgü diğer sınıfların deklarasyonları (Object'ten türeyen C-CUBE nesneleri)
// Bu sınıflar, platforma ve backend'e özgü yerel kaynakları (pencere handle'ı, shader ID'si vb.) tutar.


} namespace Graphics
} namespace StandardLibrary
} namespace CCube

#endif // CUBE_STANDARD_LIBRARY_GRAPHICS_STD_GRAPHICS_HPP