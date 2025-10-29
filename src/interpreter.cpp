#include "interpreter.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <chrono>

// Built-in fonksiyonlar için ileri bildirim (eğer BuiltinFunctions içinde değillerse)
 static Value clockNative(Interpreter& interpreter, const std::vector<Value>& arguments) {
     return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
 }

// Constructor
Interpreter::Interpreter(ErrorReporter& reporter, Gc& gc_instance, ModuleLoader& loader)
    : globals(std::make_shared<Environment>()), environment(globals),
      errorReporter(reporter), gc(gc_instance), moduleLoader(loader) {
    // Yerleşik fonksiyonları global ortama ekle
    BuiltinFunctions::defineBuiltins(globals, gc); // BuiltinFunctions'ın da Gc'yi kullanması sağlanmalı
}

// Programı yorumlamaya başlar
void Interpreter::interpret(const std::vector<StmtPtr>& statements) {
    try {
        // GC'ye global ve mevcut ortamları kök olarak bildir.
        // Bu, GC'nin mark aşamasında bu ortamları ve içlerindeki objeleri tarayabilmesi için önemlidir.
        // Environment'lar doğrudan ObjPtr olmadığı için, Gc::addRoot(Value* val) metodunu
        // doğrudan Environment::values içindeki ObjPtr'lara uygulayabilmek gerekir.
        // Veya Gc'nin Environment'ları özel olarak taramasını sağlamak gerekir.
        // Geçici çözüm: Gc::markMap(env->getValues()) benzeri bir mekanizma (Gc'nin Interpreter'ı çağırması)
        // en temiz çözüm, Interpreter'ın Gc'ye "benim köklerim şunlar" demesidir.
        // Bunun için Gc'ye yeni bir metod ekleyip, Interpreter'dan Environment'ları vermemiz gerekir.
        // Veya Gc'ye bir "scanRoots" callback'i geçirebiliriz.

        // Şimdilik, Interpreter'ın Environment'larını doğrudan Gc'nin roots'una eklemek yerine,
        // Gc'nin mark aşamasında Interpreter'ın getGlobalsEnvironment() ve getCurrentEnvironment() metodlarını çağırarak
        // ilgili Environment'ların içindeki ObjPtr'ları tarayacağını varsayalım.
        // (Bu, Gc'nin Interpreter'a bağımlı olmasını gerektirir, ideal değildir. Daha iyi çözüm daha sonra.)

        for (const auto& stmt : statements) {
            execute(stmt);
        }
    } catch (const RuntimeException& e) {
        errorReporter.runtimeError(e);
    } catch (const ReturnException& e) {
        // main'de yakalanmalı, burada değil.
        // Genellikle fonksiyon dışından return fırlatılmaz.
        errorReporter.runtimeError(RuntimeException(Token(TokenType::RETURN, "return", std::monostate{}, -1), "Top-level return."));
    }
}

Value Interpreter::evaluate(ExprPtr expr) {
    return expr->accept(*this);
}

void Interpreter::execute(StmtPtr stmt) {
    stmt->accept(*this);
}

bool Interpreter::isTruthy(const Value& value) {
    if (std::holds_alternative<std::monostate>(value)) return false; // none is false
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
    if (std::holds_alternative<double>(value)) return std::get<double>(value) != 0.0;
    if (std::holds_alternative<std::string>(value)) return !std::get<std::string>(value).empty();
    // Diğer tüm objeler (fonksiyonlar, sınıflar, objeler, listeler, modüller) true'dur.
    return true;
}

bool Interpreter::isEqual(const Value& a, const Value& b) {
    if (a.index() != b.index()) return false;

    if (std::holds_alternative<std::monostate>(a)) {
        return true;
    } else if (std::holds_alternative<bool>(a)) {
        return std::get<bool>(a) == std::get<bool>(b);
    } else if (std::holds_alternative<double>(a)) {
        return std::get<double>(a) == std::get<double>(b);
    } else if (std::holds_alternative<std::string>(a)) {
        return std::get<std::string>(a) == std::get<std::string>(b);
    } else if (std::holds_alternative<ObjPtr>(a)) {
        return std::get<ObjPtr>(a) == std::get<ObjPtr>(b);
    }
    return false;
}

