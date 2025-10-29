#include "utils.h"
#include "value.h" // Value variant ve ValuePtr için
#include "object.h" // C_CUBE_Object'ten toString çağırmak için
#include "function.h" // C_CUBE_Function'dan toString çağırmak için
#include "class.h"    // C_CUBE_Class'tan toString çağırmak için
#include "c_cube_module.h" // C_CUBE_Module'dan toString çağırmak için

#include <string>
#include <vector>
#include <cmath> // std::isnan için
#include <variant> // std::visit, std::holds_alternative, std::get için
#include <iostream> // Debugging/Error

namespace Utils {

    // Helper to check if two values are numbers
    bool areNumbers(ValuePtr a, ValuePtr b) {
        return a && b && std::holds_alternative<double>(*a) && std::holds_alternative<double>(*b);
    }

     // Helper to check if two values are strings
    bool areStrings(ValuePtr a, ValuePtr b) {
        return a && b && std::holds_alternative<std::string>(*a) && std::holds_alternative<std::string>(*b);
    }

    // Converts a ValuePtr to its string representation
    std::string valueToString(ValuePtr value) {
        if (!value) {
            return "nullptr"; // Should not happen if ValuePtr is managed correctly, but as a fallback
        }

        // Use std::visit for cleaner handling of variant types
        return std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return "none"; // Represents C-CUBE's 'none'
            } else if constexpr (std::is_same_v<T, bool>) {
                return arg ? "true" : "false";
            } else if constexpr (std::is_same_v<T, double>) {
                // std::to_string might need formatting for precision etc.
                // Handle NaN and infinity explicitly if needed
                 if (std::isnan(arg)) return "NaN";
                 if (std::isinf(arg)) return (arg < 0 ? "-Infinity" : "Infinity");

                std::string s = std::to_string(arg);
                // Remove trailing zeros and decimal point for whole numbers (like Python)
                s.erase(s.find_last_not_of('0') + 1, std::string::npos);
                 if (s.back() == '.') s.pop_back();
                 return s;

            } else if constexpr (std::is_same_v<T, std::string>) {
                return arg; // String literals don't need quotes in this representation
            }
            // Handle shared pointers to complex types (if they are part of Value variant)
            else if constexpr (std::is_same_v<T, std::shared_ptr<C_CUBE_Object>>) {
                 if (arg) return arg->toString();
                 return "null object"; // Should not happen with GcObject
            }
             else if constexpr (std::is_same_v<T, std::shared_ptr<C_CUBE_Function>>) {
                 if (arg) return arg->toString();
                 return "null function"; // Should not happen with GcObject
             }
             else if constexpr (std::is_same_v<T, std::shared_ptr<C_CUBE_Class>>) {
                 if (arg) return arg->toString();
                 return "null class"; // Should not happen with GcObject
             }
              else if constexpr (std::is_same_v<T, std::shared_ptr<C_CUBE_Module>>) {
                 if (arg) return arg->toString();
                 return "null module"; // Should not happen with GcObject
             }
            // Add cases for other Value types (List, Dict, Callable base if needed)
            else if constexpr (std::is_same_v<T, std::shared_ptr<Callable>>) {
                 if (arg) return arg->toString(); // Fallback for generic callable
                 return "null callable";
            }

