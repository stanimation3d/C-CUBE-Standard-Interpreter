#include "parser.h"
#include <iostream>   // Debugging için
#include <stdexcept>  // std::runtime_error için

// Constructor
Parser::Parser(const std::vector<Token>& tokens, ErrorReporter& reporter)
    : tokens(tokens), errorReporter(reporter) {}

// Token akışının sonuna ulaşıldı mı?
bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

// Sonraki token'ı tüketir ve döndürür
Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

// Mevcut token'ı tüketmeden döndürür
Token Parser::peek() const {
    return tokens[current];
}

// Tüketilen son token'ı döndürür
Token Parser::previous() const {
    return tokens[current - 1];
}

// Mevcut token'ın belirtilen türde olup olmadığını kontrol eder
bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

// Mevcut token'ın türü, verilen türlerden biriyle eşleşirse true döndürür ve ilerler
bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

// Belirtilen token türünü tüketir, aksi takdirde hata verir
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

// Hata raporlar ve ParseError fırlatır
Parser::ParseError Parser::error(const Token& token, const std::string& message) {
    errorReporter.error(token, message);
    return ParseError(); // Hata kurtarma için özel bir exception fırlatır
}

// Hata kurtarma mekanizması: Bir sonraki geçerli ifade başlangıcına atlar
void Parser::synchronize() {
    advance(); // Hata olan token'ı atla

    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return; // Noktalı virgülden sonra devam et

        // İfade veya bildirim başlangıç token'larını ara
        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
            case TokenType::MATCH:
            case TokenType::IMPORT:
                return; // Geçerli bir bildirim başlangıcı bulduk
            default:
                // Hiçbiri değilse, token'ı atlamaya devam et
                advance();
        }
    }
}

// --- Gramer Kurallarına Karşılık Gelen Parsing Metodları ---

// Ana parsing metodu: Token listesini alır ve bir AST döndürür
std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        try {
            statements.push_back(declaration()); // Bildirimleri parse et
        } catch (const ParseError& e) {
            synchronize(); // Hata durumunda kurtarma yap
        }
    }
    return statements;
}

// En üst seviye bildirimleri işler (var, class, fun veya normal statement)
StmtPtr Parser::declaration() {
    if (match({TokenType::VAR})) return varDeclaration();
    if (match({TokenType::CLASS})) return classDeclaration();
    if (match({TokenType::FUN})) return funDeclaration("function");
    if (match({TokenType::IMPORT})) return importStatement(); // Import bildirimi

    return statement(); // Eğer özel bir bildirim değilse, genel bir bildirimdir
}

// 'var' bildirimi: var identifier = expression;
StmtPtr Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Değişken ismi bekleniyor.");

    ExprPtr initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression(); // Başlangıç değeri varsa parse et
    }

    consume(TokenType::SEMICOLON, "Değişken bildiriminden sonra ';' bekleniyor.");
    return std::make_shared<VarStmt>(name, initializer);
}

// 'class' bildirimi: class ClassName < SuperClass { ... }
StmtPtr Parser::classDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Sınıf ismi bekleniyor.");

    ExprPtr superclass = nullptr;
    if (match({TokenType::LESS})) { // Miras alma varsa (<)
        consume(TokenType::IDENTIFIER, "Üst sınıf ismi bekleniyor.");
        superclass = std::make_shared<VariableExpr>(previous()); // Üst sınıf bir değişken ifadesidir
    }

    consume(TokenType::LEFT_BRACE, "Sınıf isminden sonra '{' bekleniyor.");

    std::vector<FunStmtPtr> methods;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        methods.push_back(std::dynamic_pointer_cast<FunStmt>(funDeclaration("method")));
    }

    consume(TokenType::RIGHT_BRACE, "Sınıf gövdesinden sonra '}' bekleniyor.");
    return std::make_shared<ClassStmt>(name, superclass, methods);
}

