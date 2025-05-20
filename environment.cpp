#include "environment.h"

// Global ortam için constructor (parent'ı yok)
Environment::Environment() : enclosing(nullptr) {}

// İç içe geçmiş ortamlar için constructor (bir parent'ı var)
Environment::Environment(std::shared_ptr<Environment> enclosing)
    : enclosing(enclosing) {}

// Yeni bir değişken tanımlar
void Environment::define(const std::string& name, Value value) {
    values[name] = value;
}

// Mevcut bir değişkene değer atar
// Atama işlemi, değişkeni mevcut ortamdan başlayarak üst ortamlarda arar.
void Environment::assign(const Token& name, Value value) {
    if (values.count(name.lexeme)) { // Mevcut ortamda var mı?
        values[name.lexeme] = value;
        return;
    }
    if (enclosing != nullptr) { // Üst ortamda ara
        enclosing->assign(name, value);
        return;
    }
    // Değişken bulunamadı, hata fırlat
    throw RuntimeException(name, "Tanımlanmamış değişken '" + name.lexeme + "'.");
}

// Bir değişkenin değerini döndürür
// Değişkeni mevcut ortamdan başlayarak üst ortamlarda arar.
Value Environment::get(const Token& name) {
    if (values.count(name.lexeme)) { // Mevcut ortamda var mı?
        return values.at(name.lexeme);
    }
    if (enclosing != nullptr) { // Üst ortamda ara
        return enclosing->get(name);
    }
    // Değişken bulunamadı, hata fırlat
    throw RuntimeException(name, "Tanımlanmamış değişken '" + name.lexeme + "'.");
}

// Belirtilen uzaklıktaki ortamı bulmaya yardımcı metod
// distance: 0 = mevcut ortam, 1 = parent, 2 = parent'ın parent'ı, vb.
std::shared_ptr<Environment> Environment::ancestor(int distance) {
    std::shared_ptr<Environment> environment = shared_from_this(); // Mevcut ortam
    for (int i = 0; i < distance; ++i) {
        if (environment->enclosing == nullptr) {
            // Bu durum normalde Resolver tarafından yakalanmalıydı.
            // Resolver, her değişkenin mesafesini hesaplamalıdır.
            // Buraya gelinmesi, bir Resolver hatasına işaret edebilir.
            throw std::runtime_error("Ancestor araması sırasında beklenmeyen nullptr ortam.");
        }
        environment = environment->enclosing;
    }
    return environment;
}

// Belirli bir uzaklıktaki ortamda değişkenin değerini döndürür
// (Resolver entegre edildiğinde kullanılır)
Value Environment::getAt(int distance, const std::string& name) {
    return ancestor(distance)->values.at(name);
}

// Belirli bir uzaklıktaki ortamda değişkene değer atar
// (Resolver entegre edildiğinde kullanılır)
void Environment::assignAt(int distance, const Token& name, Value value) {
    ancestor(distance)->values[name.lexeme] = value;
}

// Bir ortamın belirtilen değişkeni içerip içermediğini kontrol eder.
bool Environment::contains(const std::string& name) const {
    return values.count(name);
}

// Ortamın üst ortamını döndürür (eğer varsa)
std::shared_ptr<Environment> Environment::getEnclosing() const {
    return enclosing;
}
