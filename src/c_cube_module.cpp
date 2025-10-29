#include "c_cube_module.h"
#include "error_reporter.h" // RuntimeException için
#include "utils.h"          // valueToString için (genellikle)

CCubeModule::CCubeModule(const std::string& name, std::shared_ptr<Environment> env)
    : name(name), moduleEnvironment(env) {}

Value CCubeModule::getMember(const Token& name) {
    if (moduleEnvironment->contains(name.lexeme)) {
        return moduleEnvironment->get(name);
    }
    throw RuntimeException(name, "Modül '" + this->name + "' içinde '" + name.lexeme + "' adlı üye bulunamadı.");
}

std::string CCubeModule::toString() const {
    return "<module " + name + ">";
}

// GC için boyut hesaplama
size_t CCubeModule::getSize() const {
    // Modül objesinin kendi boyutu
    size_t total_size = sizeof(CCubeModule);
    // Modül adının string boyutu
    total_size += name.capacity();
    // moduleEnvironment'ın boyutu, environment'ın içindeki değişkenlerin boyutu ayrı olarak GC tarafından yönetilir.
    // Burada sadece shared_ptr'nin boyutu ve Environment objesinin kendisinin boyutu.
    total_size += sizeof(std::shared_ptr<Environment>);
    return total_size;
}
