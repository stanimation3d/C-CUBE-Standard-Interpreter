#ifndef C_CUBE_UTILS_H
#define C_CUBE_UTILS_H

#include "value.h" // ValuePtr için
#include <string>
#include <vector>
#include <memory>

// Utility functions for the C-CUBE interpreter
namespace Utils {

    // Converts a ValuePtr to its string representation for printing or debugging.
    std::string valueToString(ValuePtr value);

    // Determines the "truthiness" of a ValuePtr according to C-CUBE's rules.
    bool isTruthy(ValuePtr value);

    // Compares two ValuePtrs for equality.
    bool isEqual(ValuePtr a, ValuePtr b);

    // Compares two ValuePtrs to check if a is greater than b.
    // Primarily for numbers and strings.
    bool isGreater(ValuePtr a, ValuePtr b);

    // Compares two ValuePtrs to check if a is less than b.
    // Primarily for numbers and strings.
    bool isLess(ValuePtr a, ValuePtr b);

    // Optional: Implement other comparison operators if needed or handle in interpreter.
     bool isGreaterEqual(ValuePtr a, ValuePtr b);
     bool isLessEqual(ValuePtr a, ValuePtr b);

    // Optional: Helper for type checking
     bool areNumbers(ValuePtr a, ValuePtr b);
     bool areStrings(ValuePtr a, ValuePtr b);

} // namespace Utils

#endif // C_CUBE_UTILS_H
