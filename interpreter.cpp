#include "interpreter.h"
#include <cmath>     // pow için (üst alma)
#include <algorithm> // for std::remove_if if needed, not directly used here but good to have for general list ops
#include <sstream>   // for converting numbers to string
#include <chrono>    // for native clock function

// Constructor
Interpreter::Interpreter(ErrorReporter& reporter, Gc& gc_instance, ModuleLoader& loader)
    : globals(std::make_shared<Environment>()), environment(globals),
      errorReporter(reporter), gc(gc_instance), moduleLoader(loader) {
    // Yerleşik fonksiyonları global ortama ekle
    BuiltinFunctions::defineBuiltins(globals, gc);
}

// Programı yorumlamaya başlar
void Interpreter::interpret(const std::vector<StmtPtr>& statements) {
    try {
        for (const auto& stmt : statements) {
            execute(stmt);
        }
    } catch (const RuntimeException& e) {
        errorReporter.runtimeError(e);
    }
}

// Bir ifadeyi değerlendirir
Value Interpreter::evaluate(ExprPtr expr) {
    // Visitor desenini kullanarak uygun visit metodunu çağırır.
    // Bu, AST düğümünün türüne göre doğru implementasyonun seçilmesini sağlar.
    return expr->accept(*this);
}

// Bir bildirimi yürütür
void Interpreter::execute(StmtPtr stmt) {
    // Visitor desenini kullanarak uygun visit metodunu çağırır.
    stmt->accept(*this);
}

// Bir değerin doğruluk değerini kontrol eder (Python'daki gibi)
bool Interpreter::isTruthy(const Value& value) {
    if (std::holds_alternative<std::monostate>(value)) return false; // none is false
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
    if (std::holds_alternative<double>(value)) return std::get<double>(value) != 0.0;
    if (std::holds_alternative<std::string>(value)) return !std::get<std::string>(value).empty();
    // Diğer tüm objeler (fonksiyonlar, sınıflar, objeler) true'dur.
    return true;
}

// İki değerin eşitliğini kontrol eder
bool Interpreter::isEqual(const Value& a, const Value& b) {
    // Aynı türde değillerse eşit değildir
    if (a.index() != b.index()) return false;

    if (std::holds_alternative<std::monostate>(a)) {
        return true; // none == none
    } else if (std::holds_alternative<bool>(a)) {
        return std::get<bool>(a) == std::get<bool>(b);
    } else if (std::holds_alternative<double>(a)) {
        return std::get<double>(a) == std::get<double>(b);
    } else if (std::holds_alternative<std::string>(a)) {
        return std::get<std::string>(a) == std::get<std::string>(b);
    } else if (std::holds_alternative<ObjPtr>(a)) {
        // Obje pointer'ları doğrudan karşılaştırılır (aynı obje mi?)
        return std::get<ObjPtr>(a) == std::get<ObjPtr>(b);
    }
    // Diğer türler için özel eşitlik mantığı eklenebilir.
    return false;
}

// Sayı operandı kontrolü (tekli)
void Interpreter::checkNumberOperand(const Token& op, const Value& operand) {
    if (std::holds_alternative<double>(operand)) return;
    throw runtimeError(op, "Operand bir sayı olmalıdır.");
}

// Sayı operandı kontrolü (ikili)
void Interpreter::checkNumberOperands(const Token& op, const Value& left, const Value& right) {
    if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) return;
    throw runtimeError(op, "Operandlar sayı olmalıdır.");
}

// Çalışma zamanı hatası fırlatır
RuntimeException Interpreter::runtimeError(const Token& token, const std::string& message) {
    return RuntimeException(token, message);
}

// Ortam yönetimi: Yeni bir ortamda kod bloğunu yürütür
void Interpreter::executeBlock(const std::vector<StmtPtr>& statements, std::shared_ptr<Environment> newEnvironment) {
    std::shared_ptr<Environment> previousEnvironment = this->environment;
    try {
        this->environment = newEnvironment; // Ortamı yeniye ayarla
        for (const auto& stmt : statements) {
            execute(stmt);
        }
    } catch (...) {
        this->environment = previousEnvironment; // Çıkarken eski ortama geri dön
        throw; // Yakalanan exception'ı tekrar fırlat
    }
    this->environment = previousEnvironment; // Normalde eski ortama geri dön
}

