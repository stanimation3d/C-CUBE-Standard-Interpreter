#ifndef C_CUBE_SCANNER_H
#define C_CUBE_SCANNER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept> // std::runtime_error için

#include "token.h"          // Token sınıfı ve TokenType enum'ı için
#include "error_reporter.h" // Hata raporlama için

// Scanner sınıfı, kaynak kodu token'lara ayırır.
class Scanner {
public:
    Scanner(const std::string& source, ErrorReporter& reporter);

    // Kaynak kodu tarar ve token listesini döndürür.
    std::vector<Token> scanTokens();

private:
    const std::string& source;
    ErrorReporter& errorReporter;
    std::vector<Token> tokens; // Oluşturulan token'lar bu vektöre eklenir.

    int start = 0;   // Şu anki token'ın başlangıç pozisyonu.
    int current = 0; // Kaynak kodundaki şu anki karakter pozisyonu.
    int line = 1;    // Şu anki satır numarası.

    // Anahtar kelimelerin haritası (örn: "if", "else", "var" vb.)
    static const std::unordered_map<std::string, TokenType> keywords;

    // Tek karakterli token'ları işler
    void scanToken();

    // Token'ı ekler
    void addToken(TokenType type);
    void addToken(TokenType type, std::variant<std::monostate, std::string, double, bool> literal);

    // İleri okuma yardımcıları
    bool isAtEnd() const;         // Kaynak kodunun sonuna ulaşıldı mı?
    char advance();               // Sonraki karakteri tüketir ve döndürür.
    bool match(char expected);    // Sonraki karakterin beklenen karakter olup olmadığını kontrol eder ve tüketir.
    char peek() const;            // Sonraki karaktere bakar, ama tüketmez.
    char peekNext() const;        // Sonraki karakterin sonraki karaktere bakar, ama tüketmez.

    // Karakter kontrol yardımcıları
    bool isDigit(char c) const;      // Karakter bir rakam mı?
    bool isAlpha(char c) const;      // Karakter bir harf mi?
    bool isAlphaNumeric(char c) const; // Karakter bir harf veya rakam mı?

    // Token işleyiciler
    void string();     // String literal'leri işler.
    void number();     // Sayı literal'leri işler.
    void identifier(); // Tanımlayıcıları (değişken isimleri, anahtar kelimeler) işler.
};

#endif // C_CUBE_SCANNER_H