void Interpreter::checkNumberOperand(const Token& op, const Value& operand) {
    if (std::holds_alternative<double>(operand)) return;
    throw runtimeError(op, "Operand bir sayı olmalıdır.");
}

void Interpreter::checkNumberOperands(const Token& op, const Value& left, const Value& right) {
    if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) return;
    throw runtimeError(op, "Operanlar sayı olmalıdır.");
}

RuntimeException Interpreter::runtimeError(const Token& token, const std::string& message) {
    return RuntimeException(token, message);
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& statements, std::shared_ptr<Environment> newEnvironment) {
    std::shared_ptr<Environment> previousEnvironment = this->environment;
    this->environment = newEnvironment;
    // Gc'ye yeni ortamı root olarak bildirmek (eğer Environment'lar ObjPtr yönetiyorsa)
    // Şu anki Environment tasarımı ObjPtr tutmuyor, Value tutuyor.
    // Bu nedenle, Environment'ın içindeki ObjPtr'ları GC'nin mark aşamasında taraması gerekecek.
    // Eğer Environment'ın kendisi de ObjPtr olsaydı, burada Gc::addRoot(environment_obj) yapılırdı.

    try {
        for (const auto& stmt : statements) {
            execute(stmt);
        }
    } catch (...) {
        this->environment = previousEnvironment;
        throw;
    }
    this->environment = previousEnvironment;
}

Value Interpreter::lookUpVariable(const Token& name) {
    if (environment->contains(name.lexeme)) {
        return environment->get(name);
    } else if (globals->contains(name.lexeme)) {
        return globals->get(name);
    }
    throw runtimeError(name, "Tanımlanmamış değişken '" + name.lexeme + "'.");
}

// --- ExprVisitor Metotlarının Implementasyonları ---

Value Interpreter::visitBinaryExpr(std::shared_ptr<BinaryExpr> expr) {
    // ... (Öncekiyle aynı, ObjPtr oluşturmuyor)
    Value left = evaluate(expr->left);
    Value right = evaluate(expr->right);

    switch (expr->op.type) {
        case TokenType::MINUS:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) - std::get<double>(right);
        case TokenType::SLASH:
            checkNumberOperands(expr->op, left, right);
            if (std::get<double>(right) == 0.0) {
                throw runtimeError(expr->op, "Sıfıra bölme hatası.");
            }
            return std::get<double>(left) / std::get<double>(right);
        case TokenType::STAR:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) * std::get<double>(right);
        case TokenType::PLUS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return std::get<double>(left) + std::get<double>(right);
            }
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                return std::get<std::string>(left) + std::get<std::string>(right);
            }
            throw runtimeError(expr->op, "Operanlar sayılar veya stringler olmalıdır.");
        case TokenType::GREATER:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) > std::get<double>(right);
        case TokenType::GREATER_EQUAL:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) >= std::get<double>(right);
        case TokenType::LESS:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) < std::get<double>(right);
        case TokenType::LESS_EQUAL:
            checkNumberOperands(expr->op, left, right);
            return std::get<double>(left) <= std::get<double>(right);
        case TokenType::BANG_EQUAL: return !isEqual(left, right);
        case TokenType::EQUAL_EQUAL: return isEqual(left, right);
        default: break;
    }
    return std::monostate{};
}

