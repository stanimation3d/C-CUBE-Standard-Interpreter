#include "list.h"
#include "error_reporter.h" // RuntimeException için
#include "utils.h"          // valueToString için
#include <sstream>

// Constructor
CCubeList::CCubeList(const std::vector<Value>& initialElements) : elements(initialElements) {}

// Liste elemanına değer ekler
void CCubeList::add(Value val) {
    elements.push_back(val);
}

// Belirtilen indeksteki elemanı alır
Value CCubeList::get_at(size_t index) const {
    if (index >= elements.size()) {
        throw RuntimeException(Token(TokenType::NUMBER, "", static_cast<double>(index), -1), "Liste dizin sınırları dışında.");
    }
    return elements[index];
}

// Belirtilen indeksteki elemana değer atar
void CCubeList::set_at(size_t index, Value val) {
    if (index >= elements.size()) {
        throw RuntimeException(Token(TokenType::NUMBER, "", static_cast<double>(index), -1), "Liste dizin sınırları dışında.");
    }
    elements[index] = val;
}

// Listenin boyutunu döndürür
size_t CCubeList::size() const {
    return elements.size();
}

// Object arayüzünden toString implementasyonu
std::string CCubeList::toString() const {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        ss << valueToString(elements[i]);
        if (i < elements.size() - 1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}

// GC için boyut hesaplama
size_t CCubeList::getSize() const {
    // CCubeList'in kendi boyutu
    size_t total_size = sizeof(CCubeList);
    // Vector'ün kendi belleği ve içindeki her Value'nun boyutu
    total_size += elements.capacity() * sizeof(Value); // Kapasite kadar bellek tutabilir
    return total_size;
}