// 'fun' (fonksiyon/metot) bildirimi: fun name(params) { ... }
StmtPtr Parser::funDeclaration(const std::string& kind) {
    Token name = consume(TokenType::IDENTIFIER, kind + " ismi bekleniyor.");
    consume(TokenType::LEFT_PAREN, kind + " isminden sonra '(' bekleniyor.");

    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (parameters.size() >= 255) {
                error(peek(), "Fonksiyon çok fazla parametreye sahip olamaz.");
            }
            parameters.push_back(consume(TokenType::IDENTIFIER, "Parametre ismi bekleniyor."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Parametrelerden sonra ')' bekleniyor.");

    consume(TokenType::LEFT_BRACE, kind + " gövdesinden önce '{' bekleniyor.");
    std::vector<StmtPtr> body = std::dynamic_pointer_cast<BlockStmt>(blockStatement())->statements;

    return std::make_shared<FunStmt>(name, parameters, body);
}

// Genel bildirim
StmtPtr Parser::statement() {
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::LEFT_BRACE})) return blockStatement(); // Tek başına blok
    if (match({TokenType::MATCH})) return matchStatement(); // Match bildirimi

    return expressionStatement(); // Eğer özel bir bildirim değilse, ifade bildirimidir
}

// 'if' bildirimi: if (condition) { ... } else { ... }
StmtPtr Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "'if' den sonra '(' bekleniyor.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Koşuldan sonra ')' bekleniyor.");

    StmtPtr thenBranch = statement(); // 'then' bloğu
    StmtPtr elseBranch = nullptr;
    if (match({TokenType::ELSE})) {
        elseBranch = statement(); // 'else' bloğu varsa
    }

    return std::make_shared<IfStmt>(condition, thenBranch, elseBranch);
}

// 'while' bildirimi: while (condition) { ... }
StmtPtr Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "'while' den sonra '(' bekleniyor.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Koşuldan sonra ')' bekleniyor.");

    StmtPtr body = statement();

    return std::make_shared<WhileStmt>(condition, body);
}

// 'match' bildirimi: match (expression) { case pattern: statement; ... default: statement; }
StmtPtr Parser::matchStatement() {
    consume(TokenType::LEFT_PAREN, "'match' den sonra '(' bekleniyor.");
    ExprPtr subject = expression(); // Eşleştirilecek ifade
    consume(TokenType::RIGHT_PAREN, "Match ifadesinden sonra ')' bekleniyor.");
    consume(TokenType::LEFT_BRACE, "Match ifadesi için '{' bekleniyor.");

    std::vector<MatchCase> cases = parseMatchCases();

    consume(TokenType::RIGHT_BRACE, "Match gövdesinden sonra '}' bekleniyor.");
    return std::make_shared<MatchStmt>(subject, cases);
}

// Match ifadesi için case'leri çözümler
std::vector<MatchCase> Parser::parseMatchCases() {
    std::vector<MatchCase> cases;
    while (check(TokenType::CASE) || check(TokenType::DEFAULT)) {
        cases.push_back(parseMatchCase());
    }
    return cases;
}

// Tek bir match case'i çözer: case pattern: statement; veya default: statement;
MatchCase Parser::parseMatchCase() {
    bool isDefault = false;
    if (match({TokenType::DEFAULT})) {
        isDefault = true;
    } else {
        consume(TokenType::CASE, "'case' veya 'default' anahtar kelimesi bekleniyor.");
    }

    ExprPtr pattern = nullptr;
    if (!isDefault) {
        pattern = parsePattern(); // Deseni parse et
    }

    consume(TokenType::COLON, isDefault ? "Default durumundan sonra ':' bekleniyor." : "Desenden sonra ':' bekleniyor.");

    // Match case'in gövdesi bir blok veya tek bir ifade olabilir
    StmtPtr body;
    if (check(TokenType::LEFT_BRACE)) {
        body = blockStatement();
    } else {
        body = statement(); // Basit bir ifade bildirimi olabilir (e.g. print("Hello");)
    }

    return MatchCase(pattern, body);
}

// Match ifadesindeki desenleri çözümle.
// Şimdilik sadece literal desenleri (sayı, string, boolean, none) ve tanımlayıcıları (var desenleri için) destekleyeceğiz.
// Daha karmaşık desenler (listeler, objeler) için daha fazla mantık gerekir.
ExprPtr Parser::parsePattern() {
    if (match({TokenType::NUMBER, TokenType::STRING, TokenType::TRUE, TokenType::FALSE, TokenType::NONE})) {
        return std::make_shared<LiteralExpr>(previous().literal);
    }
    if (match({TokenType::IDENTIFIER})) {
        // Bu bir değişken deseni olabilir (örneğin 'case x:'), bu durumda x'i VariableExpr olarak döndürüyoruz.
        // Interpreter'ın 'match' implementasyonu bu tür desenleri özel olarak ele alacaktır.
        return std::make_shared<VariableExpr>(previous());
    }
    // TODO: Daha karmaşık desen türlerini (liste desenleri, obje desenleri, if koşullu desenler) burada ekle
    throw error(peek(), "Beklenmeyen desen tipi.");
}