Value Interpreter::visitCallExpr(std::shared_ptr<CallExpr> expr) {
    Value callee = evaluate(expr->callee);

    std::vector<Value> arguments;
    for (const auto& arg : expr->arguments) {
        arguments.push_back(evaluate(arg));
    }

    if (!std::holds_alternative<ObjPtr>(callee)) {
        throw runtimeError(expr->paren, "Sadece fonksiyonlar ve sınıflar çağrılabilir.");
    }

    ObjPtr obj_callee = std::get<ObjPtr>(callee);
    if (!obj_callee->isCallable()) {
        throw runtimeError(expr->paren, "Sadece fonksiyonlar ve sınıflar çağrılabilir.");
    }

    Callable* callable = static_cast<Callable*>(obj_callee.get());

    if (arguments.size() != callable->arity()) {
        throw runtimeError(expr->paren, "Beklenen " + std::to_string(callable->arity()) +
                                        " argüman, ancak " + std::to_string(arguments.size()) + " geldi.");
    }

    return callable->call(*this, arguments);
}

Value Interpreter::visitGetExpr(std::shared_ptr<GetExpr> expr) {
    Value object = evaluate(expr->object);

    if (std::holds_alternative<ObjPtr>(object)) {
        ObjPtr instance = std::get<ObjPtr>(object);
        if (instance->getType() == Object::ObjectType::INSTANCE) {
            auto ccube_instance = std::static_pointer_cast<CCubeInstance>(instance);
            Value result = ccube_instance->get(expr->name);
            if (std::holds_alternative<ObjPtr>(result) &&
                std::static_pointer_cast<CCubeFunction>(std::get<ObjPtr>(result))) {
                // Metodu objeye bağla ve Gc aracılığıyla oluştur
                return gc.createObject(std::make_shared<BoundMethod>(ccube_instance, std::static_pointer_cast<CCubeFunction>(std::get<ObjPtr>(result))));
            }
            return result;
        } else if (instance->getType() == Object::ObjectType::C_CUBE_MODULE) {
            auto module = std::static_pointer_cast<CCubeModule>(instance);
            return module->getMember(expr->name);
        }
    }
    throw runtimeError(expr->name, "Sadece objeler, modüller veya sınıflar property'lere sahip olabilir.");
}

Value Interpreter::visitGroupingExpr(std::shared_ptr<GroupingExpr> expr) {
    return evaluate(expr->expression);
}

Value Interpreter::visitLiteralExpr(std::shared_ptr<LiteralExpr> expr) {
    return expr->value;
}

Value Interpreter::visitLogicalExpr(std::shared_ptr<LogicalExpr> expr) {
    // ... (Öncekiyle aynı, ObjPtr oluşturmuyor)
    Value left = evaluate(expr->left);

    if (expr->op.type == TokenType::OR) {
        if (isTruthy(left)) return left;
    } else { // AND
        if (!isTruthy(left)) return left;
    }
    return evaluate(expr->right);
}

Value Interpreter::visitSetExpr(std::shared_ptr<SetExpr> expr) {
    Value object = evaluate(expr->object);

    if (!std::holds_alternative<ObjPtr>(object) || std::get<ObjPtr>(object)->getType() != Object::ObjectType::INSTANCE) {
        throw runtimeError(expr->name, "Sadece objelerin property'leri atanabilir.");
    }

    Value value = evaluate(expr->value);
    std::static_pointer_cast<CCubeInstance>(std::get<ObjPtr>(object))->set(expr->name, value);
    return value;
}

Value Interpreter::visitSuperExpr(std::shared_ptr<SuperExpr> expr) {
    // 'this' değişkenini ortamdan al
    // Not: Resolver bu token'ı correct depth'e bind etmeliydi.
    // Şimdilik, ortamda arama yaparak 'this'i bulmaya çalışalım.
    Value this_value = environment->get(Token(TokenType::THIS, "this", std::monostate{}, expr->keyword.line));
    if (!std::holds_alternative<ObjPtr>(this_value) || std::get<ObjPtr>(this_value)->getType() != Object::ObjectType::INSTANCE) {
        throw runtimeError(expr->keyword, "'super' anahtar kelimesi sadece metot içinde kullanılabilir.");
    }
    std::shared_ptr<CCubeInstance> instance = std::static_pointer_cast<CCubeInstance>(std::get<ObjPtr>(this_value));

    // Üst sınıfı al
    std::shared_ptr<CCubeClass> superclass = instance->get_class()->superclass;

    if (superclass == nullptr) {
        throw runtimeError(expr->keyword, "Üst sınıfı olmayan bir objenin 'super' metodu çağrılamaz.");
    }

    // Metodu üst sınıftan bul
    std::shared_ptr<CCubeFunction> method = superclass->findMethod(expr->method.lexeme);

    if (method == nullptr) {
        throw runtimeError(expr->method, "Tanımlanmamış üst sınıf metodu '" + expr->method.lexeme + "'.");
    }

    // Metodu mevcut instance'a bağla ve Gc aracılığıyla döndür
    return gc.createObject(std::make_shared<BoundMethod>(instance, method));
}

