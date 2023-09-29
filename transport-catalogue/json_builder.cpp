#include "json_builder.h"

using namespace std;

namespace transport_catalogue {

namespace detail {

namespace json {

// Base
 
BaseContext::BaseContext(Builder& builder) 
    : builder_(builder) {
}
 
KeyContext BaseContext::Key(std::string key) {
    return builder_.Key(key);
}

Builder& BaseContext::Value(Node::Value value) {
    return builder_.Value(value);
}
 
DictContext BaseContext::StartDict() {
    return DictContext(builder_.StartDict());
}

ArrayContext BaseContext::StartArray() {
    return ArrayContext(builder_.StartArray());
}

Builder& BaseContext::EndDict() {
    return builder_.EndDict();
}
 

Builder& BaseContext::EndArray() {
    return builder_.EndArray();
}
    
// Key
 
KeyContext::KeyContext(Builder& builder) 
    : BaseContext(builder) {
}
 
DictContext  KeyContext::Value(Node::Value value) {
    return BaseContext::Value(move(value));
}

// Dict
 
DictContext::DictContext(Builder& builder) 
    : BaseContext(builder) {
}
 
// Array
 
ArrayContext::ArrayContext(Builder& builder) 
    : BaseContext(builder) {
}
 
ArrayContext ArrayContext::Value (Node::Value value) {
    return BaseContext::Value(move(value)); 
}

// Builder

Node Builder::CreateNode(Node::Value value) const {
    if (holds_alternative<bool>(value)) {
        return Node(get<bool>(value));
    }
    if (holds_alternative<int>(value)) {
        return Node(get<int>(value));
    }
    if (holds_alternative<double>(value)) {
        return Node(get<double>(value));
    }
    if (holds_alternative<string>(value)) {
        return Node(get<string>(value));
    }
    if (holds_alternative<Array>(value)) {
        return Node(get<Array>(value));
    }
    if (holds_alternative<Dict>(value)) {
        return Node(get<Dict>(value));
    }
    return Node();
}

void Builder::AddNode(Node node) {
    if (nodes_stack_.empty()) {
        if (!root_.IsNull()) {
            throw logic_error("Failed to add a node: root has been already added"s);
        }

        root_ = node;
        return;
    }

    if (!nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsString()) {
        throw logic_error("Failed to add a node: incorrect type"s);
    }

    if (nodes_stack_.back()->IsArray()) {
        Array tmp = nodes_stack_.back()->AsArray();
        tmp.emplace_back(node);
        nodes_stack_.pop_back();
        nodes_stack_.emplace_back(move(new Node(tmp)));

        return;
    }

    // if nodes_stack_.back()->IsString():
    string tmp = nodes_stack_.back()->AsString();
    nodes_stack_.pop_back();

    if (nodes_stack_.back()->IsDict()) {
        Dict dict = nodes_stack_.back()->AsDict();
        dict.emplace(move(tmp), node);
        nodes_stack_.pop_back();
        nodes_stack_.emplace_back(move(new Node(dict)));
    }
}

KeyContext Builder::Key(string key) {
    if (nodes_stack_.empty()) {
        throw logic_error("Failed to create a key: stack is empty"s);
    }

    if (nodes_stack_.back()->IsDict()) {
        nodes_stack_.emplace_back(move(new Node(key)));
    }

    return KeyContext(*this);
}

Builder& Builder::Value(Node::Value value) {
    AddNode(CreateNode(value));
    return *this;
}

DictContext Builder::StartDict() {
    nodes_stack_.emplace_back(move(new Node(Dict())));
    return DictContext(*this);
}

ArrayContext Builder::StartArray() {
    nodes_stack_.emplace_back(move(new Node(Array())));
    return ArrayContext(*this);
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty()) {
        throw logic_error("Failed to end a dict: it was not opened"s);
    }

    Node node = *nodes_stack_.back();

    if (!node.IsDict()) {
        throw logic_error("Failed to end a dict: object is not a dict-type"s);
    }

    nodes_stack_.pop_back();
    AddNode(node);
    
    return *this;
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty()) {
        throw logic_error("Failed to end an array: it was not opened"s);
    }

    Node node = *nodes_stack_.back();

    if (!node.IsArray()) {
        throw logic_error("Failed to end an array: object is not an array-type"s);
    }

    nodes_stack_.pop_back();
    AddNode(node);
    
    return *this;
}

Node Builder::Build() {
    if (root_.IsNull()) {
        throw logic_error("Failed to build a JSON: it is empty"s);
    }

    if (!nodes_stack_.empty()) {
        throw logic_error("Failed to build a JSON: nodes stack is not empty"s);
    }

    return root_;
}

} // namespace json

} // namespace detail

} // namespace transport_catalogue