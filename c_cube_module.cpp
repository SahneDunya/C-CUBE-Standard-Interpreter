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
    // std::visit kullanarak 'content' variant'ının içerdiği türe göre farklı string döndür.
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
        } else if constexpr (std::is_same_v<T, FortranModuleHandle>) {
            return arg.toString(); // FortranModuleHandle'ın kendi toString'ini çağır
        } else if constexpr (std::is_same_v<T, JuliaModuleHandle>) {
            return arg.toString(); // JuliaModuleHandle'ın kendi toString'ini çağır
        }
        return "<Unknown Module Type for " + name + ">";
    }, content);
}

// C_CUBE_Module::markChildren() implementasyonu
// Bu metot, C_CUBE_Module nesnesinin referans verdiği diğer GcObject'leri işaretler.
void C_CUBE_Module::markChildren(GarbageCollector& gc) {
    // Modülün içerdiği 'content' variant'ını kontrol et.
    std::visit([&gc](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::pair<std::vector<StmtPtr>, EnvironmentPtr>>) {
            // Eğer bir C-CUBE (.cube) modülü ise:
            // 1. Modülün ortamını işaretle (Environment'ın kendisi GcObject olduğu varsayılır)
            EnvironmentPtr moduleEnv = arg.second;
            if (moduleEnv) {
                gc.enqueueForMarking(moduleEnv);
            }

            // 2. Modülün AST'sindeki Statement'ları ve onların içindeki Expression'ları tara.
            //    Bu StmtPtr'lar ve ExprPtr'lar genellikle doğrudan GcObject değildir, ancak
            //    içerdikleri ValuePtr'lar veya GcObject'lere işaret eden yapılar olabilir.
            //    Eğer AST düğümleri GcObject'lere doğrudan referans tutuyorsa,
            //    her bir düğümün kendi markChildren() metodunu çağırmak veya
            //    burada manuel olarak tarama yapmak gerekebilir.
            //    Basit bir C-CUBE için AST düğümleri genellikle kendileri GcObject değildir,
            //    ancak içerdikleri değişkenler (örneğin StringLiteral'lar, ObjectLiteral'lar)
            //    ValuePtr üzerinden GcObject olabilir.
            //    Bu genelde Interpreter'ın yürütme sırasında ValuePtr'ları üretirken GC'ye bildirmesiyle halledilir.
            //    Burada sadece AST'deki Statement'ları döngüye alarak, içlerinde GcObject tutan
            //    ValuePtr'lar varsa onları işaretleyebiliriz. Ancak AST düğümlerinin kendileri
            //    GcObject olmadığı sürece burada doğrudan bir işaretlemeye gerek yoktur.
            //    Eğer StmtPtr veya ExprPtr'lar GcObject'e referans tutuyorsa, bu kısım güncellenmeli.
            //    Şimdilik AST düğümlerinin doğrudan GcObject olmadığını varsayıyoruz.
            //    (Çünkü genellikle AST düğümleri sadece yapısal bilgi taşır ve runtime değerleri değillerdir).
            //    Eğer StmtPtr içinde ValuePtr'lar olsaydı (örn. bir LiteralStmt), o zaman ValuePtr'ı tarardık:
            
            for (const auto& stmt : arg.first) {
                // stmt'nin içinde ValuePtr var mı kontrol et, örneğin bir LiteralStmt'teki ValuePtr
                if (auto literalStmt = std::dynamic_pointer_cast<LiteralStmt>(stmt)) {
                    if (auto gcObj = literalStmt->value->getGcObject()) {
                        gc.enqueueForMarking(gcObj);
                    }
                }
                // Diğer statement türleri için de benzer kontroller yapılabilir
            }
            
        }
        // Diğer handle türleri (Python, Native, Fortran, Julia) genellikle kendi içlerinde
        // doğrudan GcObject'lere referans tutmazlar (raw pointerlar veya FFI handle'ları olabilirler).
        // Bu nedenle, onlar için ek bir işaretleme işlemi gerekmez.
        // Eğer bu handle'lar GcObject'leri sarmalayan özel C-CUBE objelerini (örn. C_CUBE_PythonObject)
        // içerseydi, o zaman o objeler de işaretlenirdi.
        else if constexpr (std::is_same_v<T, PythonModuleHandle> ||
                           std::is_same_v<T, NativeModuleHandle>  ||
                           std::is_same_v<T, FortranModuleHandle> ||
                           std::is_same_v<T, JuliaModuleHandle>) {
            // Bu handle'lar GcObject türevleri olmadığından ve kendi içlerinde GcObject tutmadıklarından
            // burada özel bir işaretleme işlemi gerekmez.
            // Eğer gelecekte bu handle'lar GcObject'lere referans verecek olsaydı, buraya ekleme yapardık.
        }
    }, content);
}
        // Eğer bu handle'lar GcObject'leri sarmalıyorsa (örn. bir Python objesini temsil eden C_CUBE_Object),
        // o zaman onlar da işaretlenmelidir.
    }, content);
}