Value Interpreter::visitThisExpr(std::shared_ptr<ThisExpr> expr) {
    return lookUpVariable(expr->keyword); // 'this' bir değişkendir
}

Value Interpreter::visitUnaryExpr(std::shared_ptr<UnaryExpr> expr) {
    // ... (Öncekiyle aynı, ObjPtr oluşturmuyor)
    Value right = evaluate(expr->right);

    switch (expr->op.type) {
        case TokenType::BANG: return !isTruthy(right);
        case TokenType::MINUS:
            checkNumberOperand(expr->op, right);
            return -std::get<double>(right);
        default: break;
    }
    return std::monostate{};
}

Value Interpreter::visitVariableExpr(std::shared_ptr<VariableExpr> expr) {
    return lookUpVariable(expr->name);
}

Value Interpreter::visitListLiteralExpr(std::shared_ptr<ListLiteralExpr> expr) {
    std::vector<Value> elements;
    for (const auto& elem_expr : expr->elements) {
        elements.push_back(evaluate(elem_expr));
    }
    // GC tarafından yönetilen bir liste objesi oluştur
    return gc.createList(elements);
}


// --- StmtVisitor Metotlarının Implementasyonları ---

void Interpreter::visitBlockStmt(std::shared_ptr<BlockStmt> stmt) {
    executeBlock(stmt->statements, std::make_shared<Environment>(environment));
}

void Interpreter::visitClassStmt(std::shared_ptr<ClassStmt> stmt) {
    Value superclass_value = std::monostate{};
    std::shared_ptr<CCubeClass> superclass = nullptr;

    if (stmt->superclass != nullptr) {
        superclass_value = evaluate(stmt->superclass);
        if (!std::holds_alternative<ObjPtr>(superclass_value) || std::get<ObjPtr>(superclass_value)->getType() != Object::ObjectType::CLASS) {
            // Hata token'ı: superclass ifadesinin kendisi değil, superclass isminin token'ı olmalı.
            // Bu, 'superclass' bir VariableExpr ise, onun token'ını almak gerekir.
            // Şimdilik genel bir hata token'ı kullanıyoruz.
            throw runtimeError(stmt->name, "Üst sınıf bir sınıf olmalıdır."); // Sınıfın adı token'ını kullan
        }
        superclass = std::static_pointer_cast<CCubeClass>(std::get<ObjPtr>(superclass_value));
    }

    environment->define(stmt->name.lexeme, std::monostate{}); // Placeholder

    std::unordered_map<std::string, std::shared_ptr<CCubeFunction>> methods;
    for (const auto& method_stmt : stmt->methods) {
        // Fonksiyonu Gc aracılığıyla oluştur
        std::shared_ptr<CCubeFunction> function = std::make_shared<CCubeFunction>(method_stmt, environment, method_stmt->name.lexeme == "init");
        methods[method_stmt->name.lexeme] = function;
    }

    // CCubeClass objesini Gc aracılığıyla oluştur ve global ortama ekle
    std::shared_ptr<CCubeClass> klass = std::make_shared<CCubeClass>(stmt->name.lexeme, superclass, methods);
    environment->assign(stmt->name, gc.createObject(klass)); // Sınıfı ortamda ata
}

void Interpreter::visitExprStmt(std::shared_ptr<ExprStmt> stmt) {
    evaluate(stmt->expression);
}

