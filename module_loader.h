#ifndef C_CUBE_MODULE_LOADER_H
#define C_CUBE_MODULE_LOADER_H

#include "c_cube_module.h" // C_CUBE_ModulePtr için
#include "environment.h"   // EnvironmentPtr için

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// İleri bildirimler
class Interpreter;
class Lexer;
class Parser;
 class Value; // value.h zaten c_cube_module.h tarafından dahil ediliyor

// Modülleri yükleyen ve yöneten sınıf
class ModuleLoader {
private:
    // Modül önbelleği: Yüklenen modülleri (modül adı -> modül objesi) saklar
    std::unordered_map<std::string, C_CUBE_ModulePtr> moduleCache;

    // Modül kodunu çalıştırmak için Interpreter'a referans
    Interpreter& interpreter;

    // Modül kaynak dosyalarının aranacağı yollar (gelecekte eklenebilir)
     std::vector<std::string> searchPaths;


    // Yardımcı fonksiyonlar (private)
     std::string findModuleFile(const std::string& modulePath); // Dosya sisteminde modül dosyasını bulur
    std::string readFile(const std::string& path); // Dosyadan kaynak kodu okur

public:
    // Constructor: Interpreter'a referans alır.
    ModuleLoader(Interpreter& interpreter);

    // Modülü yükler ve çalıştırır. Modül objesini döndürür.
    // Eğer modül önbellekte varsa, önbellekten döner.
    C_CUBE_ModulePtr loadModule(const std::string& modulePath);

    // Önbelleği temizleme gibi ek metodlar eklenebilir.
};

#endif // C_CUBE_MODULE_LOADER_H
