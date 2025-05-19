#ifndef C_CUBE_C_CUBE_MODULE_H
#define C_CUBE_C_CUBE_MODULE_H

#include "environment.h" // EnvironmentPtr için
#include "value.h"       // ValuePtr için
#include "token.h"       // Token için (hata raporlama)

#include <string>
#include <memory>

// C-CUBE modülünü temsil eden runtime değeri
class C_CUBE_Module {
private:
    // Modülün üst seviye ortamı (global değişkenleri, fonksiyonları vb. içerir)
    EnvironmentPtr environment;
    std::string name; // Modülün adı

public:
    // Constructor
    C_CUBE_Module(std::string name, EnvironmentPtr environment)
        : name(std::move(name)), environment(environment) {}

    // Modülün bir üyesine (değişken, fonksiyon vb.) erişim.
    // Modülün ortamında arama yapar.
    // name: Erişilmek istenen üyenin adı (Token).
    ValuePtr get(const Token& name);

    // Modülün string temsilini döndürür.
    std::string toString() const {
        return "<module '" + name + "'>";
    }
};

using C_CUBE_ModulePtr = std::shared_ptr<C_CUBE_Module>;

#endif // C_CUBE_C_CUBE_MODULE_H