// 'import' bildirimi: import module_name; veya import module_name as alias;
StmtPtr Parser::importStatement() {
    Token moduleName = consume(TokenType::IDENTIFIER, "İmport edilecek modül ismi bekleniyor.");
    Token alias; // Opsiyonel alias

    if (match({TokenType::AS})) { // 'as' anahtar kelimesi varsa
        alias = consume(TokenType::IDENTIFIER, "Modül için alias ismi bekleniyor.");
    }

    consume(TokenType::SEMICOLON, "İmport bildiriminden sonra ';' bekleniyor.");
    return std::make_shared<ImportStmt>(moduleName, alias.lexeme.empty() ? "" : alias.lexeme);
}


// 'return' bildirimi: return expression; veya return;
StmtPtr Parser::returnStatement() {
    Token keyword = previous(); // 'return' token'ı

    ExprPtr value = nullptr;
    if (!check(TokenType::SEMICOLON)) { // Eğer noktalı virgül gelmiyorsa, bir değer döndürüyor demektir
        value = expression();
    }

    consume(TokenType::SEMICOLON, "Return bildiriminden sonra ';' bekleniyor.");
    return std::make_shared<ReturnStmt>(keyword, value);
}

// Süslü parantez içindeki kod bloğu { ... }
StmtPtr Parser::blockStatement() {
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration()); // Blok içinde de bildirimler olabilir
    }
    consume(TokenType::RIGHT_BRACE, "Bloktan sonra '}' bekleniyor.");
    return std::make_shared<BlockStmt>(statements);
}

// Sadece bir ifade olan bildirim: expression;
StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON, "İfade bildiriminden sonra ';' bekleniyor.");
    return std::make_shared<ExprStmt>(expr);
}

// --- İfade (Expression) Parsing Metodları ---

// Ana ifade parsing metodu (en düşük öncelikli operatörden başlar)
ExprPtr Parser::expression() {
    return assignment();
}

// Atama: identifier = expression; veya object.property = expression;
ExprPtr Parser::assignment() {
    ExprPtr expr = logicalOr(); // Atama sol tarafı bir L-value olmalı, bu yüzden en azından bir atama öncesi ifade olmalı

    if (match({TokenType::EQUAL})) {
        Token equals = previous(); // '=' token'ı
        ExprPtr value = assignment(); // Sağ taraftaki değer (sağdan sola öncelik için recursive çağrı)

        if (std::dynamic_pointer_cast<VariableExpr>(expr)) {
            // Değişken ataması (örn: x = 10)
            Token name = std::dynamic_pointer_cast<VariableExpr>(expr)->name;
            return std::make_shared<AssignExpr>(name, value);
        } else if (std::dynamic_pointer_cast<GetExpr>(expr)) {
            // Property ataması (örn: obj.prop = 10)
            GetExprPtr get = std::dynamic_pointer_cast<GetExpr>(expr);
            return std::make_shared<SetExpr>(get->object, get->name, value);
        }

        errorReporter.error(equals, "Geçersiz atama hedefi.");
        return nullptr; // Hata durumunda null döndür veya ParseError fırlat
    }

    return expr;
}

// Mantıksal OR: expr or expr
ExprPtr Parser::logicalOr() {
    ExprPtr expr = logicalAnd();

    while (match({TokenType::OR})) {
        Token op = previous();
        ExprPtr right = logicalAnd();
        expr = std::make_shared<LogicalExpr>(expr, op, right);
    }
    return expr;
}

// Mantıksal AND: expr and expr
ExprPtr Parser::logicalAnd() {
    ExprPtr expr = equality();

    while (match({TokenType::AND})) {
        Token op = previous();
        ExprPtr right = equality();
        expr = std::make_shared<LogicalExpr>(expr, op, right);
    }
    return expr;
}

