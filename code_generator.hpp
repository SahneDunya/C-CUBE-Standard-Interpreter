#ifndef CUBE_CODEGEN_CODE_GENERATOR_HPP
#define CUBE_CODEGEN_CODE_GENERATOR_HPP

#include <vector>
#include <string>
#include <memory> // unique_ptr için
#include <unordered_map> // Sembollerden IR adreslerine eşleme için
#include <stack> // Break/Continue hedeflerini yönetmek için

// Gerekli derleyici bileşenleri
#include "Syntax/ast.hpp"              // AST düğümleri için
#include "Semantics/symbol_table.hpp"  // Sembol tablosu için
#include "Data Types/type_system.hpp"  // Tip sistemi için
#include "ErrorHandling/error_reporter.hpp" // Hata raporlama için
#include "Control Structures/control_flow_generator.hpp" // Eğer ayrı bir sınıf olsaydı

// TODO: Çalışma zamanı somut Object tiplerini ve Value tiplerini dahil etmeniz gerekebilir
// IR'daki operantlar gerçek değerler veya Object referansları olabilir.
#include "Data Types/IntegerObject.hpp"
#include "Data Types/StringObject.hpp"
#include "Data Types/BooleanObject.hpp"

namespace CCube {

// --- Basit Kavramsal Ara Temsil (IR) Tanımları (Yığın Tabanlı Makine Varsayımı) ---

// IR komut kodları
enum class OpCode {
    // Literalleri yığına yükleme
    LOAD_INT,    // Operands: int_value
    LOAD_FLOAT,  // Operands: float_value
    LOAD_STRING, // Operands: string_literal_index (Global data'da tutulur)
    LOAD_BOOL,   // Operands: bool_value (0 or 1)
    LOAD_NONE,   // Operands: none (placeholder)

    // Değişken yükleme/saklama
    LOAD_VAR,    // Operands: variable_index (Stack/Register/Heap konumu) - Yığına yükler
    STORE_VAR,   // Operands: variable_index - Yığından pop yapıp değişkene saklar

    // İkili Operatörler (Yığından 2 operand pop yap, sonucu yığına push yap)
    ADD_INT, SUB_INT, MUL_INT, DIV_INT, MOD_INT,
    ADD_FLOAT, SUB_FLOAT, MUL_FLOAT, DIV_FLOAT,
    EQ_INT, NEQ_INT, LT_INT, GT_INT, LTE_INT, GTE_INT,
    EQ_FLOAT, NEQ_FLOAT, LT_FLOAT, GT_FLOAT, LTE_FLOAT, GTE_FLOAT,
    EQ_BOOL, NEQ_BOOL, // vs.
    ADD_STRING, // String birleştirme
    AND_BOOL, OR_BOOL, // Mantıksal AND/OR (kısa devre değerlendirme IR'da dallanma ile yapılır)

    // Birli Operatörler (Yığından 1 operand pop yap, sonucu yığına push yap)
    NEG_INT, NEG_FLOAT, // Birli eksi
    NOT_BOOL, // Mantıksal NOT

    // Kontrol Akışı
    JUMP,        // Operands: target_block_index
    BRANCH_IF,   // Operands: condition (bool yığında), true_block_index, false_block_index
    LABEL,       // Operands: label_index (Bu komut BasicBlock'un başındadır)
    RETURN,      // Operands: (Yığındaki değeri döndürür veya void için operand yok)

    // Fonksiyon Çağrısı
    CALL,        // Operands: function_symbol_index, num_args (Yığından num_args kadar pop yap, sonucu yığına push yap)

    // Obje ve Üye Erişimi (Eğer IR objelerle çalışıyorsa)
    GET_FIELD,   // Operands: field_index (Objeyi yığından pop yap, alan değerini yığına push yap)
    SET_FIELD,   // Operands: field_index (Değeri ve objeyi yığından pop yap)
    INVOKE_METHOD, // CALL gibi ama metod arama (vtable lookup gerektirebilir)

