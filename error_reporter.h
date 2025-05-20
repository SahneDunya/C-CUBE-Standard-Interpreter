#ifndef C_CUBE_ERROR_REPORTER_H
#define C_CUBE_ERROR_REPORTER_H

#include <string>
#include <iostream> // Hata mesajlarını basmak için
#include <stdexcept> // std::runtime_error için

#include "token.h" // Hataların token'larla ilişkilendirilmesi için

// Çalışma Zamanı Hataları için özel bir exception sınıfı
// Interpreter tarafından kullanılır.
class RuntimeException : public std::runtime_error {
public:
    Token token; // Hatanın oluştuğu token
    std::string message; // Hata mesajı

    RuntimeException(const Token& token, const std::string& message)
        : std::runtime_error("Runtime Error"), token(token), message(message) {}
};

class ErrorReporter {
private:
    bool hadError_ = false;       // Herhangi bir sözdizimi/tarama hatası oldu mu?
    bool hadRuntimeError_ = false; // Herhangi bir çalışma zamanı hatası oldu mu?

    // Yardımcı metod: Hata mesajını formatlar ve basar
    void report(int line, const std::string& where, const std::string& message);

public:
    ErrorReporter();

    // Tarama ve çözümleme hataları için (Lexer/Parser)
    void error(int line, const std::string& message);
    void error(const Token& token, const std::string& message);

    // Çalışma zamanı hataları için (Interpreter)
    void runtimeError(const RuntimeException& error);

    // Hata durumunu sorgulamak için
    bool hasError() const;
    bool hasRuntimeError() const;

    // Hata durumunu sıfırlamak için (özellikle REPL'de)
    void reset();
};

#endif // C_CUBE_ERROR_REPORTER_H
