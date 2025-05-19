#include "module_loader.h"
#include "interpreter.h" // Interpreter sınıfını kullanıyoruz (kodu çalıştırmak için)
#include "environment.h"   // Modül ortamları oluşturmak için
#include "lexer.h"         // Modül kodunu token'lara ayırmak için
#include "parser.h"        // Modül kodunu AST'ye ayrıştırmak için
#include "value.h"         // ValuePtr kullanıyoruz (modül objesi ve dönüş değeri için)
#include "c_cube_module.h" // C_CUBE_Module objeleri oluşturuyoruz
#include "ast.h"           // AST düğümleri için (Parser'dan dönen)

#include <fstream>     // Dosya okuma için
#include <sstream>     // String stream için
#include <iostream>    // std::cerr için
#include <filesystem>  // Dosya yolları ve varlık kontrolü için (C++17 ve sonrası)

// Namespace kısaltması (C++17)
namespace fs = std::filesystem;


// ModuleLoader implementasyonu

// Constructor
ModuleLoader::ModuleLoader(Interpreter& interpreter) : interpreter(interpreter) {
    // Arama yolları burada başlatılabilir (örneğin, geçerli dizin)
     searchPaths.push_back("."); // Geçerli dizini ekle
}

// Modül dosyasını bulma (Basit implementasyon: Path'i doğrudan dosya adı varsay)
// Daha karmaşık bir implementasyon arama yolları listesini kontrol etmelidir.

std::string ModuleLoader::findModuleFile(const std::string& modulePath) {
    // Modül path'ini dosya yoluna çevir (örn: my_module -> my_module.cube)
    std::string fileName = modulePath + ".cube";

    // Arama yollarında dosyayı ara
    for (const auto& searchPath : searchPaths) {
        fs::path fullPath = fs::path(searchPath) / fileName;
        if (fs::exists(fullPath)) {
            return fullPath.string(); // Dosya bulundu
        }
    }

    // Dosya bulunamadı
    return ""; // Boş string hata gösterir
}


// Dosyadan kaynak kodu okuma
std::string ModuleLoader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        // Hata: Dosya açılamadı (bulunamadı veya erişim sorunu)
         runtimeError(?, "Could not read module file '" + path + "'."); // Token bilgisi yok
         std::cerr << "Runtime Error: Could not read module file '" << path << "'." << std::endl;
         interpreter.hadRuntimeError = true;
        throw std::runtime_error("Could not read module file '" + path + "'."); // Veya farklı bir hata türü
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


// Modülü yükle ve çalıştır
C_CUBE_ModulePtr ModuleLoader::loadModule(const std::string& modulePath) {
    // 1. Önbellekte var mı kontrol et
    if (moduleCache.count(modulePath)) {
        std::cout << "Debug: Loading module '" << modulePath << "' from cache." << std::endl;
        return moduleCache.at(modulePath); // Önbellekten dön
    }

    std::cout << "Debug: Loading module '" << modulePath << "' from file." << std::endl;

    // 2. Modül dosyasını bul (şimdilik doğrudan dosya adını varsayalım)
    // Gerçek implementasyonda findModuleFile kullanılacak
    std::string filePath = modulePath + ".cube"; // Örnek: 'my_module' -> 'my_module.cube'

    // Dosyanın varlığını kontrol et (C++17)
    if (!fs::exists(filePath)) {
         std::cerr << "Runtime Error: Module file '" << filePath << "' not found." << std::endl;
         interpreter.hadRuntimeError = true;
         return nullptr; // Hata durumunda
          throw std::runtime_error("Module file '" + filePath + "' not found.");
    }


    // 3. Kaynak kodu dosyadan oku
    std::string source;
    try {
        source = readFile(filePath);
    } catch (const std::runtime_error& e) {
        // readFile içindeki hata zaten raporlandı
        return nullptr; // Okuma hatası durumunda
    }


    // 4. Modül kodunu token'lara ayır (Lexing)
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    // Lexer hatası varsa (Lexer'da hata flag'i set edilmeli veya istisna fırlatılmalı)
     if (lexer.hadError) return nullptr; // Eğer lexer kendi hatasını yönetiyorsa

    // 5. Tokenları ayrıştır (Parsing)
    Parser parser(tokens);
    std::vector<StmtPtr> statements;
     try {
        statements = parser.parse();
     } catch (const ParseError& error) {
        // Parser hatası zaten raporlandı
         interpreter.hadError = true; // Interpreter'da varsa
        return nullptr; // Parser hatası durumunda
     }


    // Parser hatası varsa (Parser'da hata flag'i set edilmeli veya istisna fırlatılmalı)
     if (parser.hadError) return nullptr; // Eğer parser kendi hatasını yönetiyorsa


    // 6. Modül için yeni bir ortam oluştur
    // Bu ortam, ana interpreter'ın global ortamından miras almalıdır
    // ki modül built-in'lere erişebilsin.
    EnvironmentPtr moduleEnvironment = std::make_shared<Environment>(interpreter.globals); // interpreter.globals public olmalı


    // 7. Modül objesini oluştur
    C_CUBE_ModulePtr moduleObject = std::make_shared<C_CUBE_Module>(modulePath, moduleEnvironment);


    // 8. Modülün AST'sini yeni ortamda çalıştır
    // Interpreter'ın modül ortamında çalıştırabilen bir metoduna ihtiyacı var
    // Örneğin: interpreter.executeInEnvironment(statements, moduleEnvironment);
    // Veya Interpreter'ın mevcut ortamını geçici olarak değiştirerek execute etmek
    EnvironmentPtr originalEnvironment = interpreter.environment; // Mevcut ortamı kaydet
    interpreter.environment = moduleEnvironment; // Ortamı modül ortamına ayarla

    try {
        interpreter.interpret(statements); // Modül kodunu çalıştır
        // Interpreter içindeki runtime hataları burada yakalanabilir
    } catch (const std::runtime_error& e) {
         // Modül çalışırken runtime hatası
          std::cerr << "Runtime error while loading module '" << modulePath << "': " << e.what() << std::endl;
          interpreter.hadRuntimeError = true;
         interpreter.environment = originalEnvironment; // Ortamı geri yükle
         return nullptr; // Runtime hatası durumunda
    }


    interpreter.environment = originalEnvironment; // Ortamı geri yükle

    // 9. Modül objesini önbelleğe al
    moduleCache[modulePath] = moduleObject;

    // 10. Modül objesini döndür (ValuePtr olarak sarılması interpreter tarafından yapılacak)
    return moduleObject;
}
