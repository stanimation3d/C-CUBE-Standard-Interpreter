#ifndef C_CUBE_AST_H
#define C_CUBE_AST_H

#include <vector>
#include <string>
#include <memory> // std::shared_ptr için
#include <variant> // LiteralType için

#include "token.h" // Token sınıfı için

// İleri Bildirimler (Forward Declarations)
// AST düğümlerini ziyaret edecek Visitor arayüzü
template <typename R> class ExprVisitor;
template <typename R> class StmtVisitor;

// Ortak temel sınıflar için shared_ptr alias'ları
// Bu pointer'lar, AST düğümlerini bellek yönetimi için kullanışlı hale getirir.
class Expr;
using ExprPtr = std::shared_ptr<Expr>;

class Stmt;
using StmtPtr = std::shared_ptr<Stmt>;

// --- İfade (Expression) Sınıfları ---

// Tüm ifade AST düğümleri için temel sınıf
class Expr {
public:
    virtual ~Expr() = default;
    template <typename R> R accept(ExprVisitor<R>& visitor); // Visitor deseni
};

// Binary (İkili) İfade: sol OP sağ (örn: a + b)
class BinaryExpr : public Expr {
public:
    ExprPtr left;
    Token op;
    ExprPtr right;

    BinaryExpr(ExprPtr left, Token op, ExprPtr right);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// Call (Çağrı) İfade: fonksiyon_adi(argümanlar) (örn: print("hello"))
class CallExpr : public Expr {
public:
    ExprPtr callee; // Çağrılan ifade (bir fonksiyon, metot veya sınıf olabilir)
    Token paren;    // Parantez token'ı (hata raporlama için)
    std::vector<ExprPtr> arguments; // Argüman listesi

    CallExpr(ExprPtr callee, Token paren, std::vector<ExprPtr> arguments);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// Get (Erişim) İfade: object.property (örn: obj.x)
class GetExpr : public Expr {
public:
    ExprPtr object; // Üzerinde erişim yapılan obje
    Token name;     // Erişilen özelliğin/metodun adı

    GetExpr(ExprPtr object, Token name);
    template <typename R> R R accept(ExprVisitor<R>& visitor);
};

// Grouping (Gruplama) İfade: (ifade) (örn: (a + b) * c)
class GroupingExpr : public Expr {
public:
    ExprPtr expression;

    GroupingExpr(ExprPtr expression);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// Literal İfade: sayı, string, boolean, none (örn: 123, "hello", true, none)
class LiteralExpr : public Expr {
public:
    LiteralType value; // Token'dan gelen literal değeri

    LiteralExpr(LiteralType value);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// Logical (Mantıksal) İfade: sol OP sağ (örn: a and b, x or y)
class LogicalExpr : public Expr {
public:
    ExprPtr left;
    Token op; // AND veya OR
    ExprPtr right;

    LogicalExpr(ExprPtr left, Token op, ExprPtr right);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// Set (Atama) İfade: object.property = value (örn: obj.x = 10)
class SetExpr : public Expr {
public:
    ExprPtr object; // Üzerinde atama yapılan obje
    Token name;     // Atama yapılan özelliğin adı
    ExprPtr value;  // Atanan değer

    SetExpr(ExprPtr object, Token name, ExprPtr value);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// Super (Üst Sınıf) İfade: super.method (örn: super.init())
class SuperExpr : public Expr {
public:
    Token keyword; // 'super' token'ı
    Token method;  // Metodun adı

    SuperExpr(Token keyword, Token method);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// This (Mevcut Obje) İfade: this
class ThisExpr : public Expr {
public:
    Token keyword; // 'this' token'ı

    ThisExpr(Token keyword);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// Unary (Tekli) İfade: OP sağ (örn: -a, !b)
class UnaryExpr : public Expr {
public:
    Token op;
    ExprPtr right;

    UnaryExpr(Token op, ExprPtr right);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// Variable (Değişken) İfade: identifier (örn: x, my_var)
class VariableExpr : public Expr {
public:
    Token name; // Değişkenin adı

