#include "c_cube_module.h"
#include "environment.h" // Environment sınıfını kullanıyoruz
#include "value.h"       // ValuePtr kullanıyoruz
#include "token.h"       // Token kullanıyoruz

#include <iostream> // Hata raporlama için

// C_CUBE_Module sınıfı implementasyonu

// Constructor zaten .h dosyasında tanımlandı.

// Modül üyesine erişim
ValuePtr C_CUBE_Module::get(const Token& name) {
    // Modülün ortamında değişkeni ara
    // Environment::get metodu zaten üst ortamda arama yapar,
    // ancak modül ortamı ana interpreter ortamının üstünde olduğu için bu doğru olmaz.
    // Modülün sadece kendi ortamında araması veya farklı bir get metodu kullanması gerekir.
    // Basitlik için Environment'ın sadece kendi skopunda arayan bir metot ekleyelim (veya burada manuel arayalım).

    // Geçici çözüm: Environment'ın sadece kendi map'ine bakma (gerçek Environment::get recursive)
    // Environment sınıfınıza sadece kendi map'inde arayan bir 'getLocal' metodu ekleyebilirsiniz.
    // Veya burada map'e direk erişin (private üyeye erişim gerektirir, ideal değil)
    // ya da Environment'a public bir getter ekleyin (daha iyi).

    // Varsayalım Environment sınıfında sadece kendi skopunda arayan bir 'getLocal' metodu var:
     ValuePtr value = environment->getLocal(name.lexeme);

    // Veya Environment sınıfına sadece kendi map'inde arama yapan bir metot eklemiyorsak,
    // get metodu zaten istenen davranışı yapacaktır çünkü modülün ortamı main global ortamın
    // üzerinde yer alacak. Ancak bu durumda modül içinden kendi kendine erişim de (modül_adı.üye)
    // bu get metodunu kullanır. Bu beklenen davranış olabilir.

     ValuePtr value = environment->get(name); // Modülün ortamında ara

    if (value) {
        return value;
    }

    // Modülde böyle bir üye yoksa hata ver
    std::cerr << "[Line " << name.line << "] Runtime Error: Module '" << this->name << "' has no member '" << name.lexeme << "'." << std::endl;
     Interpreter::hadRuntimeError = true;
    return nullptr; // Hata durumunda
}

std::string C_CUBE_Module::toString() const {
    // std::visit kullanarak farklı content türlerine göre string döndür
    return std::visit([this](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return "<Module " + name + " (empty)>";
        } else if constexpr (std::is_same_v<T, std::pair<std::vector<StmtPtr>, EnvironmentPtr>>) {
            return "<C-CUBE Module " + name + ">";
        } else if constexpr (std::is_same_v<T, PythonModuleHandle>) {
            return arg.toString(); // PythonModuleHandle'ın kendi toString'ini çağır
        } else if constexpr (std::is_same_v<T, NativeModuleHandle>) {
            return arg.toString(); // NativeModuleHandle'ın kendi toString'ini çağır
        }
        return "<Unknown Module Type for " + name + ">";
    }, content);
}

void C_CUBE_Module::markChildren() {
    // Modülün içerdiği GcObject'leri işaretle
    std::visit([this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::pair<std::vector<StmtPtr>, EnvironmentPtr>>) {
            // C-CUBE modülü: Ortamını ve AST'sini işaretle
            // Ortamın kendisi GcObject ise (environment.h'da GcObject'ten türemişse)
            if (arg.second) {
                arg.second->markChildren(); // Ortam içindeki GcObject'leri işaretle
            }
            // AST'deki GcObject'leri işaretle (eğer AST düğümleri GcObject'ler tutuyorsa)
            // Bunu Interpreter'ın kendisi yürütürken yapabilir
            // Veya her StmtPtr içinde GcObject'lere referans tutan bir yapı varsa, o da işaretlenmeli
        }
        // PythonModuleHandle, NativeModuleHandle gibi sınıflar kendi içlerinde
        // GcObject'lere referans tutmuyorsa (örn. sadece raw pointer veya FFI handle'larıysa),
        // bu durumda markChildren'a ek bir şey gerekmez.
        // Eğer bu handle'lar GcObject'leri sarmalıyorsa (örn. bir Python objesini temsil eden C_CUBE_Object),
        // o zaman onlar da işaretlenmelidir.
    }, content);
}
