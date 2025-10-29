#ifndef C_CUBE_LEXER_H
#define C_CUBE_LEXER_H

#include <string>
#include <vector>
#include <unordered_map> // Anahtar kelimeler için
#include <memory>        // Eğer Token'lar shared_ptr olsaydı
#include "token.h"       // Token sınıfı için
#include "error_reporter.h" // Hata raporlayıcı için

// İleri bildirimler (gerekirse)
 class Interpreter; // Lexer, Interpreter'ı doğrudan kullanmaz, ancak hataları ErrorReporter ile raporlar

class Lexer {
private:
    const std::string& source; // Taranacak kaynak kod
    std::vector<Token> tokens; // Oluşturulan token'ların listesi
    ErrorReporter& errorReporter; // Hata raporlama sistemi

    int start = 0;   // Geçerli token'ın başlangıç indeksi
    int current = 0; // Kaynak kodundaki mevcut karakterin indeksi
    int line = 1;    // Mevcut satır numarası

    // Anahtar kelimeler haritası (static, tüm Lexer instance'ları arasında paylaşılır)
    static const std::unordered_map<std::string, TokenType> keywords;

    // Yardımcı metodlar
    bool isAtEnd() const;          // Kaynak kodunun sonuna ulaşıldı mı?
    char advance();                // Sonraki karakteri tüketir ve döndürür
    void addToken(TokenType type); // Token'ı literal değeri olmadan ekler
    void addToken(TokenType type, LiteralType literal); // Token'ı literal değeriyle ekler

    bool match(char expected);     // Sonraki karakterin beklenen karakterle eşleşip eşleşmediğini kontrol eder
    char peek() const;             // Mevcut karakteri tüketmeden döndürür
    char peekNext() const;         // Mevcut karakterden bir sonraki karakteri tüketmeden döndürür

    bool isAlpha(char c) const;    // Karakter bir harf mi?
    bool isDigit(char c) const;    // Karakter bir rakam mı?
    bool isAlphaNumeric(char c) const; // Karakter harf veya rakam mı?

    void skipWhitespace();         // Boşlukları ve yorumları atlar
    void scanString();             // String literal'i tarar
    void scanNumber();             // Sayı literal'i tarar
    void scanIdentifier();         // Tanımlayıcı veya anahtar kelimeyi tarar

    void scanToken();              // Tek bir token'ı tarar

public:
    Lexer(const std::string& source, ErrorReporter& reporter);

    // Kaynak kodu tarar ve token'ların listesini döndürür
    std::vector<Token> scanTokens();
};

#endif // C_CUBE_LEXER_H
