#ifndef C_CUBE_PARSER_H
#define C_CUBE_PARSER_H

#include <vector>
#include <string>
#include <memory> // std::shared_ptr için

#include "token.h"        // Token sınıfı için
#include "ast.h"          // AST düğümleri (Stmt, Expr) için
#include "error_reporter.h" // Hata raporlama için

// İleri bildirimler (gerekirse)
 class Interpreter; // Parser, Interpreter'ı doğrudan kullanmaz

class Parser {
private:
    const std::vector<Token>& tokens; // Lexer'dan gelen token'ların listesi
    ErrorReporter& errorReporter;    // Hata raporlama sistemi
    int current = 0;                 // Mevcut token'ın indeksi

    // Hata kurtarma için özel exception
    struct ParseError : public std::runtime_error {
        ParseError() : std::runtime_error("Parser Error") {}
    };

    // Yardımcı Metodlar
    bool isAtEnd() const;          // Token akışının sonuna ulaşıldı mı?
    Token advance();               // Sonraki token'ı tüketir ve döndürür
    Token peek() const;            // Mevcut token'ı tüketmeden döndürür
    Token previous() const;        // Tüketilen son token'ı döndürür

    bool check(TokenType type) const; // Mevcut token'ın belirtilen türde olup olmadığını kontrol eder
    bool match(const std::vector<TokenType>& types); // Mevcut token'ın türü, verilen türlerden biriyle eşleşirse true döndürür ve ilerler
    Token consume(TokenType type, const std::string& message); // Belirtilen token türünü tüketir, aksi takdirde hata verir

    ParseError error(const Token& token, const std::string& message); // Hata raporlar ve ParseError fırlatır
    void synchronize();            // Hata kurtarma mekanizması: Bir sonraki geçerli ifade başlangıcına atlar

    // Gramer Kurallarına Karşılık Gelen Parsing Metodları
    // En üst seviye parse metodu: Tüm programı (bildirim listesi) çözümler
    std::vector<StmtPtr> program();

    // Bildirim (Statement) Parsing Metodları
    StmtPtr declaration();
    StmtPtr varDeclaration();         // 'var' bildirimi
    StmtPtr classDeclaration();       // 'class' bildirimi
    StmtPtr funDeclaration(const std::string& kind); // 'fun' (fonksiyon/metot) bildirimi
    StmtPtr statement();              // Genel bildirim
    StmtPtr ifStatement();            // 'if' bildirimi
    StmtPtr whileStatement();         // 'while' bildirimi
    StmtPtr matchStatement();         // 'match' bildirimi
    StmtPtr importStatement();        // 'import' bildirimi
    StmtPtr returnStatement();        // 'return' bildirimi
    StmtPtr blockStatement();         // Süslü parantez içindeki blok
    StmtPtr expressionStatement();    // Sadece bir ifade olan bildirim

    // İfade (Expression) Parsing Metodları (öncelik sırasına göre)
    ExprPtr expression();
    ExprPtr assignment();             // Atama
    ExprPtr logicalOr();              // Mantıksal OR
    ExprPtr logicalAnd();             // Mantıksal AND
    ExprPtr equality();               // Eşitlik (==, !=)
    ExprPtr comparison();             // Karşılaştırma (>, >=, <, <=)
    ExprPtr addition();               // Toplama/Çıkarma (+, -)
    ExprPtr multiplication();         // Çarpma/Bölme (*, /)
    ExprPtr unary();                  // Tekli operatörler (!, -)
    ExprPtr call();                   // Fonksiyon çağrısı, metot çağrısı, dizin erişimi
    ExprPtr primary();                // Temel ifadeler (literaller, parantezli ifadeler, tanımlayıcılar)

    // Match statement'ı için özel yardımcı metodlar
    std::vector<MatchCase> parseMatchCases();
    MatchCase parseMatchCase();
    ExprPtr parsePattern(); // Match ifadesindeki desenleri çözümle

public:
    Parser(const std::vector<Token>& tokens, ErrorReporter& reporter);

    // Ana parsing metodu: Token listesini alır ve bir AST döndürür
    std::vector<StmtPtr> parse();
};

#endif // C_CUBE_PARSER_H