    // Diğer
    POP,         // Yığının üstündekini atar
    DUP,         // Yığının üstündekini kopyalar
    // ... İhtiyaç duyulan diğer komutlar ...
};

// Operand Türleri
enum class OperandType {
    NONE, INT, FLOAT, BOOL, STRING_INDEX, VAR_INDEX, BLOCK_INDEX, SYMBOL_INDEX
};

// Operand Değeri (union bellek paylaşır, tipi takip etmek önemlidir)
union OperandValue {
    int i;
    double f; // double float için de kullanılabilir
    bool b;
    size_t index; // String, Değişken, Blok, Sembol indeksleri için
};

// Bir komutun işleneni
struct Operand {
    OperandType type;
    OperandValue value;

    // Kolay oluşturucular
    static Operand createInt(int val) { return {OperandType::INT, {.i = val}}; }
    static Operand createFloat(double val) { return {OperandType::FLOAT, {.f = val}}; }
    static Operand createBool(bool val) { return {OperandType::BOOL, {.b = val}}; }
    static Operand createStringIndex(size_t idx) { return {OperandType::STRING_INDEX, {.index = idx}}; }
    static Operand createVarIndex(size_t idx) { return {OperandType::VAR_INDEX, {.index = idx}}; }
    static Operand createBlockIndex(size_t idx) { return {OperandType::BLOCK_INDEX, {.index = idx}}; }
    static Operand createSymbolIndex(size_t idx) { return {OperandType::SYMBOL_INDEX, {.index = idx}}; }
    static Operand createNone() { return {OperandType::NONE, {}}; }

     // Kopyalama/Atama için operatörler tanımlanmalı çünkü union içerir
    Operand(const Operand& other);
    Operand& operator=(const Operand& other);
    // Yıkıcı (union basit tipler içeriyorsa varsayılan yeterli)
    ~Operand() = default;
};

// Temel Blok (Basic Block): Tek girişli, tek çıkışlı komut dizisi
struct BasicBlock {
    std::vector<Instruction> instructions;
    int id = -1; // Hata ayıklama için kimlik
    // Bağlantılar: Son komut bir dallanma ise, burası hangi bloklara atlayabileceğini gösterir.
};

// Bir komut
struct Instruction {
    OpCode opcode;
    std::vector<Operand> operands; // Komutun işlenenleri

     // Hata ayıklama için string gösterimi
    std::string toString() const;
};

// Bir fonksiyonun IR temsili
struct Function {
    std::shared_ptr<Symbol> symbol; // Fonksiyonun compile-time sembolü
    std::vector<std::unique_ptr<BasicBlock>> basic_blocks; // Fonksiyonun temel blokları
    // Parametre bilgisi (Symbol'den alınabilir)
    // Yerel değişken bilgisi
};

// Tüm derlenmiş modülün IR temsili
struct CompiledModule {
    std::vector<std::unique_ptr<Function>> functions; // Fonksiyonlar
    std::vector<std::string> string_literals; // String sabitleri havuzu
    // Global değişkenler, sabitler vb.
};


// --- CodeGenerator Tanımı ---

// AST'yi alıp Ara Temsil (IR) üreten sınıf
class CodeGenerator {
public:
    // Kurucu: AST, Sembol Tablosu, Tip Sistemi ve Hata Raporlayıcıya referans alır.
    CodeGenerator(Program& ast, SymbolTable& symbol_table, TypeSystem& type_system, ErrorReporter& reporter);

    // Yıkıcı
    ~CodeGenerator();

    // Kod üretim sürecini başlatır.
    void generate();

    // Üretilen Ara Temsili (IR) döndürür.
    const CompiledModule& getGeneratedCode() const { return generated_module_; }

    // --- Kontrol Akışı Entegrasyon Metotları (ControlFlowGenerator mantığı burada) ---
    // Bu metotlar, ControlFlowGenerator'ın önceki tasarımındaki mantığı CodeGenerator'ın IR emisyonuna bağlar.

    // Yeni bir Basic Block oluşturur ve mevcut blok olarak ayarlar.
    BasicBlock* createBasicBlock(const std::string& name_hint = "");

    // Verilen blok indexine/pointerına koşulsuz dallanma komutu ekler.
    void emitUnconditionalBranch(BasicBlock* target_block);

    // Koşullu dallanma komutu ekler. Condition ifadesinin sonucu yığının tepesinde olmalıdır.
    void emitConditionalBranch(BasicBlock* true_block, BasicBlock* false_block);

