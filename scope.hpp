#ifndef CUBE_SEMANTICS_SCOPE_HPP
#define CUBE_SEMANTICS_SCOPE_HPP

#include <string>
#include <unordered_map>
#include <memory> // shared_ptr için
#include "symbol.hpp" // Symbol tanımı için (Bu dosyayı oluşturmanız gereklidir!)

namespace CCube {

// Tek bir kapsamı (scope) temsil eden yapı (örn: global, fonksiyon, blok, sınıf)
struct Scope {
    // Kapsam içindeki sembolleri isimlerine göre saklayan harita.
    // shared_ptr kullanmak, SymbolTable Scope'ları temizlese bile sembollere
    // başka yerlerden (örn. AST düğümleri, Variable yapıları) erişimi kolaylaştırır.
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols;

    // Üst kapsama referans (kapsam zinciri için). Ham işaretçi kullanılır
    // çünkü sahiplik SymbolTable'ın scopes_ vektöründedir.
    Scope* parent_scope;

    // Kurucu
    Scope(Scope* parent = nullptr) : parent_scope(parent) {}

    // Varsayılan yıkıcı yeterli. symbols haritasındaki shared_ptr'ların referans
    // sayıları azalır, 0 olunca sembol nesneleri silinir.
    ~Scope() = default;

    // Kopya oluşturmayı ve atamayı yasakla (kapsamlar genellikle kopyalanmaz, yığına itilir/çekilir)
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

    // TODO: İleride kapsama özgü metotlar eklenebilir (örn. sembol arama önbelleği vb.)
};

} namespace CCube

#endif // CUBE_SEMANTICS_SCOPE_HPP