#include "error_reporter.hpp"
#include <sstream> // Mesaj formatlamak için

namespace CCube {

// ErrorCode'a karşılık gelen varsayılan mesaj şablonunun implementasyonu.
// Bu şablonlar daha sonra detaylarla doldurulacaktır.
const char* getErrorMessageTemplate(ErrorCode code) {
    switch (code) {
        case ErrorCode::UNKNOWN_ERROR: return "Unknown error.";
        case ErrorCode::INTERNAL_ERROR: return "Internal compiler error: %s"; // %s detay için yer tutucu

        // Lexer
        case ErrorCode::LEXER_UNEXPECTED_CHARACTER: return "Unexpected character '%s'.";
        case ErrorCode::LEXER_UNTERMINATED_STRING: return "Unterminated string literal.";
        case ErrorCode::LEXER_INVALID_NUMBER: return "Invalid number format.";

        // Parser
        case ErrorCode::PARSER_UNEXPECTED_TOKEN: return "Unexpected token '%s'. Expected %s."; // %s token lexeme, %s beklenen tür
        case ErrorCode::PARSER_MISSING_SEMICOLON: return "Missing ';' after statement.";
        case ErrorCode::PARSER_MISSING_RPAREN: return "Missing ')' after %s.";
        case ErrorCode::PARSER_MISSING_RBRACE: return "Missing '}' after %s.";
        case ErrorCode::PARSER_MISSING_RSQUARE: return "Missing ']' after %s.";
        case ErrorCode::PARSER_EXPECT_EXPRESSION: return "Expect expression.";
        case ErrorCode::PARSER_EXPECT_IDENTIFIER: return "Expect identifier.";
        case ErrorCode::PARSER_EXPECT_COLON: return "Expect ':' after %s.";
        case ErrorCode::PARSER_EXPECT_INDENT: return "Expect indented block.";
        case ErrorCode::PARSER_EXPECT_DEDENT: return "Expect dedent.";


        // Semantic Analiz
        case ErrorCode::SEMANTIC_UNDEFINED_VARIABLE: return "Undefined variable '%s'.";
        case ErrorCode::SEMANTIC_REDECLARATION: return "Redeclaration of '%s'.";
        case ErrorCode::SEMANTIC_UNDEFINED_FUNCTION: return "Undefined function '%s'.";
        case ErrorCode::SEMANTIC_UNDEFINED_CLASS: return "Undefined class '%s'.";
        case ErrorCode::SEMANTIC_UNSUPPORTED_BINARY_OP: return "Unsupported operator '%s' for operand types '%s' and '%s'."; // %s op, %s sol tip, %s sağ tip
        case ErrorCode::SEMANTIC_UNSUPPORTED_UNARY_OP: return "Unsupported unary operator '%s' for operand type '%s'."; // %s op, %s operand tip
        case ErrorCode::SEMANTIC_INVALID_ASSIGNMENT_TARGET: return "Invalid assignment target.";
        case ErrorCode::SEMANTIC_TYPE_MISMATCH: return "Type mismatch: cannot convert from '%s' to '%s'."; // %s kaynak, %s hedef
        case ErrorCode::SEMANTIC_CALL_NON_CALLABLE: return "Expression is not callable.";
        case ErrorCode::SEMANTIC_ARGUMENT_COUNT_MISMATCH: return "Function '%s' expects %d arguments, but %d provided."; // %s func adı, %d beklenen, %d verilen
        case ErrorCode::SEMANTIC_ARGUMENT_TYPE_MISMATCH: return "Argument %d type mismatch: expected '%s', got '%s'."; // %d arg index, %s beklenen, %s verilen
        case ErrorCode::SEMANTIC_RETURN_OUTSIDE_FUNCTION: return "'return' statement outside of function body.";
        case ErrorCode::SEMANTIC_BREAK_OUTSIDE_LOOP: return "'break' statement outside of loop body.";
        case ErrorCode::SEMANTIC_CONTINUE_OUTSIDE_LOOP: return "'continue' statement outside of loop body.";
        case ErrorCode::SEMANTIC_INVALID_RETURN_TYPE: return "Cannot convert return value type '%s' to function return type '%s'.";


        // Runtime
        case ErrorCode::RUNTIME_DIVISION_BY_ZERO: return "Division by zero.";
        case ErrorCode::RUNTIME_NULL_POINTER_DEREFERENCE: return "Null pointer dereference.";
        case ErrorCode::RUNTIME_INDEX_OUT_OF_BOUNDS: return "Index out of bounds.";
        case ErrorCode::RUNTIME_INVALID_CAST: return "Invalid type cast from '%s' to '%s'.";
        case ErrorCode::RUNTIME_METHOD_NOT_FOUND: return "Method '%s' not found on object of type '%s'.";
        case ErrorCode::RUNTIME_PROPERTY_NOT_FOUND: return "Property '%s' not found on object of type '%s'.";


        // Uyarılar
        case ErrorCode::WARNING_UNUSED_VARIABLE: return "Unused variable '%s'.";
        case ErrorCode::WARNING_UNUSED_FUNCTION: return "Unused function '%s'.";
        case ErrorCode::WARNING_DEPRECATED_FEATURE: return "Deprecated feature used: %s";

        default: return "Unhandled error code."; // Bilinmeyen ErrorCode geldiğinde
    }
}


// ErrorReporter Kurucu
ErrorReporter::ErrorReporter()
    : has_errors_(false) {
    // Kurucu içinde özel bir başlatma gerekmiyor
}

// Hata raporla
void ErrorReporter::reportError(ErrorCode code, SourceLocation loc, const std::string& details) {
    has_errors_ = true; // Hata oluştuğunu işaretle

    // Hata mesajını formatla (varsayılan şablon + detaylar)
    std::stringstream ss;
    ss << getErrorMessageTemplate(code);
    // Eğer detaylar varsa şablon içindeki %s'lere yerleştirilebilir.
    // Şu an basitlik için şablonun sonuna detayları ekleyelim veya detay mesaj olarak kullanılsın.
    // Daha gelişmiş implementasyonda, şablon string içindeki format belirteçleri (%s, %d)
    // details stringindeki veya ayrı ayrı verilen argümanlardaki değerlerle değiştirilmelidir.
    std::string final_message = ss.str();
     if (!details.empty()) {
         // Basit: detayı mesaj sonuna ekle veya şablonun içine uygun şekilde yerleştir.
         // Çok basit bir örnek: Eğer şablonda %s varsa detayı oraya koymaya çalış.
         size_t pos = final_message.find("%s");
         if (pos != std::string::npos && !details.empty()) {
             final_message.replace(pos, 2, details);
         } else if (pos != std::string::npos && details.empty()) {
              final_message.replace(pos, 2, "[no details]"); // %s var ama detay yok
         }
         // Daha fazla format belirteci (%d vb.) için daha karmaşık işleme gerekir.
          // Eğer şablonda %s yoksa ve detay varsa, detayı sonuna ekleyebiliriz.
          if (pos == std::string::npos && !details.empty()) {
               final_message += " (" + details + ")"; // Detayları parantez içinde ekle
          }

     }


    // Hatayı listeye ekle
    errors_.emplace_back(code, loc, final_message, false); // Error struct'ın kurucusunu çağır

    // Hata ayıklama sırasında hemen konsola yazdır (isteğe bağlı)
    std::cerr << "Error [" << loc.line << ":" << loc.column << "]: " << final_message << std::endl;
}

// Uyarı raporla
void ErrorReporter::reportWarning(ErrorCode code, SourceLocation loc, const std::string& details) {
    // Uyarı mesajını formatla
     std::stringstream ss;
    ss << getErrorMessageTemplate(code);
    std::string final_message = ss.str();
     if (!details.empty()) {
         size_t pos = final_message.find("%s");
         if (pos != std::string::npos && !details.empty()) {
             final_message.replace(pos, 2, details);
         } else if (pos != std::string::npos && details.empty()) {
             final_message.replace(pos, 2, "[no details]");
         }
           if (pos == std::string::npos && !details.empty()) {
              final_message += " (" + details + ")";
           }
     }


    // Uyarıyı listeye ekle
    errors_.emplace_back(code, loc, final_message, true); // Error struct'ın kurucusunu çağır

    // Hata ayıklama sırasında hemen konsola yazdır (isteğe bağlı)
    std::cerr << "Warning [" << loc.line << ":" << loc.column << "]: " << final_message << std::endl;
}

// Hata olup olmadığını kontrol et
bool ErrorReporter::hasErrors() const {
    return has_errors_;
}

// Raporlanan hata ve uyarı listesini döndür
const std::vector<Error>& ErrorReporter::getErrors() const {
    return errors_;
}

// Raporlanan tüm hata ve uyarıları belirtilen akışa yazdırır
void ErrorReporter::printReports(std::ostream& os) const {
    for (const auto& err : errors_) {
        os << (err.is_warning ? "Warning" : "Error")
           << " [" << err.location.line << ":" << err.location.column << "]: "
           << err.message << std::endl;
    }
}


} namespace CCube