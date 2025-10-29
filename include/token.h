#ifndef C_CUBE_TOKEN_H
#define C_CUBE_TOKEN_H

#include <string>
#include <variant>   // std::variant için
#include <monostate> // std::monostate için (none değeri)

// Token Tipleri
enum class TokenType {
    // Tek karakterli tokenlar
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
    LEFT_BRACKET, RIGHT_BRACKET, // Listeler için
    COLON, // Match statement için

    // Bir veya iki karakterli tokenlar
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literaller
    IDENTIFIER, STRING, NUMBER,

    // Anahtar kelimeler
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NONE, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    IMPORT, AS, MATCH, CASE, DEFAULT,

    END_OF_FILE
};

// Token sınıfı
class Token {
public:
    TokenType type;
    std::string lexeme; // Kaynak kodundaki orijinal metin (örn: "var", "foo", "123")
    std::variant<std::monostate, std::string, double, bool> literal; // Literal değer (stringler, sayılar, bool'lar)
    int line; // Token'ın kaynak kodundaki satır numarası

    // Constructor
    Token(TokenType type, std::string lexeme, std::variant<std::monostate, std::string, double, bool> literal, int line)
        : type(type), lexeme(std::move(lexeme)), literal(std::move(literal)), line(line) {}
    // `std::move` kullanarak string kopyalamalarını optimize ediyoruz.

    // Debugging ve hata mesajları için string temsili
    std::string toString() const {
        std::string lit_str;
        if (std::holds_alternative<std::monostate>(literal)) {
            lit_str = "none";
        } else if (std::holds_alternative<std::string>(literal)) {
            lit_str = std::get<std::string>(literal);
        } else if (std::holds_alternative<double>(literal)) {
            lit_str = std::to_string(std::get<double>(literal));
        } else if (std::holds_alternative<bool>(literal)) {
            lit_str = std::get<bool>(literal) ? "true" : "false";
        }

        // TokenType'ı string'e çevirmek için yardımcı bir fonksiyon veya switch kullanabiliriz.
        // Basitlik için burada sadece lexeme ve literal'ı kullanıyorum.
        // Daha eksiksiz bir `toString` için `token_type_to_string` gibi bir fonksiyon gerekebilir.
        return "Type: " + std::to_string(static_cast<int>(type)) +
               ", Lexeme: '" + lexeme +
               "', Literal: '" + lit_str + "'";
    }
};

#endif // C_CUBE_TOKEN_H
