#ifndef C_CUBE_INTERPRETER_H
#define C_CUBE_INTERPRETER_H

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

// Proje bağımlılıkları
#include "ast.h"            // AST düğümleri ve Visitor arayüzleri
#include "token.h"          // Token tipleri
#include "value.h"          // C-CUBE değer tipleri
#include "environment.h"    // Ortam (scope) yönetimi
#include "error_reporter.h" // Hata raporlama ve RuntimeException
#include "callable.h"       // Çağrılabilir nesneler (fonksiyonlar, sınıflar) için temel sınıf
#include "function.h"       // Fonksiyonlar için C-CUBE sınıfı
#include "object.h"         // C-CUBE obje sınıfı
#include "class.h"          // C-CUBE sınıf sınıfı
#include "instance.h"       // CCubeInstance
#include "list.h"           // CCubeList
#include "c_cube_module.h"  // C-CUBE modül tipi
#include "module_loader.h"  // Modül yükleme mekanizması
#include "utils.h"          // Yardımcı fonksiyonlar (örn. valueToString)
#include "gc.h"             // Çöp toplayıcı (YENİ EKLEME)


// Fonksiyon dönüşlerini işlemek için özel exception
// Bu dosyanın ayrı bir 'return_exception.h' olarak tanımlandığını varsayıyoruz.
 #include "return_exception.h" // Eğer ayrı bir dosya ise

// Eğer ReturnException ayrı bir dosya değilse ve buraya dahil edeceksek:
// (Zaten error_reporter.h içinde tanımlı olduğundan tekrar tanıma gerek yok)
 class ReturnException : public std::runtime_error { /* ... */ };


class Interpreter : public ExprVisitor<Value>, public StmtVisitor<void> {
private:
    // Global ortam. Tüm programın genel değişkenlerini ve fonksiyonlarını tutar.
    std::shared_ptr<Environment> globals;
    // Mevcut yürütme ortamı. Fonksiyon çağrıları ve bloklar için değişir.
    std::shared_ptr<Environment> environment;
    ErrorReporter& errorReporter;
    Gc& gc; // Çöp toplayıcıya referans (ZATEN VARDI)
    ModuleLoader& moduleLoader; // Modül yükleyiciye referans

    // Resolver'ın ürettiği lokal değişken mesafeleri (eğer Resolver entegre edildiyse)
     std::unordered_map<const Expr*, int> locals;

    // Interpreter'ın kendi yığınındaki geçici değerleri ve değişkenleri tutmak için
    // Bu, GC'nin mark aşamasında taranması gereken ek köklerdir.
    // Ancak bu, `Gc`'nin `Interpreter`'ı doğrudan sorgulamasını gerektirir,
    // veya `Interpreter`'ın yığınındaki her şeyi `Gc`'ye bildirmesi gerekir ki bu da karmaşıktır.
    // Şimdilik, `environment` ve `globals`'ı ana kökler olarak kabul edelim ve diğer geçici değerlerin
    // kısa ömürlü olduğunu veya environment içinde bulunduğunu varsayalım.
    // Gerçek bir üretim dilinde, burası daha karmaşık bir yığın tarama mekanizması gerektirebilir.

    // Yardımcı metotlar
    Value evaluate(ExprPtr expr);
    void execute(StmtPtr stmt);
    bool isTruthy(const Value& value);
    bool isEqual(const Value& a, const Value& b);
    void checkNumberOperand(const Token& op, const Value& operand);
    void checkNumberOperands(const Token& op, const Value& left, const Value& right);

    // Çalışma zamanı hatası fırlatır
    RuntimeException runtimeError(const Token& token, const std::string& message);

    // Ortam yönetimi için özel metotlar
    void executeBlock(const std::vector<StmtPtr>& statements, std::shared_ptr<Environment> newEnvironment);

    // Değişken çözümlemesi (Şimdilik doğrudan ortamda arama yapar, Resolver yoksa)
    Value lookUpVariable(const Token& name);
    // Veya eğer Resolver varsa ve `locals` map'ini kullanıyorsa:
     Value lookUpVariable(const Token& name, ExprPtr expr); // Token'dan çözümlenen depth ile

public:
    // Constructor
    Interpreter(ErrorReporter& reporter, Gc& gc_instance, ModuleLoader& loader);

    // Programı yorumlamaya başlar
    void interpret(const std::vector<StmtPtr>& statements);

    // GC'nin kökleri tarayabilmesi için Environment'lara erişim sağlayan getter'lar
    // Bu metodlar, Gc sınıfının Interpreter'a bağlı olmasını sağlar, ideal değil.
    // Daha iyisi, Interpreter'ın GC'ye köklerini bildirmesidir.
    std::shared_ptr<Environment> getGlobalsEnvironment() const { return globals; }
    std::shared_ptr<Environment> getCurrentEnvironment() const { return environment; }


    // --- ExprVisitor Metodları (ifadeleri değerlendirme) ---
    Value visitBinaryExpr(std::shared_ptr<BinaryExpr> expr) override;
    Value visitCallExpr(std::shared_ptr<CallExpr> expr) override;
    Value visitGetExpr(std::shared_ptr<GetExpr> expr) override;
    Value visitGroupingExpr(std::shared_ptr<GroupingExpr> expr) override;
    Value visitLiteralExpr(std::shared_ptr<LiteralExpr> expr) override;
    Value visitLogicalExpr(std::shared_ptr<LogicalExpr> expr) override;
    Value visitSetExpr(std::shared_ptr<SetExpr> expr) override;
    Value visitSuperExpr(std::shared_ptr<SuperExpr> expr) override;
    Value visitThisExpr(std::shared_ptr<ThisExpr> expr) override;
    Value visitUnaryExpr(std::shared_ptr<UnaryExpr> expr) override;
    Value visitVariableExpr(std::shared_ptr<VariableExpr> expr) override;
    Value visitListLiteralExpr(std::shared_ptr<ListLiteralExpr> expr) override;

    // --- StmtVisitor Metodları (bildirimleri yürütme) ---
    void visitBlockStmt(std::shared_ptr<BlockStmt> stmt) override;
    void visitClassStmt(std::shared_ptr<ClassStmt> stmt) override;
    void visitExprStmt(std::shared_ptr<ExprStmt> stmt) override;
    void visitFunStmt(std::shared_ptr<FunStmt> stmt) override;
    void visitIfStmt(std::shared_ptr<IfStmt> stmt) override;
    void visitImportStmt(std::shared_ptr<ImportStmt> stmt) override;
    void visitReturnStmt(std::shared_ptr<ReturnStmt> stmt) override;
    void visitVarStmt(std::shared_ptr<VarStmt> stmt) override;
    void visitWhileStmt(std::shared_ptr<WhileStmt> stmt) override;
    void visitMatchStmt(std::shared_ptr<MatchStmt> stmt) override;

    void printValue(const Value& value);
};

#endif // C_CUBE_INTERPRETER_H