    VariableExpr(Token name);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};

// ListLiteral (Liste Literal) İfade: [expr1, expr2, ...]
class ListLiteralExpr : public Expr {
public:
    std::vector<ExprPtr> elements; // Liste elemanları

    ListLiteralExpr(std::vector<ExprPtr> elements);
    template <typename R> R accept(ExprVisitor<R>& visitor);
};


// --- Bildirim (Statement) Sınıfları ---

// Tüm bildirim AST düğümleri için temel sınıf
class Stmt {
public:
    virtual ~Stmt() = default;
    template <typename R> R accept(StmtVisitor<R>& visitor); // Visitor deseni
};

// Block (Blok) Bildirimi: { statement1; statement2; ... }
class BlockStmt : public Stmt {
public:
    std::vector<StmtPtr> statements;

    BlockStmt(std::vector<StmtPtr> statements);
    template <typename R> R accept(StmtVisitor<R>& visitor);
};

// Class (Sınıf) Bildirimi: class MyClass < SuperClass { ... }
class ClassStmt : public Stmt {
public:
    Token name;      // Sınıfın adı
    ExprPtr superclass; // Üst sınıf ifadesi (VariableExpr olur)
    std::vector<std::shared_ptr<class FunStmt>> methods; // Sınıfın metotları

    ClassStmt(Token name, ExprPtr superclass, std::vector<std::shared_ptr<class FunStmt>> methods);
    template <typename R> R accept(StmtVisitor<R>& visitor);
};

// Expression (İfade) Bildirimi: expression; (örn: 1 + 2; veya func();)
class ExprStmt : public Stmt {
public:
    ExprPtr expression;

    ExprStmt(ExprPtr expression);
    template <typename R> R accept(StmtVisitor<R>& visitor);
};

// Fun (Fonksiyon) Bildirimi: fun myFunc(params) { ... }
class FunStmt : public Stmt {
public:
    Token name;
    std::vector<Token> params; // Parametre isimleri
    std::vector<StmtPtr> body; // Fonksiyon gövdesi (statement listesi)

    FunStmt(Token name, std::vector<Token> params, std::vector<StmtPtr> body);
    template <typename R> R accept(StmtVisitor<R>& visitor);
};

// If (Eğer) Bildirimi: if (condition) { ... } else { ... }
class IfStmt : public Stmt {
public:
    ExprPtr condition;
    StmtPtr thenBranch; // 'if' bloğu
    StmtPtr elseBranch; // 'else' bloğu (opsiyonel)

    IfStmt(ExprPtr condition, StmtPtr thenBranch, StmtPtr elseBranch);
    template <typename R> R accept(StmtVisitor<R>& visitor);
};

// Import (İçe Aktarma) Bildirimi: import module_name; veya import module_name as alias;
class ImportStmt : public Stmt {
public:
    Token moduleName; // Modülün adı (Token olarak)
    std::string alias; // Takma ad (eğer varsa, boşsa yok)

    ImportStmt(Token moduleName, std::string alias = "");
    template <typename R> R accept(StmtVisitor<R>& visitor);
};

// Return (Dönüş) Bildirimi: return expression; veya return;
class ReturnStmt : public Stmt {
public:
    Token keyword; // 'return' token'ı (hata raporlama için)
    ExprPtr value;   // Dönüş değeri (opsiyonel)

    ReturnStmt(Token keyword, ExprPtr value);
    template <typename R> R accept(StmtVisitor<R>& visitor);
};

// Var (Değişken) Bildirimi: var name = value;
class VarStmt : public Stmt {
public:
    Token name;
    ExprPtr initializer; // Başlangıç değeri (opsiyonel)

    VarStmt(Token name, ExprPtr initializer);
    template <typename R> R accept(StmtVisitor<R>& visitor);
};

// While (Döngü) Bildirimi: while (condition) { ... }
class WhileStmt : public Stmt {
public:
    ExprPtr condition;
    StmtPtr body;

