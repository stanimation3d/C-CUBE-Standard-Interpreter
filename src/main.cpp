#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory> // std::shared_ptr için

// Proje bağımlılıkları
#include "scanner.h"          // Kaynak kodu taramak için
#include "parser.h"           // Tokenları AST'ye dönüştürmek için
#include "interpreter.h"      // AST'yi yorumlamak için
#include "error_reporter.h"   // Hata yönetimi için
#include "gc.h"               // Çöp toplayıcı için
#include "module_loader.h"    // Modül yükleme için
#include "builtin_functions.h" // Yerleşik fonksiyonlar için

// Global hata raporlayıcı
ErrorReporter errorReporter;

// Kaynak kodu çalıştıran ana fonksiyon
void run(const std::string& source) {
    Scanner scanner(source, errorReporter);
    std::vector<Token> tokens = scanner.scanTokens();

    if (errorReporter.hadError()) return;

    Parser parser(tokens, errorReporter);
    std::vector<StmtPtr> statements = parser.parse();

    if (errorReporter.hadError()) return;

    // ----- GC Entegrasyonu Başlangıcı -----
    // Gc nesnesini oluştur
    // youngGenCapacity ve oldGenCapacity değerlerini projenizin ihtiyacına göre ayarlayın.
    // İlk denemeler için küçük değerler kullanmak, GC'nin çalıştığını görmenizi kolaylaştırır.
    // Örneğin, 10MB ve 100MB yerine 1MB ve 10MB gibi.
    // production code'da bu değerler daha büyük olacaktır.
    Gc gc(1 * 1024 * 1024, 10 * 1024 * 1024); // Young Gen: 1MB, Old Gen: 10MB

    // Modül Yükleyiciyi oluştur
    ModuleLoader moduleLoader(errorReporter, gc); // ModuleLoader'ın da GC'ye ihtiyacı var

    // Yorumlayıcıyı oluştur ve Gc referansını ona ilet
    Interpreter interpreter(errorReporter, gc, moduleLoader);

    // Yerleşik fonksiyonları tanımla (artık Gc'yi kullanarak)
    BuiltinFunctions::defineBuiltins(interpreter.getGlobalsEnvironment(), gc);

    // GC'nin Interpreter'ın ana ortamlarına erişmesini sağlamak için (kök taraması için)
    // Gc sınıfında bu ortamları kaydetmek için bir mekanizma eklemiş olmalıyız.
    // Gc::addRoot(interpreter.getGlobalsEnvironment()); // Bu tür bir mekanizma varsayalım.
    // Alternatif olarak, Gc'nin mark aşamasında Interpreter'a danışması.
    // Şimdilik, Interpreter'ın içindeki globals ve environment'ın otomatik olarak Gc tarafından
    // (örneğin markMap aracılığıyla) taranacağını varsayıyoruz.

    // GC toplama eşiği için bir mekanizma eklenebilir.
    // Örneğin, her N statement'ta bir GC çalıştırma veya bellek tahsis eşiğine göre.
    // Şimdilik Interpreter'ın sonunda bir tam toplama yapalım.
    try {
        interpreter.interpret(statements);
    } catch (const RuntimeException& e) {
        errorReporter.runtimeError(e);
    }

    // Program bittiğinde veya çıkış yapmadan önce manuel olarak tam bir GC döngüsü çalıştır.
    // Bu, programın sonunda tüm bellek kaynaklarının temizlendiğinden emin olmanın iyi bir yoludur.
    // Normalde, GC otomatik olarak çalıştığı için bu zorunlu değildir, ancak debug ve tam temizlik için faydalıdır.
    std::cout << "\n--- Program Sonuçları ---" << std::endl;
    gc.collectFull();
    gc.printStats();
    // ----- GC Entegrasyonu Sonu -----
}

// Dosyadan kodu okuyan fonksiyon
void runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Dosya okunamadı: " << path << std::endl;
        exit(1); // Hata durumunda çıkış
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());

    if (errorReporter.hadError()) exit(65);       // Syntax error
    if (errorReporter.hadRuntimeError()) exit(70); // Runtime error
}

// Etkileşimli kabuk (REPL) fonksiyonu
void runPrompt() {
    std::string line;
    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        run(line);
        errorReporter.resetErrors(); // REPL'de her satırda hataları sıfırla
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Kullanım: c-cube [dosya]" << std::endl;
        exit(64); // Yanlış argüman sayısı
    } else if (argc == 2) {
        runFile(argv[1]); // Dosya verildi
    } else {
        runPrompt(); // Argüman verilmedi, REPL başlat
    }

    return 0;
}