// Değişken çözümlemesi (Şimdilik doğrudan ortamda arama yapar)
Value Interpreter::lookUpVariable(const Token& name) {
    // Resolver henüz entegre edilmediği için doğrudan mevcut ortamda arama yaparız.
    // Eğer değişken mevcut ortamda yoksa, global ortama kadar yukarı çıkarız.
    // Resolver entegre edildiğinde, burada 'locals' map'i kullanılacak.
    if (environment->contains(name.lexeme)) {
        return environment->get(name);
    } else if (globals->contains(name.lexeme)) {
        return globals->get(name);
    }
    throw runtimeError(name, "Tanımlanmamış değişken '" + name.lexeme + "'.");
}


// --- ExprVisitor Metotlarının Implementasyonları ---

Value Interpreter::visitBinaryExpr(std::shared_ptr<BinaryExpr> expr) {
    Value left = evaluate(expr->left);
    Value right = evaluate(expr->right);

    switch (expr->op.type) {
        case TokenType::MINUS:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) - std::get<double>(right);
        case TokenType::SLASH:
            checkNumberOperands(expr->op, left, right);
            if (std::get<double>(right) == 0.0) {
                throw runtimeError(expr->op, "Sıfıra bölme hatası.");
            }
            return std::get<double>(left) / std::get<double>(right);
        case TokenType::STAR:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) * std::get<double>(right);
        case TokenType::PLUS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return std::get<double>(left) + std::get<double>(right);
            }
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                return std::get<std::string>(left) + std::get<std::string>(right);
            }
            throw runtimeError(expr->op, "Operanlar sayılar veya stringler olmalıdır.");
        case TokenType::GREATER:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) > std::get<double>(right);
        case TokenType::GREATER_EQUAL:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) >= std::get<double>(right);
        case TokenType::LESS:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) < std::get<double>(right);
        case TokenType::LESS_EQUAL:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) <= std::get<double>(right);
        case TokenType::BANG_EQUAL: return !isEqual(left, right);
        case TokenType::EQUAL_EQUAL: return isEqual(left, right);
        default:
            // Ulaşılmamalı
            break;
    }
    return std::monostate{}; // Varsayılan dönüş
}

Value Interpreter::visitCallExpr(std::shared_ptr<CallExpr> expr) {
    Value callee = evaluate(expr->callee); // Çağrılan ifadeyi değerlendir

    std::vector<Value> arguments;
    for (const auto& arg : expr->arguments) {
        arguments.push_back(evaluate(arg));
    }

    if (!std::holds_alternative<ObjPtr>(callee)) {
        throw runtimeError(expr->paren, "Sadece fonksiyonlar ve sınıflar çağrılabilir.");
    }

    ObjPtr obj_callee = std::get<ObjPtr>(callee);
    if (!obj_callee->isCallable()) {
        throw runtimeError(expr->paren, "Sadece fonksiyonlar ve sınıflar çağrılabilir.");
    }

    Callable* callable = static_cast<Callable*>(obj_callee.get());

    if (arguments.size() != callable->arity()) {
        throw runtimeError(expr->paren, "Beklenen " + std::to_string(callable->arity()) +
                                        " argüman, ancak " + std::to_string(arguments.size()) + " geldi.");
    }

    return callable->call(*this, arguments);
}

Value Interpreter::visitGetExpr(std::shared_ptr<GetExpr> expr) {
    Value object = evaluate(expr->object);

    if (std::holds_alternative<ObjPtr>(object)) {
        ObjPtr instance = std::get<ObjPtr>(object);
        if (instance->getType() == Object::ObjectType::INSTANCE) {
            auto ccube_instance = std::static_pointer_cast<CCubeInstance>(instance);
            Value result = ccube_instance->get(expr->name);
            if (std::holds_alternative<ObjPtr>(result) && std::static_pointer_cast<CCubeFunction>(std::get<ObjPtr>(result))) {
                // Metodu objeye bağla
                return std::make_shared<BoundMethod>(ccube_instance, std::static_pointer_cast<CCubeFunction>(std::get<ObjPtr>(result)));
            }
            return result;
        } else if (instance->getType() == Object::ObjectType::C_CUBE_MODULE) {
            // Modül içindeki üyeye erişim
            auto module = std::static_pointer_cast<CCubeModule>(instance);
            return module->getMember(expr->name);
        }
    }
    // Diğer tiplerde property erişimi hata verir
    throw runtimeError(expr->name, "Sadece objeler, modüller veya sınıflar property'lere sahip olabilir.");
}