// Eşitlik: expr == expr | expr != expr
ExprPtr Parser::equality() {
    ExprPtr expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        ExprPtr right = comparison();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// Karşılaştırma: expr > expr | expr >= expr | expr < expr | expr <= expr
ExprPtr Parser::comparison() {
    ExprPtr expr = addition();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        ExprPtr right = addition();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// Toplama/Çıkarma: expr + expr | expr - expr
ExprPtr Parser::addition() {
    ExprPtr expr = multiplication();

    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        ExprPtr right = multiplication();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// Çarpma/Bölme: expr * expr | expr / expr
ExprPtr Parser::multiplication() {
    ExprPtr expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR})) {
        Token op = previous();
        ExprPtr right = unary();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// Tekli operatörler: !expr | -expr
ExprPtr Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = unary(); // Sağdan sola öncelik için recursive çağrı
        return std::make_shared<UnaryExpr>(op, right);
    }
    return call(); // Tekli operatörden sonra çağrı ifadeleri gelebilir (örn: -obj.method())
}

// Fonksiyon çağrısı, metot çağrısı, dizin erişimi
ExprPtr Parser::call() {
    ExprPtr expr = primary(); // Çağrının sol tarafı (birincil ifade veya tanımlayıcı olabilir)

    while (true) {
        if (match({TokenType::LEFT_PAREN})) { // Fonksiyon çağrısı
            std::vector<ExprPtr> arguments;
            if (!check(TokenType::RIGHT_PAREN)) { // Argümanlar boş değilse
                do {
                    if (arguments.size() >= 255) {
                        error(peek(), "Fonksiyon çok fazla argümana sahip olamaz.");
                    }
                    arguments.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            Token paren = consume(TokenType::RIGHT_PAREN, "Argümanlardan sonra ')' bekleniyor.");
            expr = std::make_shared<CallExpr>(expr, paren, arguments);
        } else if (match({TokenType::DOT})) { // Property erişimi (örn: object.property)
            Token name = consume(TokenType::IDENTIFIER, "Property ismi bekleniyor.");
            expr = std::make_shared<GetExpr>(expr, name);
        } else if (match({TokenType::LEFT_BRACKET})) { // Dizin erişimi (örn: array[index])
            ExprPtr index = expression();
            Token bracket = consume(TokenType::RIGHT_BRACKET, "Dizin erişiminden sonra ']' bekleniyor.");
            expr = std::make_shared<GetExpr>(expr, index->toString()); // Geçiçi çözüm: IndexExpr tipi oluşturulabilir
            // TODO: Dizilere doğrudan erişim için ayrı bir IndexExpr yapısı daha uygun olabilir
             return std::make_shared<IndexExpr>(expr, index);
        }
        else {
            break; // Daha fazla çağrı veya erişim yoksa döngüden çık
        }
    }
    return expr;
}

// Temel ifadeler (literaller, parantezli ifadeler, tanımlayıcılar, 'this', 'super')
ExprPtr Parser::primary() {
    if (match({TokenType::FALSE})) return std::make_shared<LiteralExpr>(false);
    if (match({TokenType::TRUE})) return std::make_shared<LiteralExpr>(true);
    if (match({TokenType::NONE})) return std::make_shared<LiteralExpr>(std::monostate{});

    if (match({TokenType::NUMBER})) return std::make_shared<LiteralExpr>(std::get<double>(previous().literal));
    if (match({TokenType::STRING})) return std::make_shared<LiteralExpr>(std::get<std::string>(previous().literal));

    if (match({TokenType::SUPER})) {
        Token keyword = previous();
        consume(TokenType::DOT, "'super' anahtar kelimesinden sonra '.' bekleniyor.");
        Token method = consume(TokenType::IDENTIFIER, "Üst sınıf metot ismi bekleniyor.");
        return std::make_shared<SuperExpr>(keyword, method);
    }
    if (match({TokenType::THIS})) return std::make_shared<ThisExpr>(previous());

    if (match({TokenType::IDENTIFIER})) return std::make_shared<VariableExpr>(previous());

    if (match({TokenType::LEFT_PAREN})) {
        ExprPtr expr = expression();
        consume(TokenType::RIGHT_PAREN, "İfadeden sonra ')' bekleniyor.");
        return std::make_shared<GroupingExpr>(expr);
    }

    // List literals: [expr, expr, ...]
    if (match({TokenType::LEFT_BRACKET})) {
        std::vector<ExprPtr> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACKET, "Liste literalinden sonra ']' bekleniyor.");
        return std::make_shared<ListLiteralExpr>(elements);
    }

    // Eğer hiçbir şey eşleşmezse, beklenmeyen bir token'dır
    throw error(peek(), "Beklenmeyen ifade.");
}
