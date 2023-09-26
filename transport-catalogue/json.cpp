#include "json.h"

using namespace std;

namespace transport_catalogue {

namespace detail {

namespace json {

namespace {

Node LoadNode(istream& input);

string LoadLine(istream& input) {
    string result;

    while (isalpha(input.peek())) {
        result.push_back(static_cast<char>(input.get()));
    }

    return result;
}

Node LoadArray(istream& input) {
    vector<Node> result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (!input) {
        throw ParsingError("Failed to parse an array"s);
    }

    return Node(result);
}

Node LoadNull(istream& input) {
    const auto line = LoadLine(input);
    if (line == "null"s) {
        return Node(nullptr);
    }

    throw ParsingError("Failed to parse '"s + line + "' as null"s);
}

Node LoadBool(istream& input) {
    const auto line = LoadLine(input);

    if (line == "true"s) {
        return Node(true);
    }
    if (line == "false"s) {
        return Node(false);
    }

    throw ParsingError("Failed to parse '"s + line + "' as bool"s);
}

Node LoadNumber(std::istream& input) {
    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    if (input.peek() == '0') {
        read_char();
    }
    else {
        read_digits();
    }

    bool is_int = true;

    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            try {
                return Node(stoi(parsed_num));
            }
            catch (...) {
            
            }
        }
        return Node(stod(parsed_num));
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        }
        else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        }
        else {
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            string key = LoadString(input).AsString();
            
            if (input >> c && c == ':') {
                if (result.find(key) != result.end()) {
                    throw ParsingError("Failed to parse a dictionary, duplicate key '"s + key + "' has been found"s);
                }

                result.emplace(move(key), LoadNode(input));
            }

            else {
                throw ParsingError("Colon symbol ':' expected, but '"s + c + "' found"s);
            }
        }

        else if (c != ',') {
            throw ParsingError("Comma symbol ',' expected, but '"s + c + "' found"s);
        }
    }

    if (input) {
        return Node(result);
    }

    throw ParsingError("Failed to parse a dictionary: empty input"s);
}

Node LoadNode(istream& input) {
    char c;

    if (input >> c) {
        switch (c) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDict(input);
        case '"':
            return LoadString(input);
        case 't':
            input.putback(c);
            return LoadBool(input);
        case 'f':
            input.putback(c);
            return LoadBool(input);
        case 'n':
            input.putback(c);
            return LoadNull(input);
        default:
            input.putback(c);
            return LoadNumber(input);
        }
    }

    throw ParsingError("Empty input"s);
}

}  // namespace

const Value& Node::GetValue() const {
    return *this;
}

const Array& Node::AsArray() const {
    if (IsArray()) {
        return get<Array>(*this);
    }
    throw logic_error("Current value is not an array-type");
}

const Dict& Node::AsMap() const {
    if (IsMap()) {
        return get<Dict>(*this);
    }
    throw logic_error("Current value is not a dict-type");
}

int Node::AsInt() const {
    if (IsInt()) {
        return get<int>(*this);
    }
    throw logic_error("Current value is not an integer-type");
}

const string& Node::AsString() const {
    if (IsString()) {
        return get<string>(*this);
    }
    throw logic_error("Current value is not a string-type");
}

double Node::AsDouble() const {
    if (IsPureDouble()) {
        return get<double>(*this);
    }
    if (IsDouble()) {
        return AsInt();
    }
    throw logic_error("Current value is not a double-type");
}

bool Node::AsBool() const {
    if (IsBool()) {
        return get<bool>(*this);
    }
    throw logic_error("Current value is not a bool-type");
}

bool Node::IsInt() const { return holds_alternative<int>(*this); }

bool Node::IsDouble() const { return holds_alternative<double>(*this) || holds_alternative<int>(*this); }

bool Node::IsPureDouble() const { return holds_alternative<double>(*this); }

bool Node::IsBool() const { return holds_alternative<bool>(*this); }

bool Node::IsString() const { return holds_alternative<string>(*this); }

bool Node::IsNull() const { return holds_alternative<nullptr_t>(*this); }

bool Node::IsArray() const { return holds_alternative<Array>(*this); }

bool Node::IsMap() const { return holds_alternative<Dict>(*this); }

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }
};

void PrintNode(const Node& node, const PrintContext& context);

void PrintString(const std::string& value, std::ostream& out) {
    out.put('"');

    for (const char c : value) {
        switch (c) {
        case '\r':
            out << "\\r";
            break;
        case '\n':
            out << "\\n";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\\':
            out << "\\\\";
            break;
        default:
            out.put(c);
            break;
        }
    }

    out.put('"');
}

template<typename Value>
void PrintValue(const Value& value, const PrintContext& context) {
    context.out << value;
}

void PrintValue(const std::string& value, const PrintContext& context) {
    PrintString(value, context.out);
}

void PrintValue(const nullptr_t&, const PrintContext& context) {
    context.out << "null"s;
}

void PrintValue(bool value, const PrintContext& context) {
    context.out << boolalpha << value;
}

void PrintValue(Array nodes, const PrintContext& context) {
    context.out << "[\n"s;

    bool flag = true;
    auto inner_context = context.Indented();

    for (const Node& node : nodes) {
        if (flag) {
            flag = false;
        }
        else {
            context.out << ",\n"s;
        }

        inner_context.PrintIndent();
        PrintNode(node, inner_context);
    }

    context.out.put('\n');
    context.PrintIndent();
    context.out.put(']');
}

void PrintValue(Dict nodes, const PrintContext& context) {
    context.out << "{\n"s;

    bool flag = true;
    auto inner_context = context.Indented();

    for (const auto& [key, node] : nodes) {
        if (flag) {
            flag = false;
        }
        else {
            context.out << ",\n"s;
        }

        inner_context.PrintIndent();
        PrintString(key, context.out);
        context.out << ": "s;
        PrintNode(node, inner_context);
    }

    context.out.put('\n');
    context.PrintIndent();
    context.out.put('}');
}

void PrintNode(const Node& node, const PrintContext& context) {
    visit([&context](const auto& value) { PrintValue(value, context); }, node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{ output });
}

}  // namespace json

} // namespace detail

} // namespace transport_catalogue