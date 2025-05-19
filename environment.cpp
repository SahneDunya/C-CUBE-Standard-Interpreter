#include "environment.h"
#include "token.h" // Hata mesajları için
#include "value.h" // ValuePtr tipini kullanıyor
#include <iostream>
#include <stdexcept> // std::runtime_error için

// Global skop için constructor
Environment::Environment() : enclosing(nullptr) {}

// İç içe skoplar için constructor
Environment::Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

// Değişken tanımlama
void Environment::define(const std::string& name, ValuePtr value) {
    // Dinamik diller genellikle yeniden tanımlamaya izin verir
     values[name] = value;

    // Eğer yeniden tanımlamaya izin vermek istemiyorsanız:
    if (values.count(name)) {
        // error(name, "Variable '" + name + "' already defined in this scope.");
        // Veya bir istisna fırlatabilirsiniz
        std::cerr << "Runtime Error: Variable '" << name << "' already defined in this scope." << std::endl;
        // Hata durumunu işaretleyebilirsiniz
         Interpreter::hadRuntimeError = true;
        return; // Tanımlama başarısız oldu
    }
    values[name] = value;
}

// Değişken değeri alma
ValuePtr Environment::get(const Token& name) {
    // Önce mevcut ortamda ara
    if (values.count(name.lexeme)) {
        return values.at(name.lexeme);
    }

    // Mevcut ortamda yoksa, bir üst ortama sor
    if (enclosing) {
        return enclosing->get(name); // Rekürsif çağrı
    }

    // En üst ortama kadar çıktık ve hala bulamadık
     runtimeError(name, "Undefined variable '" + name.lexeme + "'.");
     std::cerr << "[Line " << name.line << "] Runtime Error: Undefined variable '" << name.lexeme << "'." << std::endl;
     // Hata durumunu işaretleyebilirsiniz
      Interpreter::hadRuntimeError = true;

     // Tanımlanmamış değişken için özel bir değer döndür (örneğin None) veya istisna fırlat
      return std::make_shared<Value>(); // std::monostate (None) içeren Value
     // veya istisna: throw std::runtime_error("Undefined variable '" + name.lexeme + "' at line " + std::to_string(name.line));
     // Şimdilik null pointer döndürelim, interpreter bunu kontrol etmeli
     return nullptr;
}

// Değişken değeri atama (zaten tanımlanmış olmalı)
void Environment::assign(const Token& name, ValuePtr value) {
    // Önce mevcut ortamda ara
    if (values.count(name.lexeme)) {
        values[name.lexeme] = value;
        return; // Atama tamamlandı
    }

    // Mevcut ortamda yoksa, bir üst ortama sor
    if (enclosing) {
        enclosing->assign(name, value); // Rekürsif çağrı
        return; // Atama tamamlandı (üst ortamda bulundu)
    }

    // En üst ortama kadar çıktık ve hala bulamadık (değişken tanımlı değil)
     runtimeError(name, "Undefined variable '" + name.lexeme + "'.");
    std::cerr << "[Line " << name.line << "] Runtime Error: Undefined variable '" << name.lexeme << "'." << std::endl;
    // Hata durumunu işaretleyebilirsiniz
     Interpreter::hadRuntimeError = true;
}

// Eğer Resolver aşaması eklenecekse, bu metodlar implement edilir

EnvironmentPtr Environment::ancestor(int distance) {
    std::shared_ptr<Environment> environment = shared_from_this(); // Şu anki ortam
    for (int i = 0; i < distance; ++i) {
        environment = environment->enclosing;
    }
    return environment;
}

ValuePtr Environment::getAt(int distance, const std::string& name) {
    return ancestor(distance)->values.at(name);
}

void Environment::assignAt(int distance, const std::string& name, ValuePtr value) {
    ancestor(distance)->values[name] = value;
}
