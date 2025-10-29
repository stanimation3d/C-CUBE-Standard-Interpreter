#include "value.h"
#include "gc.h"             // GcObject ve GcPtr tanımları için
#include "object.h"         // C_CUBE_Object
#include "function.h"       // C_CUBE_Function
#include "class.h"          // C_CUBE_Class
#include "c_cube_module.h"  // C_CUBE_Module (ModulePtr için)
#include "callable.h"       // Callable

// GcObject'ten türemiş bir shared_ptr'ı güvenli bir şekilde GcPtr olarak döndürür.
// ValuePtr'ın içerdiği bir GcObject ise, onu GcPtr'a downcast eder.
GcPtr Value::getGcObject() const {
    // std::visit kullanarak variant'ın içerdiği her tipi kontrol et
    return std::visit([](auto&& arg) -> GcPtr {
        using T = std::decay_t<decltype(arg)>;

        // Eğer variant'ın içeriği bir shared_ptr ve bu shared_ptr bir GcObject'ten türemişse
        if constexpr (std::is_base_of<GcObject, typename T::element_type>::value &&
                      std::is_same_v<T, std::shared_ptr<typename T::element_type>>) {
            // Güvenli downcast yaparak GcPtr olarak döndür
            return std::dynamic_pointer_cast<GcObject>(arg);
        }
        // Diğer durumlar için nullptr döndür (örn. bool, double, string, monostate)
        else {
            return nullptr;
        }
    }, data);
}

// Diğer Value metodları buraya gelebilir (örn. operatör aşırı yüklemeleri, hash vb.)
