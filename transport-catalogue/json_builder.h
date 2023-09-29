#pragma once

#include "json.h"

namespace transport_catalogue {

namespace detail {

namespace json {

class KeyContext;
class DictContext;
class ArrayContext;

class Builder {
public:
    KeyContext Key(std::string key);
    Builder& Value(Node::Value value);

    DictContext StartDict();
    ArrayContext StartArray();

    Builder& EndDict();
    Builder& EndArray();

    Node Build();

private:
    Node CreateNode(Node::Value value) const;
    void AddNode(Node node);

    Node root_;
    std::vector<Node*> nodes_stack_;
};

class BaseContext {
public:
    BaseContext(Builder& builder);

    KeyContext Key(std::string key);

    Builder& Value(Node::Value value);

    DictContext StartDict();

    ArrayContext StartArray();

    Builder& EndDict();

    Builder& EndArray();

protected:
    Builder& builder_;
};

class KeyContext final : public BaseContext {
public:
    KeyContext(Builder& builder);

    KeyContext Key(std::string key) = delete;

    DictContext Value(Node::Value value);

    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
};

class DictContext final : public BaseContext {
public:
    DictContext(Builder& builder);

    Builder& Value(Node::Value value) = delete;

    DictContext StartDict() = delete;
    ArrayContext StartArray() = delete;

    Builder& EndArray() = delete;
};

class ArrayContext final : public BaseContext {
public:
    ArrayContext(Builder& builder);
    
    KeyContext Key(std::string key) = delete;

    ArrayContext Value(Node::Value value);

    Builder& EndDict() = delete;
};

} // namespace json

} // namespace detail

} // namespace transport_catalogue
