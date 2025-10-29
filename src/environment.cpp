#include "environment.h"

// Constructor
Environment::Environment() : enclosing(nullptr) {}

// Constructor for nested environments
Environment::Environment(std::shared_ptr<Environment> enclosing)
    : enclosing(enclosing) {}

// Defines a new variable in the current environment
void Environment::define(const std::string& name, Value value) {
    values[name] = value;
}

// Assigns a value to an existing variable, searching up the scope chain
void Environment::assign(const Token& name, Value value) {
    if (values.count(name.lexeme)) {
        values[name.lexeme] = value;
        return;
    }
    if (enclosing != nullptr) {
        enclosing->assign(name, value);
        return;
    }
    throw RuntimeException(name, "Tanımlanmamış değişken '" + name.lexeme + "'.");
}

// Retrieves the value of a variable, searching up the scope chain
Value Environment::get(const Token& name) {
    if (values.count(name.lexeme)) {
        return values.at(name.lexeme);
    }
    if (enclosing != nullptr) {
        return enclosing->get(name);
    }
    throw RuntimeException(name, "Tanımlanmamış değişken '" + name.lexeme + "'.");
}

// Helper method to find an ancestor environment at a given distance
std::shared_ptr<Environment> Environment::ancestor(int distance) {
    std::shared_ptr<Environment> environment = shared_from_this();
    for (int i = 0; i < distance; ++i) {
        if (environment->enclosing == nullptr) {
            // This should ideally not happen if a resolver pre-calculated distances.
            throw std::runtime_error("Ancestor araması sırasında beklenmeyen nullptr ortam.");
        }
        environment = environment->enclosing;
    }
    return environment;
}

// Retrieves the value of a variable at a specific scope distance
Value Environment::getAt(int distance, const std::string& name) {
    return ancestor(distance)->values.at(name);
}

// Assigns a value to a variable at a specific scope distance
void Environment::assignAt(int distance, const Token& name, Value value) {
    ancestor(distance)->values[name.lexeme] = value;
}

// Checks if the current environment contains a variable
bool Environment::contains(const std::string& name) const {
    return values.count(name);
}

// Returns the enclosing environment
std::shared_ptr<Environment> Environment::getEnclosing() const {
    return enclosing;
}
