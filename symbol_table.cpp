#include "symbol_table.hpp"
#include <iostream> // Hata ayıklama çıktıları için

namespace CCube {

// SymbolTable Kurucu
SymbolTable::SymbolTable(ErrorReporter& reporter)
    : reporter_(reporter), current_scope_(nullptr), current_scope_level_(0) {
    // Global kapsamı oluştur ve gir
    enterScope();
}

// Yeni bir kapsam başlatır
void SymbolTable::enterScope() {
    // Mevcut kapsamı ebeveyn yaparak yeni bir kapsam oluştur
    scopes_.push_back(std::make_unique<Scope>(current_scope_));
     current_scope_ ham işaretçisini yeni oluşturulan Scope nesnesine yönlendir
    current_scope_ = scopes_.back().get();
    current_scope_level_++;
      std::cout << "[DEBUG] Entered scope level " << current_scope_level_ << std::endl; // Hata ayıklama
}

// Mevcut kapsamdan çıkar
void SymbolTable::exitScope() {
    if (!current_scope_) {
        // Hata: Zaten bir kapsamda değilken çıkış yapılmaya çalışıldı
        // Global kapsamdan çıkış SymbolTable yıkıcısında otomatik olabilir,
        // veya burada özel olarak ele alınabilir.
        // Eger global kapsamdan çıkış yapılıyorsa parent_scope nullptr olur.
        reporter_.reportError({0, 0}, "Internal Error: Tried to exit scope when not in a scope or already exited global scope.");
        return;
    }

    // Üst kapsama geri dön
     std::cout << "[DEBUG] Exiting scope level " << current_scope_level_ << std::endl; // Hata ayıklama
    current_scope_ = current_scope_->parent_scope;

    // Kapsam yığınından mevcut kapsamı sil (unique_ptr otomatik bellek yönetimi yapar)
    scopes_.pop_back();
    current_scope_level_--;

    // TODO: Kapsamdan çıkan sembollerin cleanup'ı veya referans sayısının azalması (shared_ptr kullanıldığı için otomatik)
}

// Mevcut kapsama bir sembol ekler.
// Başarılı olursa true, zaten tanımlıysa false döner ve hata raporlar.
// type shared_ptr olarak alınır.
bool SymbolTable::declareSymbol(const std::string& name, SymbolKind kind, std::shared_ptr<Type> type, SourceLocation loc) {
    if (!current_scope_) {
        reporter_.reportError(loc, "Internal Error: Trying to declare symbol outside any scope.");
        return false;
    }

    // Check if symbol is already defined in the *current* scope
    // Aynı kapsamda aynı isimle sembol var mı kontrolü
    if (current_scope_->symbols.count(name)) {
        // Sembol zaten bu kapsamda tanımlı
        reporter_.reportError(loc, "Redeclaration of '" + name + "' in the same scope.");
        // TODO: Önceki tanımlamanın konumunu bulup raporda göstermek
        return false;
    }

    // Sembolü oluştur ve mevcut kapsama ekle
    // Symbol içinde type shared_ptr olarak tutulur.
    auto symbol = std::make_shared<Symbol>(name, kind, type, loc, current_scope_level_);
    current_scope_->symbols[name] = symbol;
     std::cout << "[DEBUG] Declared symbol: " << symbol->toString() << " in scope " << current_scope_level_ << std::endl; // Hata ayıklama
    return true;
}

// Sembolü mevcut kapsamdan başlayarak üst kapsamlara doğru arar.
// Bulursa sembolün shared_ptr'ını, bulamazsa nullptr döndürür.
std::shared_ptr<Symbol> SymbolTable::resolveSymbol(const std::string& name) {
    Scope* scope = current_scope_;
    while (scope != nullptr) {
        auto it = scope->symbols.find(name);
        if (it != scope->symbols.end()) {
            return it->second; // Sembol bulundu
        }
        scope = scope->parent_scope; // Üst kapsama geç
    }
    // Sembol bulunamadı
     std::cout << "[DEBUG] Could not resolve symbol: " + name << std::endl; // Hata ayıklama
    return nullptr;
}

// Mevcut kapsam seviyesini döndürür
int SymbolTable::getCurrentScopeLevel() const {
    return current_scope_level_;
}

} namespace CCube