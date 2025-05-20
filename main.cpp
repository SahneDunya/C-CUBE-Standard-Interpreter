#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream> // std::stringstream için

// Proje bağımlılıkları (gerekli başlık dosyaları)
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "error_reporter.h"
#include "environment.h" // Ortam sınıfları
#include "gc.h"          // Çöp toplayıcı
#include "value.h"       // Değer tipleri
#include "c_cube_module.h" // Modül tipi
#include "module_loader.h" // Modül yükleyici
#include "utils.h"         // Yardımcı fonksiyonlar (örn. valueToString)

// Hata raporlayıcıyı global olarak tanımlayalım, tüm bileşenler erişebilsin
ErrorReporter globalErrorReporter;

// Yorumlayıcıyı global olarak tanımlayalım (veya main içinde heap'te)
 Interpreter interpreter(globalErrorReporter, {}); // Constructor bağımlılıkları nedeniyle burada direkt başlatamayız

// Dosya çalıştırma fonksiyonu
void runFile(const std::string& path, Interpreter& interpreter) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Hata: Dosya açılamadı '" << path << "'" << std::endl;
        exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // Lexer
    Lexer lexer(source, globalErrorReporter);
    std::vector<Token> tokens = lexer.scanTokens();

    if (globalErrorReporter.hasError()) {
        std::cerr << "Tarama hatası!" << std::endl;
        return;
    }

    // Parser
    Parser parser(tokens, globalErrorReporter);
    std::vector<StmtPtr> statements = parser.parse();

    if (globalErrorReporter.hasError()) {
        std::cerr << "Çözümleme hatası!" << std::endl;
        return;
    }

    // Interpreter
    try {
        interpreter.interpret(statements);
    } catch (const RuntimeException& e) {
        // Yorumlayıcıdan gelen çalışma zamanı hatalarını yakala
        globalErrorReporter.runtimeError(e);
    } catch (const std::exception& e) {
        // Diğer genel hataları yakala
        std::cerr << "Beklenmedik hata: " << e.what() << std::endl;
    }
}

// REPL (Read-Eval-Print Loop) fonksiyonu
void runRepl(Interpreter& interpreter) {
    std::cout << "C-CUBE REPL v0.1" << std::endl;
    std::cout << "Çıkmak için 'exit()' yazın veya Ctrl+C kullanın." << std::endl;

    std::string line;
    while (true) {
        std::cout << ">>> ";
        std::getline(std::cin, line);

        if (line == "exit()") {
            break;
        }

        // Hata raporlayıcıyı her REPL komutu için sıfırla
        globalErrorReporter.reset();

        // Lexer
        Lexer lexer(line, globalErrorReporter);
        std::vector<Token> tokens = lexer.scanTokens();

        if (globalErrorReporter.hasError()) {
            // Hatalar zaten raporlandığı için devam et
            continue;
        }

        // Parser
        Parser parser(tokens, globalErrorReporter);
        // REPL için tek bir ifade veya bildirimden fazlasını işlemek gerekebilir.
        // Basit REPL'ler genellikle tek bir Statement veya Expression'ı yürütür.
        // Burada basitçe ifadeler olarak parse edelim.
        std::vector<StmtPtr> statements = parser.parse();

        if (globalErrorReporter.hasError()) {
            continue;
        }

        // Interpreter
        try {
            // Her bir statement'ı ayrı ayrı yorumla
            interpreter.interpret(statements);
            // REPL'de son ifadenin değerini basmak yaygındır,
            // ancak mevcut yorumlayıcı tasarımımız bunu doğrudan desteklemiyor.
            // Bunun için interpreter.interpret'in bir değer döndürmesi veya
            // ayrı bir `evaluateExpression` metodunun olması gerekir.
            // Şimdilik sadece statement'ları yürütüyoruz.
        } catch (const RuntimeException& e) {
            globalErrorReporter.runtimeError(e);
        } catch (const std::exception& e) {
            std::cerr << "Beklenmedik hata: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    // Modül arama yollarını belirle
    std::vector<std::string> moduleSearchPaths;
    moduleSearchPaths.push_back(".");          // Mevcut dizin
    moduleSearchPaths.push_back("./modules");  // Varsayılan 'modules' dizini

    // Interpreter nesnesini başlat
    Interpreter interpreter(globalErrorReporter, moduleSearchPaths);

    if (argc == 1) {
        // Argüman yoksa REPL modunu çalıştır
        runRepl(interpreter);
    } else if (argc == 2) {
        // Tek argüman varsa dosyayı çalıştır
        runFile(argv[1], interpreter);
    } else {
        // Geçersiz argüman sayısı
        std::cerr << "Kullanım: " << argv[0] << " [dosya]" << std::endl;
        exit(64); // EX_USAGE
    }

    return 0;
}
