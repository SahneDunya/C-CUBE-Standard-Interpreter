#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "error_reporter.h"
// Diğer include'lar

// Kaynak kodu dosyadan okuma fonksiyonu
 std::string readFile(const std::string& path);

// Programı çalıştırma (dosya veya interaktif)
 void run(const std::string& source);

// Hata olduğunu belirten flag
 bool hadError = false;
 bool hadRuntimeError = false;

// Hata raporlama fonksiyonları
 void report(int line, const std::string& where, const std::string& message);
 void error(int line, const std::string& message);
 void runtimeError(const RuntimeError& error); // Interpreter'dan gelen hatalar için

int main(int argc, char* argv[]) {
  ErrorReporter errorReporter;
    std::vector<std::string> moduleSearchPaths;
    moduleSearchPaths.push_back("."); // Mevcut dizin
    moduleSearchPaths.push_back("./modules");
    if (argc > 2) {
        std::cerr << "Usage: ccube [script]" << std::endl;
        return 1;
    } else if (argc == 2) {
        // Dosyadan çalıştır
         runFile(argv[1]);
    } else {
         İnteraktif mod (REPL)
         runPrompt();
    }

    // hadError veya hadRuntimeError durumuna göre çıkış kodu döndürme
     return hadError ? 65 : (hadRuntimeError ? 70 : 0);
 Interpreter interpreter(errorReporter, moduleSearchPaths);
    // ... yorumlayıcıyı kullan
     return 0; // Şimdilik basit bir dönüş
}

// run fonksiyonunun basit bir taslağı

void run(const std::string& source) {
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    // Tokenları yazdırma (hata ayıklama için)
     for (const auto& token : tokens) {
         std::cout << token.toString() << std::endl;
     }

    if (hadError) return; // Lexer hatası varsa devam etme

    Parser parser(tokens);
    std::vector<StmtPtr> statements = parser.parse();

    if (hadError) return; // Parser hatası varsa devam etme

    Interpreter interpreter;
    interpreter.interpret(statements); // Yorumlamayı başlat

    // Runtime hatası kontrolü yapılabilir
