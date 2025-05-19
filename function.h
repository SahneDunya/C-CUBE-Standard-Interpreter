#ifndef C_CUBE_FUNCTION_H
#define C_CUBE_FUNCTION_H

#include "callable.h" // Callable arayüzünü kullanıyoruz
#include "ast.h"      // FunDeclStmt ve BlockStmt gibi AST düğümleri için
#include "token.h"    // Parametre isimleri için Token
#include "environment.h" // Fonksiyonun closure ortamı için

#include <vector>
#include <string>
#include <memory> // std::shared_ptr için

// İleri bildirimler (Interpreter ve Value zaten callable.h tarafından ele alınıyor)
 class Interpreter; // callable.h'de ileri bildirildi
 class Value;       // value.h'de tanımlı
 using ValuePtr = std::shared_ptr<Value>; // value.h'de tanımlı

// Kullanıcı tanımlı C-CUBE fonksiyonunu temsil eden sınıf
class C_CUBE_Function : public Callable, public std::enable_shared_from_this<C_CUBE_Function> {
    // std::enable_shared_from_this, fonksiyon nesnesinin içinde kendisine
    // shared_ptr oluşturabilmek için kullanılır (closure yaparken faydalı olabilir)
private:
    // Fonksiyonu tanımlayan AST düğümü (FunDeclStmt değil, sadece ilgili kısımlar)
    // Parametre listesi ve gövde AST'si
    const std::vector<Token> parameters;
    const StmtPtr body; // Genellikle bir BlockStmtPtr olur

    // Fonksiyonun tanımlandığı ortam (closure için gereklidir)
    const EnvironmentPtr closure;

    // Metotlar için (bir sınıfa bağlı fonksiyonlar) "this" binding'i
    // Bu, fonksiyonun bir metot çağrısı olduğunda hangi instance'a bağlı olduğunu tutar.
    // Bind metodu ile oluşturulur.
     const std::shared_ptr<C_CUBE_Object> instance; // Object sınıfı tanımlandığında kullanılacak

    // Fonksiyonun bir initializer metot mu olduğunu belirten flag (sınıf kurucusu gibi)
    // const bool isInitializer;


public:
    // Constructor
    // decl: Fonksiyon tanımının AST düğümü
    // closure: Fonksiyonun tanımlandığı Environment (closure ortamı)
    // isInitializer: Bu bir initializer (yapıcı) metot mu?
    C_CUBE_Function(const std::vector<Token>& parameters, StmtPtr body, EnvironmentPtr closure/*, bool isInitializer = false*/)
        : parameters(parameters), body(std::move(body)), closure(closure)/*, isInitializer(isInitializer)*/ {}


    // Callable arayüz metodlarının implementasyonu

    // Fonksiyonun beklediği argüman sayısını döndürür (parametre sayısı)
    int arity() const override {
        return parameters.size();
    }

    // Fonksiyonu çağırır
    ValuePtr call(Interpreter& interpreter, const std::vector<ValuePtr>& arguments) override;

    // Fonksiyonun string temsilini döndürür
    std::string toString() const override {
        // Fonksiyon adını saklamıyoruz (AST düğümünden geliyor), bu yüzden jenerik bir isim kullanıyoruz
        return "<fn>"; // Veya eğer adı saklıyorsanız "<fn " + name + ">"
    }

    // Metot binding'i için (bir method çağrıldığında 'this' referansını bağlar)
     C_CUBE_FunctionPtr bind(std::shared_ptr<C_CUBE_Object> instance); // Object sınıfı tanımlandığında kullanılacak

    // Bu bir initializer metot mu kontrolü
     bool isInitializerMethod() const { return isInitializer; }
};

// C_CUBE_Function nesnesine akıllı işaretçi alias'ı
using C_CUBE_FunctionPtr = std::shared_ptr<C_CUBE_Function>;

#endif // C_CUBE_FUNCTION_H
