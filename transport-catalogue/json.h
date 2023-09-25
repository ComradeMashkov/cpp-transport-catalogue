﻿#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace transport_catalogue {

namespace detail {

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    
    const Value& GetValue() const;

    Node() = default;
    Node(bool value);
    Node(Array array);
    Node(Dict map);
    Node(int value);
    Node(std::string value);
    Node(std::nullptr_t);
    Node(double value);

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    const std::string& AsString() const;
    double AsDouble() const;
    bool AsBool() const;
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

private:
    Value value_;
};
    
inline bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}

inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

class Document {
public:
    Document() = default;
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};
    
inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}
    
Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json

} // namespace detail

} // namespace transport_catalogue