    // Mevcut Basic Block'a bir dönüş komutu ekler. Yığının tepesindeki değer döndürülür.
    void emitReturn(); // Void return için

    // Mevcut Basic Block'a bir dönüş komutu ekler (değer döndüren).
    void emitReturn(Expression& return_value_expr); // Dönüş değeri ifadesi

    // --- IR Emisyon Metotları ---
    // Basit yığın tabanlı IR komutlarını ekler.

    void emit(OpCode opcode); // İşlenensiz komutlar (POP, DUP vb.)
    void emit(OpCode opcode, Operand op1);
    void void emit(OpCode opcode, Operand op1, Operand op2);
    // ... daha fazla aşırı yükleme gerekebilir


private:
    Program& ast_;
    SymbolTable& symbol_table_;
    TypeSystem& type_system_;
    ErrorReporter& reporter_;

    CompiledModule generated_module_; // Üretilen IR

    // Kod üretimi sırasında takip edilmesi gereken durum
    Function* current_function_ = nullptr; // Şu anda üretilen fonksiyon
    BasicBlock* current_block_ = nullptr;   // Şu anda üretilen temel blok
    size_t current_variable_index_ = 0;    // Basitlik için değişkenlere indeks atama

    // Loop hedeflerini tutan yığınlar (break/continue için BasicBlock pointerları)
    std::stack<BasicBlock*> break_targets_;
    std::stack<BasicBlock*> continue_targets_;

    // String sabitlerinin metinleri -> onların compiled_module_.string_literals içindeki indeksi
    std::unordered_map<std::string, size_t> string_literal_map_;


    // AST düğümlerini gezip kod üreten ana fonksiyonlar
    void generateProgram(Program& program);
    void generateStatement(Statement& stmt);
    void generateExpression(Expression& expr); // İfade kodunu üretir ve sonucunu yığına koyar

    // Belirli deyim/ifade türleri için kod üreten yardımcı fonksiyonlar
    void generateBlockStatement(BlockStmt& stmt);
    void generateVarDeclaration(VarDeclStmt& stmt);
    void generateExpressionStatement(ExpressionStmt& stmt);
    void generateIfStatement(IfStmt& stmt);
    void generateWhileStatement(WhileStmt& stmt);
    void generateMatchStatement(MatchStmt& stmt); // Oldukça karmaşık
    void generateBreakStatement(BreakStmt& stmt);
    void generateContinueStatement(ContinueStmt& stmt);
    void generateReturnStatement(ReturnStmt& stmt);
    void generateFunctionDefinition(DefStmt& stmt);
    void generateClassDefinition(ClassDeclStmt& stmt); // Karmaşık


    void generateLiteralExpr(LiteralExpr& expr);
    void generateVariableExpr(VariableExpr& expr);
    void generateBinaryExpr(BinaryExpr& expr);
    void generateUnaryExpr(UnaryExpr& UnaryExpr);
    void generateCallExpr(CallExpr& expr);
    // TODO: generateGetExpr, generateSetExpr, generateIndexExpr vb.


    // Yardımcı fonksiyonlar
    // String literal'i havuza ekler ve indexini döndürür
    size_t addStringLiteral(const std::string& str);

    // Sembolün IR'daki karşılığını (örn: yığın offseti, global adres) belirler ve döndürür.
    // SymbolTable'dan alınan bilgi burada IR'a özgü bir konuma çevrilir.
    size_t getVariableIrIndex(std::shared_ptr<Symbol> var_symbol); // Placeholder

    // İşlem sonucunun tipine göre uygun aritmetik/mantıksal/karşılaştırma opcode'unu seçer
    OpCode getBinaryOpCode(TokenType op_token_type, Type* left_type, Type* right_type);
    OpCode getUnaryOpCode(TokenType op_token_type, Type* operand_type);

    // Bir BasicBlock pointerını onun CompileModule içindeki indexine eşler
    int getBasicBlockIndex(BasicBlock* block); // Placeholder

    // CodeGenerator'ın Singleton olmadığı varsayılmıştır. Eğer Singleton olsaydı, getInstance metodu ve özel kurucu/yıkıcı olurdu.
};

} namespace CCube

#endif // CUBE_CODEGEN_CODE_GENERATOR_HPP