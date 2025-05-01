#ifndef CUBE_ERROR_HANDLING_ERROR_TYPES_HPP
#define CUBE_ERROR_HANDLING_ERROR_TYPES_HPP

#include <string>

namespace CCube {

// Farklı hata ve uyarı türlerini kategorize eden enum
enum class ErrorCode {
    // Genel Hatalar
    UNKNOWN_ERROR,
    INTERNAL_ERROR, // Derleyici iç hatası

    // Lexer Hataları (1000 - 1999)
    LEXER_UNEXPECTED_CHARACTER = 1000,
    LEXER_UNTERMINATED_STRING,
    LEXER_INVALID_NUMBER,

    // Parser Hataları (2000 - 2999)
    PARSER_UNEXPECTED_TOKEN = 2000,
    PARSER_MISSING_SEMICOLON, // Eğer noktalı virgül kullanılıyorsa
    PARSER_MISSING_RPAREN,    // Eksik )
    PARSER_MISSING_RBRACE,    // Eksik }
    PARSER_MISSING_RSQUARE,   // Eksik ]
    PARSER_EXPECT_EXPRESSION,
    PARSER_EXPECT_IDENTIFIER,
    PARSER_EXPECT_COLON,      // Eksik :
    PARSER_EXPECT_INDENT,     // Eksik girinti (eğer lexer INDENT üretmiyorsa parser takip eder)
    PARSER_EXPECT_DEDENT,     // Eksik girinti azaltma

    // Semantic Analiz Hataları (3000 - 3999)
    SEMANTIC_UNDEFINED_VARIABLE = 3000,
    SEMANTIC_REDECLARATION,
    SEMANTIC_UNDEFINED_FUNCTION,
    SEMANTIC_UNDEFINED_CLASS,
    SEMANTIC_UNSUPPORTED_BINARY_OP, // İkili operatör için uyumsuz tipler
    SEMANTIC_UNSUPPORTED_UNARY_OP,  // Birli operatör için uyumsuz tip
    SEMANTIC_INVALID_ASSIGNMENT_TARGET,
    SEMANTIC_TYPE_MISMATCH,
    SEMANTIC_CALL_NON_CALLABLE,
    SEMANTIC_ARGUMENT_COUNT_MISMATCH,
    SEMANTIC_ARGUMENT_TYPE_MISMATCH,
    SEMANTIC_RETURN_OUTSIDE_FUNCTION,
    SEMANTIC_BREAK_OUTSIDE_LOOP,
    SEMANTIC_CONTINUE_OUTSIDE_LOOP,
    SEMANTIC_INVALID_RETURN_TYPE,

    // Runtime Hataları (4000 - 4999)
    RUNTIME_DIVISION_BY_ZERO = 4000,
    RUNTIME_NULL_POINTER_DEREFERENCE,
    RUNTIME_INDEX_OUT_OF_BOUNDS,
    RUNTIME_INVALID_CAST,
    RUNTIME_METHOD_NOT_FOUND,
    RUNTIME_PROPERTY_NOT_FOUND,
    // ... diğer runtime hataları

    // Uyarılar (8000 - 8999)
    WARNING_UNUSED_VARIABLE = 8000,
    WARNING_UNUSED_FUNCTION,
    WARNING_DEPRECATED_FEATURE,
    // ... diğer uyarılar
};

// ErrorCode'a karşılık gelen varsayılan mesaj şablonunu döndüren fonksiyon.
// Konum bilgisi ve özel detaylar ErrorReporter tarafından mesaja eklenecektir.
const char* getErrorMessageTemplate(ErrorCode code);

} namespace CCube

#endif // CUBE_ERROR_HANDLING_ERROR_TYPES_HPP