#ifndef C_CUBE_ERROR_REPORTER_H
#define C_CUBE_ERROR_REPORTER_H

#include <string>
#include <vector>
#include <stdexcept> // std::runtime_error için

#include "token.h" // Hata token'ını referans almak için
#include "value.h" // ReturnException için Value


// Programın durdurulması gereken bir hata olduğunu belirtmek için genel bir bayrak.
// Bu, genellikle derleme (parsing) hataları için kullanılır.
class ErrorReporter {
private:
    bool hadErrorFlag = false;
    bool hadRuntimeErrorFlag = false;

public:
    void error(int line, const std::string& message) {
        report(line, "", message);
        hadErrorFlag = true;
    }

    // Token ile hata raporlama (Parsing hataları için daha detaylı)
    void error(const Token& token, const std::string& message) {
        if (token.type == TokenType::END_OF_FILE) {
            report(token.line, " at end", message);
        } else {
            report(token.line, " at '" + token.lexeme + "'", message);
        }
        hadErrorFlag = true;
    }

    bool hadError() const {
        return hadErrorFlag;
    }

    void resetErrors() {
        hadErrorFlag = false;
        hadRuntimeErrorFlag = false;
    }

    // Çalışma zamanı hataları için
    void runtimeError(const class RuntimeException& error) {
        std::cerr << "[Satır " << error.token.line << "] Çalışma Zamanı Hatası";
        if (!error.token.lexeme.empty()) {
             std::cerr << " at '" << error.token.lexeme << "'";
        }
        std::cerr << ": " << error.what() << std::endl;
        hadRuntimeErrorFlag = true;
    }

    bool hadRuntimeError() const {
        return hadRuntimeErrorFlag;
    }

private:
    void report(int line, const std::string& where, const std::string& message) {
        std::cerr << "[Satır " << line << "] Hata" << where << ": " << message << std::endl;
    }
};

// Yorumlayıcının çalışma zamanı hataları için özel istisna sınıfı
class RuntimeException : public std::runtime_error {
public:
    const Token token; // Hataya neden olan token

    RuntimeException(const Token& token, const std::string& message)
        : std::runtime_error(message), token(token) {}
};

// Fonksiyonlardan dönüş değerlerini yönetmek için özel istisna
// Bu, fonksiyonların normal akışını kesmek için kullanılır.
class ReturnException : public std::runtime_error {
public:
    Value value; // Fonksiyonun döndürdüğü değer

    ReturnException(Value value)
        : std::runtime_error("Fonksiyondan dönüş"), value(std::move(value)) {}
    // `std::move` kullanarak Value kopyalamasını optimize ediyoruz.
};


#endif // C_CUBE_ERROR_REPORTER_H