void Interpreter::visitFunStmt(std::shared_ptr<FunStmt> stmt) {
    // Fonksiyonu Gc aracılığıyla oluştur
    std::shared_ptr<CCubeFunction> function = std::make_shared<CCubeFunction>(stmt, environment, false);
    environment->define(stmt->name.lexeme, gc.createObject(function));
}

void Interpreter::visitIfStmt(std::shared_ptr<IfStmt> stmt) {
    if (isTruthy(evaluate(stmt->condition))) {
        execute(stmt->thenBranch);
    } else if (stmt->elseBranch != nullptr) {
        execute(stmt->elseBranch);
    }
}

void Interpreter::visitImportStmt(std::shared_ptr<ImportStmt> stmt) {
    // Modülü yükle (ModuleLoader'ın Gc'yi kullanması gerekir)
    std::shared_ptr<CCubeModule> module = moduleLoader.loadModule(stmt->moduleName, *this);
    if (!module) {
        throw runtimeError(stmt->moduleName, "Modül '" + stmt->moduleName.lexeme + "' bulunamadı veya yüklenemedi.");
    }

    std::string import_name = stmt->alias.empty() ? stmt->moduleName.lexeme : stmt->alias;
    environment->define(import_name, gc.createObject(module));
}

void Interpreter::visitReturnStmt(std::shared_ptr<ReturnStmt> stmt) {
    Value value = std::monostate{};
    if (stmt->value != nullptr) {
        value = evaluate(stmt->value);
    }
    throw ReturnException(value);
}

void Interpreter::visitVarStmt(std::shared_ptr<VarStmt> stmt) {
    Value value = std::monostate{};
    if (stmt->initializer != nullptr) {
        value = evaluate(stmt->initializer);
    }
    environment->define(stmt->name.lexeme, value);
}

void Interpreter::visitWhileStmt(std::shared_ptr<WhileStmt> stmt) {
    while (isTruthy(evaluate(stmt->condition))) {
        execute(stmt->body);
    }
}

void Interpreter::visitMatchStmt(std::shared_ptr<MatchStmt> stmt) {
    Value subject_value = evaluate(stmt->subject);

    bool matched = false;
    for (const auto& match_case : stmt->cases) {
        if (match_case.pattern == nullptr) { // 'default' durumu
            if (!matched) {
                execute(match_case.body);
                matched = true;
            }
            break;
        }

        if (std::dynamic_pointer_cast<LiteralExpr>(match_case.pattern)) {
            Value pattern_value = evaluate(match_case.pattern);
            if (isEqual(subject_value, pattern_value)) {
                execute(match_case.body);
                matched = true;
                break;
            }
        } else if (std::dynamic_pointer_cast<VariableExpr>(match_case.pattern)) {
            // Değişken deseni: her zaman eşleşir ve değeri değişkene atar
            Token var_name = std::dynamic_pointer_cast<VariableExpr>(match_case.pattern)->name;
            std::shared_ptr<Environment> case_env = std::make_shared<Environment>(environment);
            case_env->define(var_name.lexeme, subject_value);
            // Match-case body'si bir BlockStmt olmalı
            if (auto block_body = std::dynamic_pointer_cast<BlockStmt>(match_case.body)) {
                 executeBlock(block_body->statements, case_env);
            } else {
                 // Eğer body bir blok değilse, yeni ortamı kullanmayabiliriz,
                 // veya bu bir hata durumu olabilir.
                 // Şimdilik, body'nin tek bir ifade olduğunu varsayalım ve yeni ortamda yürütelim.
                  execute(match_case.body); // Bu daha karmaşık bir durum.
                 // Match case body'leri genellikle bir BlockStmt olmalıdır.
                 throw runtimeError(var_name, "Match case body'si bir blok olmalıdır.");
            }
            matched = true;
            break;
        }
    }
}

void Interpreter::printValue(const Value& value) {
    std::cout << valueToString(value) << std::endl;
}