Value Interpreter::visitGroupingExpr(std::shared_ptr<GroupingExpr> expr) {
    return evaluate(expr->expression);
}

Value Interpreter::visitLiteralExpr(std::shared_ptr<LiteralExpr> expr) {
    return expr->value;
}

Value Interpreter::visitLogicalExpr(std::shared_ptr<LogicalExpr> expr) {
    Value left = evaluate(expr->left);

    if (expr->op.type == TokenType::OR) {
        if (isTruthy(left)) return left; // Sol doğruysa, sağa bakmaya gerek yok
    } else { // AND
        if (!isTruthy(left)) return left; // Sol yanlışsa, sağa bakmaya gerek yok
    }

    return evaluate(expr->right); // Sağ tarafı değerlendir
}

Value Interpreter::visitSetExpr(std::shared_ptr<SetExpr> expr) {
    Value object = evaluate(expr->object); // Obje değerini al

    if (!std::holds_alternative<ObjPtr>(object) || std::get<ObjPtr>(object)->getType() != Object::ObjectType::INSTANCE) {
        throw runtimeError(expr->name, "Sadece objelerin property'leri atanabilir.");
    }

    Value value = evaluate(expr->value); // Atanacak değeri al
    std::static_pointer_cast<CCubeInstance>(std::get<ObjPtr>(object))->set(expr->name, value);
    return value; // Atanan değeri döndür
}

Value Interpreter::visitSuperExpr(std::shared_ptr<SuperExpr> expr) {
    // Resolver varsa, 'locals' map'ini kullanırız. Resolver yoksa, bu biraz daha karmaşık.
    // Şimdilik, 'super' bağlamını en yakın üst sınıfın metoduna bağlarız.
    // Varsayım: 'super' keyword'ü her zaman 'this' gibi bir instance method içinde kullanılır.

    // Super sınıfını bul
    // Burası, resolver'ın scope depth'ini veya başka bir bağlam bilgisini sağlamasını gerektirecek.
    // Şimdilik, basit bir örnek olarak, bir "geçici" mekanizma kullanalım.
    // Gerçek implementasyonda, resolver 'super' için özel bir scope depth verir.

    // `this` için `CCubeInstance`'ı bulmalıyız. Bu genelde mevcut environment'ın bir özelliği olur.
    // Eğer `this` kelimesi çözüldüyse, onun değeri zaten mevcut ortamda olmalı.

    // Geçici çözüm: `this`'i ortamdan bul
    Value this_value = environment->get(Token(TokenType::THIS, "this", std::monostate{}, expr->keyword.line));
    if (!std::holds_alternative<ObjPtr>(this_value) || std::get<ObjPtr>(this_value)->getType() != Object::ObjectType::INSTANCE) {
        throw runtimeError(expr->keyword, "'super' anahtar kelimesi sadece metot içinde kullanılabilir.");
    }
    std::shared_ptr<CCubeInstance> instance = std::static_pointer_cast<CCubeInstance>(std::get<ObjPtr>(this_value));

    // Üst sınıfı bul (Bu bilgi genellikle ClassStmt'den veya Resolver'dan gelir)
    // `instance->get_class()` bize mevcut sınıfı verir.
    // `instance->get_class()->superclass` bize üst sınıfı verir.
    std::shared_ptr<CCubeClass> superclass = instance->get_class()->superclass;

    if (superclass == nullptr) {
        throw runtimeError(expr->keyword, "Üst sınıfı olmayan bir objenin 'super' metodu çağrılamaz.");
    }

    // Metodu üst sınıftan al
    std::shared_ptr<CCubeFunction> method = superclass->findMethod(expr->method.lexeme);

    if (method == nullptr) {
        throw runtimeError(expr->method, "Tanımlanmamış üst sınıf metodu '" + expr->method.lexeme + "'.");
    }

    // Metodu mevcut instance'a bağla ve döndür
    return std::make_shared<BoundMethod>(instance, method);
}

Value Interpreter::visitThisExpr(std::shared_ptr<ThisExpr> expr) {
    return lookUpVariable(expr->keyword); // 'this' bir değişkendir
}

Value Interpreter::visitUnaryExpr(std::shared_ptr<UnaryExpr> expr) {
    Value right = evaluate(expr->right);

    switch (expr->op.type) {
        case TokenType::BANG: return !isTruthy(right);
        case TokenType::MINUS:
            checkNumberOperand(expr->op, right);
            return -std::get<double>(right);
        default:
            // Ulaşılmamalı
            break;
    }
    return std::monostate{}; // Varsayılan dönüş
}

