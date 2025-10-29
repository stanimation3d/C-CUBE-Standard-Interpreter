#include "scanner.h"
#include <iostream> // Debug çıktısı için kullanılabilir, normalde kaldırılır
#include <variant>

// Anahtar kelimelerin statik haritası
const std::unordered_map<std::string, TokenType> Scanner::keywords = {
    {"and", TokenType::AND},
    {"class", TokenType::CLASS},
    {"else", TokenType::ELSE},
    {"false", TokenType::FALSE},
    {"fun", TokenType::FUN},
    {"for", TokenType::FOR},
    {"if", TokenType::IF},
    {"none", TokenType::NONE},
    {"or", TokenType::OR},
    {"print", TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"true", TokenType::TRUE},
    {"var", TokenType::VAR},
    {"while", TokenType::WHILE},
    {"import", TokenType::IMPORT},
    {"as", TokenType::AS},
    {"match", TokenType::MATCH},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT}
};

Scanner::Scanner(const std::string& source, ErrorReporter& reporter)
    : source(source), errorReporter(reporter) {}

// Kaynak kodu tarar ve token listesini döndürür.
std::vector<Token> Scanner::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }

    // Dosya sonu token'ını ekle
    tokens.emplace_back(TokenType::END_OF_FILE, "", std::monostate{}, line);
    return tokens;
}

// Tek karakterli token'ları ve basit durumları işler
void Scanner::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '[': addToken(TokenType::LEFT_BRACKET); break; // Yeni
        case ']': addToken(TokenType::RIGHT_BRACKET); break; // Yeni
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '-': addToken(TokenType::MINUS); break;
        case '+': addToken(TokenType::PLUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case ':': addToken(TokenType::COLON); break; // Yeni

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

        case '/':
            if (match('/')) {
                // Yorum satırı: satır sonuna kadar ilerle
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::SLASH);
            }
            break;

        // Boşluk karakterlerini atla
        case ' ':
        case '\r':
        case '\t':
            break;

        case '\n':
            line++; // Yeni satır
            break;

        case '"': string(); break; // String literal'leri

        default:
            if (isDigit(c)) {
                number(); // Sayı literal'leri
            } else if (isAlpha(c)) {
                identifier(); // Tanımlayıcılar veya anahtar kelimeler
            } else {
                // Tanımlanamayan karakter hatası
                errorReporter.error(line, "Beklenmedik karakter.");
            }
            break;
    }
}

// Token'ı sadece tipiyle ekler
void Scanner::addToken(TokenType type) {
    addToken(type, std::monostate{}); // Literal değeri yok
}

// Token'ı tipi ve literal değeriyle ekler
void Scanner::addToken(TokenType type, std::variant<std::monostate, std::string, double, bool> literal) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, literal, line);
}

// Kaynak kodunun sonuna ulaşıldı mı?
bool Scanner::isAtEnd() const {
    return current >= source.length();
}

// Sonraki karakteri tüketir ve döndürür.
char Scanner::advance() {
    return source[current++];
}

// Sonraki karakterin beklenen karakter olup olmadığını kontrol eder ve tüketir.
bool Scanner::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;

    current++;
    return true;
}

// Sonraki karaktere bakar, ama tüketmez.
char Scanner::peek() const {
    if (isAtEnd()) return '\0'; // Boş karakter
    return source[current];
}

// Sonraki karakterin sonraki karaktere bakar, ama tüketmez.
char Scanner::peekNext() const {
    if (current + 1 >= source.length()) return '\0'; // Boş karakter
    return source[current + 1];
}

// Karakter bir rakam mı?
bool Scanner::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

// Karakter bir harf mi? (Alfabetik veya alt çizgi)
bool Scanner::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

// Karakter bir harf veya rakam mı?
bool Scanner::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

// String literal'leri işler.
void Scanner::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++; // String içinde yeni satır olamaz (Lox dilinde)
        advance();
    }

    if (isAtEnd()) {
        errorReporter.error(line, "Tanımlanmamış string.");
        return;
    }

    advance(); // Kapanış tırnak işaretini tüketir.

    // Tırnak işaretleri arasındaki string değerini al
    std::string value = source.substr(start + 1, current - 2 - start);
    addToken(TokenType::STRING, value);
}

// Sayı literal'leri işler.
void Scanner::number() {
    while (isDigit(peek())) advance();

    // Ondalık kısım var mı?
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // '.' tüketir
        while (isDigit(peek())) advance();
    }

    addToken(TokenType::NUMBER, std::stod(source.substr(start, current - start)));
}

// Tanımlayıcıları (değişken isimleri, anahtar kelimeler) işler.
void Scanner::identifier() {
    while (isAlphaNumeric(peek())) advance();

    // Tanımlayıcının metinsel değerini al
    std::string text = source.substr(start, current - start);

    // Anahtar kelime mi yoksa özel bir tanımlayıcı mı kontrol et
    TokenType type;
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        type = it->second; // Anahtar kelime
    } else {
        type = TokenType::IDENTIFIER; // Normal tanımlayıcı
    }
    addToken(type);
}