            else {
                // Fallback for unhandled types in the variant
                return "[unsupported value type]";
            }
        }, *value); // Dereference the ValuePtr to access the variant
    }

    // Determines the "truthiness" of a ValuePtr
    bool isTruthy(ValuePtr value) {
        if (!value) return false; // Null pointer is falsy

        return std::visit([](auto&& arg) -> bool {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return false; // 'none' is falsy
            } else if constexpr (std::is_same_v<T, bool>) {
                return arg; // Boolean value itself
            } else if constexpr (std::is_same_v<T, double>) {
                return arg != 0; // Numbers are falsy only if 0
            } else if constexpr (std::is_same_v<T, std::string>) {
                return !arg.empty(); // Strings are falsy only if empty
            }
            // Objects, functions, classes, modules, lists, dicts etc. are typically truthy
             else if constexpr (std::is_same_v<T, std::shared_ptr<C_CUBE_Object>>) {
                 return arg != nullptr; // Should always be true if Value holds a valid ptr
             }
              else if constexpr (std::is_same_v<T, std::shared_ptr<C_CUBE_Function>>) {
                 return arg != nullptr;
             }
              else if constexpr (std::is_same_v<T, std::shared_ptr<C_CUBE_Class>>) {
                 return arg != nullptr;
             }
               else if constexpr (std::is_same_v<T, std::shared_ptr<C_CUBE_Module>>) {
                 return arg != nullptr;
             }
            // Add cases for other Value types (List, Dict etc.)
            else if constexpr (std::is_same_v<T, std::shared_ptr<Callable>>) {
                 return arg != nullptr;
             }

            else {
                // By default, other types are truthy if the value itself is not null/empty equivalent
                return true;
            }
        }, *value);
    }

    // Compares two ValuePtrs for equality
    bool isEqual(ValuePtr a, ValuePtr b) {
        // Both are null or point to the same Value object
        if (a == b) return true;
        // One is null, the other is not
        if (!a || !b) return false;

        // Use std::visit to compare values inside the variants
        // This requires the variant types to be comparable or defining comparison logic for pairs of types
        // A common approach is to check if the types are the same first.
        if (a->index() != b->index()) {
            return false; // Different types are not equal (usually)
        }

        // Now we know the underlying types in the variant are the same.
        // Use std::visit to get the actual values and compare.
        return std::visit([](auto&& arg1, auto&& arg2) -> bool {
            using T1 = std::decay_t<decltype(arg1)>;
            using T2 = std::decay_t<decltype(arg2)>;

            // This branch is only taken if arg1 and arg2 have the same type (due to a->index() != b->index() check)
            // So T1 and T2 are the same.
            if constexpr (std::is_same_v<T1, std::monostate>) {
                return true; // none == none
            } else if constexpr (std::is_same_v<T1, bool>) {
                return arg1 == arg2;
            } else if constexpr (std::is_same_v<T1, double>) {
                // Handle floating point comparison with tolerance if needed
                return arg1 == arg2;
            } else if constexpr (std::is_same_v<T1, std::string>) {
                return arg1 == arg2;
            }
            // For object types, compare pointers (reference equality) or implement value equality if desired
             else if constexpr (std::is_same_v<T1, std::shared_ptr<C_CUBE_Object>> ||
                                std::is_same_v<T1, std::shared_ptr<C_CUBE_Function>> ||
                                std::is_same_v<T1, std::shared_ptr<C_CUBE_Class>> ||
                                std::is_same_v<T1, std::shared_ptr<C_CUBE_Module>> ||
                                std::is_same_v<T1, std::shared_ptr<Callable>>) {
                 return arg1 == arg2; // Pointer equality
             }
            // Add cases for other Value types (List, Dict etc.) - implement value equality if needed

            else {
                // Default: if types are the same but not explicitly handled, assume not equal
                return false;
            }
        }, *a, *b); // Dereference ValuePtrs and visit both variants
    }

    // Compares two ValuePtrs to check if a is greater than b
    bool isGreater(ValuePtr a, ValuePtr b) {
        // Comparison is typically only defined for numbers and strings
        if (areNumbers(a, b)) {
            return std::get<double>(*a) > std::get<double>(*b);
        } else if (areStrings(a, b)) {
            return std::get<std::string>(*a) > std::get<std::string>(*b);
        }

        // Error: Invalid operand types for > operator
         runtimeError(?, "Invalid operand types for >."); // Need token context for error
        std::cerr << "Runtime Error: Invalid operand types for >." << std::endl;
         throw std::runtime_error("Invalid operand types for >.");
        return false; // Indicate comparison failed or resulted in false
    }

    // Compares two ValuePtrs to check if a is less than b
    bool isLess(ValuePtr a, ValuePtr b) {
        // Comparison is typically only defined for numbers and strings
         if (areNumbers(a, b)) {
            return std::get<double>(*a) < std::get<double>(*b);
        } else if (areStrings(a, b)) {
            return std::get<std::string>(*a) < std::get<std::string>(*b);
        }

        // Error: Invalid operand types for < operator
         std::cerr << "Runtime Error: Invalid operand types for <." << std::endl;
         throw std::runtime_error("Invalid operand types for <.");
        return false; // Indicate comparison failed or resulted in false
    }

     // Optional: Implement other comparison operators based on isGreater and isLess
    
    bool isGreaterEqual(ValuePtr a, ValuePtr b) {
        // a >= b is equivalent to not (a < b)
        return !isLess(a, b);
    }

    bool isLessEqual(ValuePtr a, ValuePtr b) {
        // a <= b is equivalent to not (a > b)
        return !isGreater(a, b);
    }
    

} // namespace Utils