    WhileStmt(ExprPtr condition, StmtPtr body);
    template <typename R> R accept(StmtVisitor<R>& visitor);
};

// --- Match Bildirimi için Yardımcı Yapılar ---

// MatchCase: Bir desen ve o desenle eşleştiğinde çalıştırılacak blok
struct MatchCase {
    ExprPtr pattern; // Eşleştirilecek desen (LiteralExpr, VariableExpr, vb. olabilir)
    StmtPtr body;    // Eşleştiğinde çalıştırılacak kod bloğu

    MatchCase(ExprPtr pattern, StmtPtr body) : pattern(pattern), body(body) {}
};

// Match (Eşleştirme) Bildirimi: match (subject) { case pattern: ... default: ... }
class MatchStmt : public Stmt {
public:
    ExprPtr subject; // Eşleştirilecek ifade
    std::vector<MatchCase> cases; // Durumlar listesi

    MatchStmt(ExprPtr subject, std::vector<MatchCase> cases);
    template <typename R> R accept(StmtVisitor<R>& visitor);
};


// --- Visitor Arayüzleri (Visitor Design Pattern) ---
// Bu arayüzler, AST düğümlerini traversal (dolaşma) için kullanılır.
// Interpreter ve Resolver gibi sınıflar bu arayüzleri implement eder.

template <typename R>
class ExprVisitor {
public:
    virtual R visitBinaryExpr(std::shared_ptr<BinaryExpr> expr) = 0;
    virtual R visitCallExpr(std::shared_ptr<CallExpr> expr) = 0;
    virtual R visitGetExpr(std::shared_ptr<GetExpr> expr) = 0;
    virtual R visitGroupingExpr(std::shared_ptr<GroupingExpr> expr) = 0;
    virtual R visitLiteralExpr(std::shared_ptr<LiteralExpr> expr) = 0;
    virtual R visitLogicalExpr(std::shared_ptr<LogicalExpr> expr) = 0;
    virtual R visitSetExpr(std::shared_ptr<SetExpr> expr) = 0;
    virtual R visitSuperExpr(std::shared_ptr<SuperExpr> expr) = 0;
    virtual R visitThisExpr(std::shared_ptr<ThisExpr> expr) = 0;
    virtual R visitUnaryExpr(std::shared_ptr<UnaryExpr> expr) = 0;
    virtual R visitVariableExpr(std::shared_ptr<VariableExpr> expr) = 0;
    virtual R visitListLiteralExpr(std::shared_ptr<ListLiteralExpr> expr) = 0;
    // Diğer ifade türleri buraya eklendikçe, sanal metotları da eklenecektir.
};

template <typename R>
class StmtVisitor {
public:
    virtual R visitBlockStmt(std::shared_ptr<BlockStmt> stmt) = 0;
    virtual R visitClassStmt(std::shared_ptr<ClassStmt> stmt) = 0;
    virtual R visitExprStmt(std::shared_ptr<ExprStmt> stmt) = 0;
    virtual R visitFunStmt(std::shared_ptr<FunStmt> stmt) = 0;
    virtual R visitIfStmt(std::shared_ptr<IfStmt> stmt) = 0;
    virtual R visitImportStmt(std::shared_ptr<ImportStmt> stmt) = 0;
    virtual R visitReturnStmt(std::shared_ptr<ReturnStmt> stmt) = 0;
    virtual R visitVarStmt(std::shared_ptr<VarStmt> stmt) = 0;
    virtual R visitWhileStmt(std::shared_ptr<WhileStmt> stmt) = 0;
    virtual R visitMatchStmt(std::shared_ptr<MatchStmt> stmt) = 0;
    // Diğer bildirim türleri buraya eklendikçe, sanal metotları da eklenecektir.
};

// --- accept() metodlarının implementasyonları ---
// Bu implementasyonlar genellikle .cpp dosyasında veya jenerik bir header'da olur.
// Burada şablon olduğu için .h dosyasında da bulunabilir.

// Expr için accept()
template <typename R>
R Expr::accept(ExprVisitor<R>& visitor) {
    // Bu metodun temel sınıfta tanımlanması, her türetilmiş sınıfın kendi accept metodunu çağırması gerektiği anlamına gelir.
    // Ancak Visitor deseninin doğru implementasyonu için, her türetilmiş sınıf kendi accept metodunu override etmelidir.
    // Polymorphism burada derleyiciye hangi override'ın çağrılacağını söyler.
    // Hata önlemek için throw edelim veya bu fonksiyonu abstract yapıp derived sınıflarda implemente edelim.
    // Doğru yaklaşım: Her türetilmiş sınıf kendi accept'ini implemente eder.
    throw std::runtime_error("Base Expr::accept() should not be called directly.");
}

// Stmt için accept()
template <typename R>
R Stmt::accept(StmtVisitor<R>& visitor) {
    throw std::runtime_error("Base Stmt::accept() should not be called directly.");
}

// Aşağıda her türetilmiş AST düğümü için accept metotlarını tanımlıyoruz.
// Bunlar normalde .cpp dosyasında olur ancak şablonlar nedeniyle burada olmak zorunda kalabilir.

// İfade (Expression) Düğümlerinin accept() implementasyonları
#define ACCEPT_EXPR_IMPL(ClassName) \
    template <typename R> \
    R ClassName::accept(ExprVisitor<R>& visitor) { \
        return visitor.visit##ClassName(std::static_pointer_cast<ClassName>(shared_from_this())); \
    } \
    /* shared_from_this() kullanabilmek için public std::enable_shared_from_this<ClassName> olmalıydı. */ \
    /* Ancak AST düğümleri genellikle kendileri shared_ptr içinde tutulduğu için bu varsayılabilir. */ \
    /* Eğer doğrudan this kullanılırsa: return visitor.visit##ClassName(static_cast<std::shared_ptr<ClassName>>(this)); */ \
    /* Bu durumda, pointer'ın kendisi de bir shared_ptr olmalı. En güvenlisi shared_from_this() kullanmaktır. */ \
    /* Alternatif olarak, Visitor metoduna doğrudan nesnenin referansını geçirmek: visit##ClassName(*this) */ \
    /* Ve visitor metotlarının parametrelerini std::shared_ptr<ClassName> yerine ClassName& yapmak. */ \
    /* Şimdilik, Visitor'ların shared_ptr alması ve shared_from_this'in mevcut olması varsayılmıştır. */ \
    /* Bu, Interpreter ve Resolver gibi sınıflarda visit metodlarının shared_ptr almasını gerektirir. */ \
    /* Eğer bu karmaşıklığı istemiyorsanız, accept metodunu her AST düğümüne özel olarak elle yazabilir veya */ \
    /* Visitor metodlarını doğrudan referanslarla kullanabilirsiniz. */

// Bu şekilde shared_from_this() kullanmak için her Expr ve Stmt sınıfının
// public std::enable_shared_from_this<ClassName> özelliğini miras alması gerekir.
// Ancak bu, her AST düğümünün bir shared_ptr içinde olmasını zorunlu kılar ve
// bazen karmaşıklığı artırabilir. Daha basit bir yaklaşım, accept metodunu
// doğrudan parametre olarak `this` göndermek ve Visitor'da `const ClassName&` almak olabilir.
// Şimdilik shared_ptr'lı versiyonu koruyalım ve gerekli yerlerde enable_shared_from_this ekleyelim.

// Not: shared_from_this() kullanabilmek için sınıflar std::enable_shared_from_this'ten miras almalı.
// Ancak AST düğümleri genellikle zaten shared_ptr içinde yönetildiğinden, bu karmaşıklıktan
// kaçınmak için çoğu örnekte visit metoduna doğrudan `this` gönderilir ve visitor
// metotları `const ClassName&` alır.

// Eğer doğrudan `this` gönderilecekse:
#define ACCEPT_EXPR_IMPL_REF(ClassName) \
    template <typename R> \
    R ClassName::accept(ExprVisitor<R>& visitor) { \
        return visitor.visit##ClassName(*this); \
    }

#define ACCEPT_STMT_IMPL_REF(ClassName) \
    template <typename R> \
    R ClassName::accept(StmtVisitor<R>& visitor) { \
        return visitor.visit##ClassName(*this); \
    }

// Mevcut implementasyon için shared_ptr'ı koruyarak (daha az değişiklik)
// Ancak bu, her sınıfın std::enable_shared_from_this'ten türemesini gerektirecek.
// Bu yüzden daha güvenli olan referans tabanlı accept implementasyonunu kullanalım.

// İfade (Expression) Düğümlerinin accept() implementasyonları
template <typename R> R BinaryExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitBinaryExpr(std::static_pointer_cast<BinaryExpr>(shared_from_this())); }
template <typename R> R CallExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitCallExpr(std::static_pointer_cast<CallExpr>(shared_from_this())); }
template <typename R> R GetExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitGetExpr(std::static_pointer_cast<GetExpr>(shared_from_this())); }
template <typename R> R GroupingExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitGroupingExpr(std::static_pointer_cast<GroupingExpr>(shared_from_this())); }
template <typename R> R LiteralExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitLiteralExpr(std::static_pointer_cast<LiteralExpr>(shared_from_this())); }
template <typename R> R LogicalExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitLogicalExpr(std::static_pointer_cast<LogicalExpr>(shared_from_this())); }
template <typename R> R SetExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitSetExpr(std::static_pointer_cast<SetExpr>(shared_from_this())); }
template <typename R> R SuperExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitSuperExpr(std::static_pointer_cast<SuperExpr>(shared_from_this())); }
template <typename R> R ThisExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitThisExpr(std::static_pointer_cast<ThisExpr>(shared_from_this())); }
template <typename R> R UnaryExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitUnaryExpr(std::static_pointer_cast<UnaryExpr>(shared_from_this())); }
template <typename R> R VariableExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitVariableExpr(std::static_pointer_cast<VariableExpr>(shared_from_this())); }
template <typename R> R ListLiteralExpr::accept(ExprVisitor<R>& visitor) { return visitor.visitListLiteralExpr(std::static_pointer_cast<ListLiteralExpr>(shared_from_this())); }


