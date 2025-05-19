#ifndef C_CUBE_CALLABLE_H
#define C_CUBE_CALLABLE_H

#include <vector>
#include <string>
#include <memory> // std::shared_ptr için

// Interpreter sınıfını burada kullanacağımız için ileri bildirim yapıyoruz.
// Bu, Callable sınıfının Interpreter tanımından önce kullanılabilmesini sağlar
// ve dairesel bağımlılıkları önlemeye yardımcı olur.
class Interpreter;

// ValuePtr tipinin tanımlı olduğu value.h dosyasını dahil ediyoruz.
// Call metodunun argümanları ve dönüş tipi ValuePtr olacaktır.
#include "value.h"

// C-CUBE dilinde çağrılabilen her şey için temel soyut sınıf (arayüz)
class Callable {
public:
    // Sanal yıkıcı: Türemiş sınıflar doğru şekilde temizlenebilsin.
    virtual ~Callable() = default;

    // Bu çağrılabilir öğenin beklediği argüman sayısını döndürür.
    // Değişken sayıda argüman alanlar için özel bir değer (örn: -1) kullanılabilir.
    virtual int arity() const = 0;

    // Çağrılabilir öğeyi verilen argümanlarla çalıştırır.
    // Parametreler:
    //   interpreter: Çalıştırmayı gerçekleştiren yorumlayıcı nesnesi.
    //                Fonksiyon gövdesini çalıştırmak için yorumlayıcının state'ine (ortamına) erişim gerekebilir.
    //   arguments: Çağrıya geçirilen değerlerin bir vektörü (ValuePtr).
    // Geri dönüş değeri: Çağrının sonucu olan değer (ValuePtr).
    virtual ValuePtr call(Interpreter& interpreter, const std::vector<ValuePtr>& arguments) = 0;

    // Çağrılabilir öğenin string temsilini döndürür (hata ayıklama veya print için kullanışlı).
    virtual std::string toString() const = 0;
};

// Callable nesnelerine akıllı işaretçi (shared_ptr) için bir alias
using CallablePtr = std::shared_ptr<Callable>;

#endif // C_CUBE_CALLABLE_H
