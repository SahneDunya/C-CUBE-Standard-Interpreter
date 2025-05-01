#ifndef CUBE_VARIABLES_VARIABLE_HPP
#define CUBE_VARIABLES_VARIABLE_HPP

#include <string>
#include <memory> // shared_ptr için
#include "Semantics/symbol_table.hpp" // Symbol bilgisi için
#include "Data Types/object.hpp" // Değer (Object) için

namespace CCube {

// Çalışma zamanı değişkeninin kavramsal temsilcisi
// Bu yapı, compile-time Symbol bilgisini, runtime Object değeri ile bağlar.
// Derlenmiş dillerde runtime'da her değişken için ayrı bir "Variable" nesnesi olmak zorunda değildir,
// ancak bu yapı, hata ayıklama, yorumlama veya daha soyut runtime modelleri için faydalı olabilir.
struct Variable {
    // Bu değişkenin compile-time'daki sembol tablosu bilgisi
    // shared_ptr kullanmak, SymbolTable Scope'ları silinse bile Symbol bilgisine erişimi sağlar.
    std::shared_ptr<Symbol> symbol;

    // Bu değişkenin o anki değeri (bir Object'e işaretçi)
    // Object yaşam döngüsü GC tarafından yönetilir.
    Object* value;

    // Kurucu
    Variable(std::shared_ptr<Symbol> symbol_info, Object* initial_value)
        : symbol(std::move(symbol_info)), value(initial_value) {}

    // Kopyalama ve atama yapılabilir veya yasaklanabilir.
    // Runtime değişken referansları gibi davranması için default bırakabiliriz.
    // Veya tamamen yasaklayabiliriz eğer sadece pointerları geçeceksek.
     Variable(const Variable&) = delete;
     Variable& operator=(const Variable&) = delete;
     ~Variable() = default; // Pointer'a sahip değil, delete etmemeli

     // Hata ayıklama için string gösterimi
    std::string toString() const;

};

} namespace CCube

#endif // CUBE_VARIABLES_VARIABLE_HPP