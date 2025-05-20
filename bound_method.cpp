#include "bound_method.h"
#include "interpreter.h" // Interpreter'ı kullanır

BoundMethod::BoundMethod(std::shared_ptr<CCubeInstance> instance, std::shared_ptr<CCubeFunction> function)
    : instance(instance), function(function) {}

Value BoundMethod::call(Interpreter& interpreter, const std::vector<Value>& arguments) {
    // Fonksiyonu çağırmadan önce 'this' anahtar kelimesi için geçici bir ortam oluştur
    // Bu, CCubeFunction::call metodunun içinde de yapılabilir, ancak burada daha açık.
    // Fonksiyonun closure ortamını genişleten yeni bir ortam oluştur.
    std::shared_ptr<Environment> environment = std::make_shared<Environment>(function->getClosure());
    environment->define("this", instance); // 'this'i mevcut instance'a bağla
    // Çağrıyı orijinal fonksiyona devret
    return function->call(interpreter, arguments); // Bu çağrıda yeni ortam kullanılacak mı?
                                                 // function->call zaten kendi closure'ını yönetiyor.
                                                 // BoundMethod'un amacı, çağrıya başlamadan önce 'this'i ayarlamaktır.
                                                 // Aslında, 'this'i doğrudan Function::call metodunun Environment parametresine geçirmeliyiz.
                                                 // Bu, Function::call'ın imzasını değiştirmeyi gerektirebilir.
                                                 // Şu anki Function::call, Interpreter'ın geçerli ortamını kullanıyor.
                                                 // Function::call'a 'this' için özel bir parametre eklemek daha temiz olur.

    // Gelecekteki implementasyon notu: CCubeFunction::call metodu, BoundMethod tarafından geçirilen
    // 'this' nesnesini alıp kendi ortamını oluştururken onu kullanmalıdır.
    // Şimdiki haliyle, 'this'in ortamda bulunması gerekecek ki bu da current environment'ı değiştirmek anlamına gelir.
    // Daha temiz bir çözüm: CCubeFunction::call(Interpreter& interpreter, const std::vector<Value>& arguments, Value this_obj = std::monostate{})
    // ve BoundMethod bu this_obj'yi iletir.

    // Geçici olarak, 'this'i Function::call'ın içinde işlemek için Interpreter'ın o anki ortamını manipüle etmesini varsayalım.
    // Ancak bu iyi bir tasarım değildir ve özyinelemeli çağrılarda sorunlara yol açabilir.
    // En iyi yol, CCubeFunction::call'ın bir "this" argümanı almasıdır.
    // Bu yüzden bu kısım, CCubeFunction::call'ın dahili olarak nasıl çalıştığına bağlıdır.
    // interpreter.executeBlock(function->declaration->body->statements, new_environment); // Bu şekilde çağırılmalı

    // Interpreter'ın call mekanizmasında bu BoundMethod'un 'this'i nasıl enjekte ettiğine bakın.
    // Aslında, 'call' metodunda instance'ı argüman olarak geçirmek yerine,
    // doğrudan environment'a 'this'i ekleyip ardından fonksiyonu çağırmamız gerekiyor.
    // Bu, BoundMethod'ın kendi Environment oluşturması gerektiği anlamına gelir.

    // Yeniden düşünülmüş BoundMethod::call:
    // Fonksiyonun closure'ını devralan yeni bir ortam oluştur
    std::shared_ptr<Environment> method_environment = std::make_shared<Environment>(function->getClosure());
    method_environment->define("this", ObjPtr(instance)); // 'this'i bu yeni ortama tanımla

    // Argümanları fonksiyona ekle
    for (size_t i = 0; i < arguments.size(); ++i) {
        method_environment->define(function->declaration->params[i].lexeme, arguments[i]);
    }

    // Fonksiyonun gövdesini bu yeni ortamda yürüt
    try {
        interpreter.executeBlock(function->declaration->body->statements, method_environment);
    } catch (const ReturnException& result) {
        if (function->isInitializer) return instance; // Kurucular her zaman 'this'i döndürür
        return result.value;
    }

    if (function->isInitializer) return instance; // Kurucular her zaman 'this'i döndürür
    return std::monostate{}; // Void fonksiyon dönüşü
}

size_t BoundMethod::arity() const {
    return function->arity();
}

std::string BoundMethod::toString() const {
    return "<bound method " + function->toString() + ">";
}

// GC için boyut hesaplama
size_t BoundMethod::getSize() const {
    // BoundMethod objesinin kendi boyutu ve tuttuğu shared_ptr'lerin boyutu
    return sizeof(BoundMethod);
}
