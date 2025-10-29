#include "error_reporter.h"

// Constructor
ErrorReporter::ErrorReporter() : hadError_(false), hadRuntimeError_(false) {}

// Yardımcı metod: Hata mesajını formatlar ve basar
void ErrorReporter::report(int line, const std::string& where, const std::string& message) {
    std::cerr << "[Satır " << line << "] Hata" << where << ": " << message << std::endl;
    hadError_ = true; // Genel hata bayrağını ayarla
}

// Tarama ve çözümleme hataları için (Lexer/Parser)
void ErrorReporter::error(int line, const std::string& message) {
    report(line, "", message); // Belirli bir token olmadan genel satır hatası
}

void ErrorReporter::error(const Token& token, const std::string& message) {
    if (token.type == TokenType::EOF_TOKEN) {
        report(token.line, " sonunda", message); // Dosya sonunda hata
    } else {
        report(token.line, " '" + token.lexeme + "' üzerinde", message); // Belirli bir token üzerinde hata
    }
}

// Çalışma zamanı hataları için (Interpreter)
void ErrorReporter::runtimeError(const RuntimeException& error_obj) {
    std::cerr << "[Satır " << error_obj.token.line << "] Çalışma Zamanı Hatası: " << error_obj.message << std::endl;
    hadRuntimeError_ = true; // Çalışma zamanı hata bayrağını ayarla
}

// Hata durumunu sorgulamak için
bool ErrorReporter::hasError() const {
    return hadError_;
}

bool ErrorReporter::hasRuntimeError() const {
    return hadRuntimeError_;
}

// Hata durumunu sıfırlamak için (özellikle REPL'de)
void ErrorReporter::reset() {
    hadError_ = false;
    hadRuntimeError_ = false;
}
