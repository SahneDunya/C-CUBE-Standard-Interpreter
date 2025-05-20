#include "environment.h"
#include "error_reporter.h" // Hata raporlama için (eğer global ise)
#include "value.h"          // ValuePtr'ları kullanmak için
#include "token.h"          // Hata raporlamada Token kullanmak için

#include <iostream> // Debugging için

// Harici bir ErrorReporter instance'ı olduğunu varsayalım
 extern ErrorReporter globalErrorReporter;

// Global skop için constructor
Environment::Environment() : enclosing(nullptr) {}

// İç içe skoplar için constructor
Environment::Environment(std::shared_ptr<Environment> enclosing)
    : _enclosing(enclosing) {
}

// Bir değişken tanımlar veya günceller.
void Environment::define(const std::string& name, ValuePtr value) {
    values[name] = value;
}

// Bir değişkenin değerini alır. Bulamazsa hata fırlatır.
ValuePtr Environment::get(const Token& name) {
    if (values.count(name.lexeme)) {
        return values.at(name.lexeme);
    }

    // Eğer bu ortamda yoksa, üst ortamda ara (kapsam zinciri)
    if (_enclosing) {
        return _enclosing->get(name);
    }

    // globalErrorReporter.runtimeError(name, "Undefined variable '" + name.lexeme + "'.");
    throw std::runtime_error("Undefined variable '" + name.lexeme + "' at line " + std::to_string(name.line) + ".");
}

// Bir değişkenin değerini atar. Bulamazsa hata fırlatır.
void Environment::assign(const Token& name, ValuePtr value) {
    if (values.count(name.lexeme)) {
        values[name.lexeme] = value;
        return;
    }

    // Eğer bu ortamda yoksa, üst ortamda ata
    if (_enclosing) {
        _enclosing->assign(name, value);
        return;
    }

    // globalErrorReporter.runtimeError(name, "Undefined variable '" + name.lexeme + "'.");
    throw std::runtime_error("Undefined variable '" + name.lexeme + "' at line " + std::to_string(name.line) + ".");
}

// GcObject arayüzü: Bu ortamın çocuklarını işaretle.
void Environment::markChildren(GarbageCollector& gc) {
    // 1. Üst ortamı işaretle (eğer varsa ve GcObject ise)
    if (_enclosing) {
        gc.enqueueForMarking(_enclosing); // _enclosing, kendisi GcObject olduğundan doğrudan enqueue edilebilir
    }

    // 2. Bu ortamdaki tüm değerleri işaretle
    for (const auto& pair : values) {
        // Her ValuePtr'ın içinde bir GcObject olup olmadığını kontrol et
        if (auto gcObj = pair.second->getGcObject()) {
            gc.enqueueForMarking(gcObj); // Eğer varsa, çöp toplayıcıya bildir
        }
    }
}
void Environment::forEachValue(std::function<void(ValuePtr)> callback) const {
    for (const auto& pair : values) {
        callback(pair.second);
    }
}
