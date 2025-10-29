#include "lexer.h"
#include <iostream> // Debugging için
#include <algorithm> // isdigit, isalpha gibi fonksiyonlar için
#include <stdexcept> // std::runtime_error için

// Statik anahtar kelime haritasının başlatılması
const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"and", TokenType::AND},
    {"class", TokenType::CLASS},
    {"else", TokenType::ELSE},
    {"fun", TokenType::FUN},
    {"if", TokenType::IF},
    {"none", TokenType::NONE},
    {"or", TokenType::OR},
    {"return", TokenType::RETURN},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"var", TokenType::VAR},
    {"while", TokenType::WHILE},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"match", TokenType::MATCH},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT},
    {"import", TokenType::IMPORT}
};

// Constructor
Lexer::Lexer(const std::string& source, ErrorReporter& reporter)
    : source(source), errorReporter(reporter) {}

// Kaynak kodunun sonuna ulaşıldı mı?
bool Lexer::isAtEnd() const {
    return current >= source.length();
}

// Sonraki karakteri tüketir ve döndürür
char Lexer::advance() {
    return source[current++];
}

// Token'ı literal değeri olmadan ekler
void Lexer::addToken(TokenType type) {
    addToken(type, std::monostate{});
}

// Token'ı literal değeriyle ekler
void Lexer::addToken(TokenType type, LiteralType literal) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, literal, line);
}

// Sonraki karakterin beklenen karakterle eşleşip eşleşmediğini kontrol eder
// Eğer eşleşirse, karakteri tüketir (advance) ve true döndürür.
bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;

    current++;
    return true;
}

// Mevcut karakteri tüketmeden döndürür (ileriye bak)
char Lexer::peek() const {
    if (isAtEnd()) return '\0'; // Null karakter ile dosya sonunu işaret et
    return source[current];
}

// Mevcut karakterden bir sonraki karakteri tüketmeden döndürür (daha da ileriye bak)
char Lexer::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

// Karakter bir harf mi? (Alfabetik veya alt çizgi)
bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

// Karakter bir rakam mı?
bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

// Karakter harf veya rakam mı?
bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

// Boşlukları ve yorumları atlar
void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(); // Boşluk karakterlerini atla
                break;
            case '\n':
                line++; // Yeni satıra geçildiğinde satır numarasını artır
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    // Tek satırlık yorum: Satır sonuna kadar atla
                    while (peek() != '\n' && !isAtEnd()) {
                        advance();
                    }
                } else if (peekNext() == '*') {
                    // Çok satırlık yorum: */ görene kadar atla
                    advance(); // Tüket '/'
                    advance(); // Tüket '*'
                    while (!(peek() == '*' && peekNext() == '/') && !isAtEnd()) {
                        if (peek() == '\n') line++;
                        advance();
                    }
                    if (!isAtEnd()) { // */ karakterlerini tüket
                        advance();
                        advance();
                    } else {
                        errorReporter.error(line, "Beklenmeyen dosya sonu: Çok satırlı yorum kapatılmadı.");
                    }
                } else {
                    // Normal bölme operatörü, atlamıyoruz
                    return;
                }
                break;
            default:
                return; // Boşluk veya yorum değilse çık
        }
    }
}

// String literal'i tarar
void Lexer::scanString() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++; // String içinde yeni satır
        advance();
    }

    if (isAtEnd()) {
        errorReporter.error(line, "Kapatılmamış string.");
        return;
    }

    advance(); // Kapanış tırnağını (") tüket

    // Tırnakları çıkararak string değerini al
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
}

// Sayı literal'i tarar
void Lexer::scanNumber() {
    while (isDigit(peek())) {
        advance();
    }

    // Ondalık kısım var mı?
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // Noktayı tüket
        while (isDigit(peek())) {
            advance();
        }
    }

    try {
        double value = std::stod(source.substr(start, current - start));
        addToken(TokenType::NUMBER, value);
    } catch (const std::out_of_range& e) {
        errorReporter.error(line, "Çok büyük veya çok küçük sayı: " + source.substr(start, current - start));
    } catch (const std::invalid_argument& e) {
        errorReporter.error(line, "Geçersiz sayı formatı: " + source.substr(start, current - start));
    }
}

// Tanımlayıcı veya anahtar kelimeyi tarar
void Lexer::scanIdentifier() {
    while (isAlphaNumeric(peek())) {
        advance();
    }

    std::string text = source.substr(start, current - start);
    TokenType type = TokenType::IDENTIFIER;

    // Anahtar kelime mi diye kontrol et
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        type = it->second; // Anahtar kelime ise ilgili TokenType'ı kullan
    }

    addToken(type);
}

// Tek bir token'ı tarar
void Lexer::scanToken() {
    char c = advance(); // Mevcut karakteri al ve ilerle
    switch (c) {
        // Tek karakterli token'lar
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '[': addToken(TokenType::LEFT_BRACKET); break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '-': addToken(TokenType::MINUS); break;
        case '+': addToken(TokenType::PLUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case '/':
            // Yorum mu yoksa bölme mi? skipWhitespace() zaten yorumları halletmiş olmalı.
            // Eğer buraya gelindiyse, tek bir '/' dir.
            addToken(TokenType::SLASH);
            break;

        // Bir veya iki karakterli token'lar
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;

        // Literaller
        case '"': scanString(); break;

        // Boşluklar ve yorumlar (skipWhitespace() burada tekrar çağrılır, ancak zaten ana döngüde hallediliyor)
        case ' ':
        case '\r':
        case '\t':
            // Bu karakterler, scanToken çağrılmadan önce skipWhitespace tarafından atlanmalıdır.
            // Eğer buraya ulaşıldıysa, muhtemelen bir hata vardır veya beklenmeyen bir durumdur.
            // Bu switch bloğuna girmeden önce skipWhitespace çağrıldığı için bu case'ler gereksizdir.
            // Ancak, güvenli olmak adına burada bırakılabilir veya kaldırılabilir.
            break;
        case '\n':
            // Yeni satırlar da skipWhitespace tarafından halledilir.
            line++;
            break;

        default:
            if (isDigit(c)) {
                scanNumber();
            } else if (isAlpha(c)) {
                scanIdentifier();
            } else {
                errorReporter.error(line, "Beklenmeyen karakter: '" + std::string(1, c) + "'");
            }
            break;
    }
}

// Kaynak kodu tarar ve token'ların listesini döndürür
std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current; // Her yeni token'ın başlangıcını ayarla

        skipWhitespace(); // Her token'dan önce boşlukları ve yorumları atla
        if (isAtEnd()) break; // Boşlukları atlarken dosya sonuna ulaşılmış olabilir

        scanToken(); // Bir token'ı tara
    }

    // Dosya sonu token'ını ekle
    tokens.emplace_back(TokenType::EOF_TOKEN, "", std::monostate{}, line);
    return tokens;
}