Value Interpreter::visitVariableExpr(std::shared_ptr<VariableExpr> expr) {
    return lookUpVariable(expr->name);
}

Value Interpreter::visitListLiteralExpr(std::shared_ptr<ListLiteralExpr> expr) {
    std::vector<Value> elements;
    for (const auto& elem_expr : expr->elements) {
        elements.push_back(evaluate(elem_expr));
    }
    // GC tarafından yönetilen bir liste objesi oluştur
    return gc.createList(elements);
}


// --- StmtVisitor Metotlarının Implementasyonları ---

void Interpreter::visitBlockStmt(std::shared_ptr<BlockStmt> stmt) {
    // Yeni bir ortam oluştur ve bloğu bu ortamda yürüt
    executeBlock(stmt->statements, std::make_shared<Environment>(environment));
}

void Interpreter::visitClassStmt(std::shared_ptr<ClassStmt> stmt) {
    Value superclass_value = std::monostate{};
    std::shared_ptr<CCubeClass> superclass = nullptr;

    if (stmt->superclass != nullptr) {
        superclass_value = evaluate(stmt->superclass);
        if (!std::holds_alternative<ObjPtr>(superclass_value) || std::get<ObjPtr>(superclass_value)->getType() != Object::ObjectType::CLASS) {
            throw runtimeError(stmt->superclass->name, "Üst sınıf bir sınıf olmalıdır."); // Name member is not available on ExprPtr
        }
        superclass = std::static_pointer_cast<CCubeClass>(std::get<ObjPtr>(superclass_value));
    }

    // Sınıfı ortamda tanımla (geçici olarak null)
    // Bu, sınıfın kendi içinde referans verilmesini sağlar.
    environment->define(stmt->name.lexeme, std::monostate{}); // Placeholder

    // Metotları depolamak için bir harita oluştur
    std::unordered_map<std::string, std::shared_ptr<CCubeFunction>> methods;
    for (const auto& method_stmt : stmt->methods) {
        // 'this' anahtar kelimesini ve 'super' çağrılarını doğru bağlamak için
        // fonksiyonu tanımlandığı ortamdan almalıyız.
        std::shared_ptr<CCubeFunction> function = std::make_shared<CCubeFunction>(method_stmt, environment, method_stmt->name.lexeme == "init");
        gc.addRoot(function); // GC kökü olarak ekle

        methods[method_stmt->name.lexeme] = function;
    }

    // CCubeClass objesini oluştur ve global ortama ekle (veya bulunduğu ortama)
    std::shared_ptr<CCubeClass> klass = std::make_shared<CCubeClass>(stmt->name.lexeme, superclass, methods);
    gc.addRoot(klass); // GC kökü olarak ekle

    environment->assign(stmt->name, gc.createObject(klass)); // Sınıfı ortamda ata
}

void Interpreter::visitExprStmt(std::shared_ptr<ExprStmt> stmt) {
    Value result = evaluate(stmt->expression);
    // REPL modunda ise sonucu yazdır (main.cpp'deki runRepl'den çağrıldığında)
    // Şimdilik manuel olarak print etmiyoruz, kullanıcı print() çağrısını kullanmalı.
     printValue(result); // Eğer REPL'de son ifade çıktısı isteniyorsa
}

void Interpreter::visitFunStmt(std::shared_ptr<FunStmt> stmt) {
    // Bir C-CUBE fonksiyonu oluştur
    std::shared_ptr<CCubeFunction> function = std::make_shared<CCubeFunction>(stmt, environment, false);
    gc.addRoot(function); // GC kökü olarak ekle (fonksiyonlar da GC tarafından yönetilebilir)
    environment->define(stmt->name.lexeme, gc.createObject(function)); // Fonksiyonu ortamda tanımla
}

void Interpreter::visitIfStmt(std::shared_ptr<IfStmt> stmt) {
    if (isTruthy(evaluate(stmt->condition))) {
        execute(stmt->thenBranch);
    } else if (stmt->elseBranch != nullptr) {
        execute(stmt->elseBranch);
    }
}