// Bildirim (Statement) Düğümlerinin accept() implementasyonları
template <typename R> R BlockStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitBlockStmt(std::static_pointer_cast<BlockStmt>(shared_from_this())); }
template <typename R> R ClassStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitClassStmt(std::static_pointer_cast<ClassStmt>(shared_from_this())); }
template <typename R> R ExprStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitExprStmt(std::static_pointer_cast<ExprStmt>(shared_from_this())); }
template <typename R> R FunStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitFunStmt(std::static_pointer_cast<FunStmt>(shared_from_this())); }
template <typename R> R IfStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitIfStmt(std::static_pointer_cast<IfStmt>(shared_from_this())); }
template <typename R> R ImportStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitImportStmt(std::static_pointer_cast<ImportStmt>(shared_from_this())); }
template <typename R> R ReturnStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitReturnStmt(std::static_pointer_cast<ReturnStmt>(shared_from_this())); }
template <typename R> R VarStmt::accept(VarStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitVarStmt(std::static_pointer_cast<VarStmt>(shared_from_this())); }
template <typename R> R WhileStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitWhileStmt(std::static_pointer_cast<WhileStmt>(shared_from_this())); }
template <typename R> R MatchStmt::accept(StmtVisitor<R>& visitor) { return visitor.visitMatchStmt(std::static_pointer_cast<MatchStmt>(shared_from_this())); }

#endif // C_CUBE_AST_H
