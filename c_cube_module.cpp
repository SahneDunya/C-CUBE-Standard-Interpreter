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