void Interpreter::visitImportStmt(std::shared_ptr<ImportStmt> stmt) {
    // Modülü yükle
    std::shared_ptr<CCubeModule> module = moduleLoader.loadModule(stmt->moduleName, *this);
    if (!module) {
        throw runtimeError(stmt->moduleName, "Modül '" + stmt->moduleName.lexeme + "' bulunamadı veya yüklenemedi.");
    }
    gc.addRoot(module); // Modülü GC kökü olarak ekle

    std::string import_name = stmt->alias.empty() ? stmt->moduleName.lexeme : stmt->alias;
    environment->define(import_name, gc.createObject(module));
}

void Interpreter::visitReturnStmt(std::shared_ptr<ReturnStmt> stmt) {
    Value value = std::monostate{}; // Varsayılan dönüş değeri none
    if (stmt->value != nullptr) {
        value = evaluate(stmt->value);
    }
    // ReturnException'ı fırlatarak çağrı yığınından çık
    throw ReturnException(value);
}

void Interpreter::visitVarStmt(std::shared_ptr<VarStmt> stmt) {
    Value value = std::monostate{}; // Varsayılan değer none
    if (stmt->initializer != nullptr) {
        value = evaluate(stmt->initializer); // Başlangıç değeri varsa değerlendir
    }
    environment->define(stmt->name.lexeme, value); // Ortamda değişkeni tanımla
}

void Interpreter::visitWhileStmt(std::shared_ptr<WhileStmt> stmt) {
    while (isTruthy(evaluate(stmt->condition))) {
        execute(stmt->body);
    }
}

void Interpreter::visitMatchStmt(std::shared_ptr<MatchStmt> stmt) {
    Value subject_value = evaluate(stmt->subject); // Eşleştirilecek ifadeyi değerlendir

    bool matched = false;
    for (const auto& match_case : stmt->cases) {
        if (match_case.pattern == nullptr) { // Bu bir 'default' durumu
            if (!matched) { // Eğer daha önce hiçbir desen eşleşmediyse
                execute(match_case.body);
                matched = true;
            }
            break; // Default'tan sonra başka case olmaz
        }

        // Desen (pattern) değerlendir.
        // Match desenleri özeldir ve burada runtime değerlendirme yapılır.
        // Şimdilik sadece literal desenleri ve değişken desenlerini destekleyelim.
        // Daha karmaşık desenler için bu kısım genişletilmelidir.

        if (std::dynamic_pointer_cast<LiteralExpr>(match_case.pattern)) {
            // Literal desen (örn: case 10:, case "hello":)
            Value pattern_value = evaluate(match_case.pattern);
            if (isEqual(subject_value, pattern_value)) {
                execute(match_case.body);
                matched = true;
                break; // Eşleşme bulundu, döngüden çık
            }
        } else if (std::dynamic_pointer_cast<VariableExpr>(match_case.pattern)) {
            // Değişken deseni (örn: case x:, x değişkenine subject_value atanır)
            // Bu durumda, pattern'daki değişken, o case bloğu için yeni bir scope'ta tanımlanır.
            // Bu, 'if' koşulu gibi özel bir durum içermiyorsa her zaman eşleşir.
            // Match-case içindeki değişkenler için yeni bir scope oluşturulması gerekebilir.
            // Şimdilik, eğer pattern bir VariableExpr ise, onu eşleştirme olarak kabul edip,
            // bloğu kendi alt ortamında yürüteceğiz ve değişkeni orada tanımlayacağız.
            Token var_name = std::dynamic_pointer_cast<VariableExpr>(match_case.pattern)->name;
            std::shared_ptr<Environment> case_env = std::make_shared<Environment>(environment);
            case_env->define(var_name.lexeme, subject_value); // Değişkeni case ortamında tanımla
            executeBlock(std::static_pointer_cast<BlockStmt>(match_case.body)->statements, case_env); // Bloğu yeni ortamda yürüt
            matched = true;
            break;
        }
        // TODO: Daha karmaşık desen türlerini (liste desenleri, obje desenleri, koşullu desenler) burada işle
        // Örneğin:
        
        else if (std::dynamic_pointer_cast<ListLiteralExpr>(match_case.pattern)) {
            // Liste deseni: [x, y, z] gibi
            // subject_value'nun bir liste olup olmadığını kontrol et ve desenle eşleşip eşleşmediğini kontrol et
        }
        
    }
}

// Genel REPL çıktıları için (eğer interpret REPL'de tek ifade yürütüyorsa)
void Interpreter::printValue(const Value& value) {
    std::cout << valueToString(value) << std::endl;
}
