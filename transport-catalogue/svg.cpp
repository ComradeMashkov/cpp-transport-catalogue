#include "svg.h"

namespace svg {

using namespace std::literals;
    
// ------------------ Color ----------------------
    
inline void PrintColor(std::ostream& out, Rgb& rgb) {
    out << "rgb("sv << static_cast<int>(rgb.red) << ","sv << static_cast<int>(rgb.green) << ","sv << static_cast<int>(rgb.blue) << ")"sv;
}
    
inline void PrintColor(std::ostream& out, Rgba& rgba) {
    out << "rgba("sv << static_cast<int>(rgba.red) << ","sv << static_cast<int>(rgba.green) << ","sv << static_cast<int>(rgba.blue) << ","sv << rgba.opacity << ")"sv;
}
    
inline void PrintColor(std::ostream& out, std::monostate) {
    out << "none"sv;
}
    
inline void PrintColor(std::ostream& out, std::string& color) {
    out <<  color;
}
    
std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit([&out] (auto value) { PrintColor(out, value); }, color);
    return out;
}

// ---------------- Object ---------------------
    
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;

    RenderAttrs(context.out);

    out << "/>"sv;
}

// ------------ Polyline ----------------

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;

    for (size_t i = 0u; i < points_.size(); ++i) {
        if (i != points_.size() - 1u) {
            out << points_[i].x << ","sv << points_[i].y << " "sv;
        }
        else {
            out << points_[i].x << ","sv << points_[i].y;
        }
    }

    out << "\""sv;
    RenderAttrs(context.out);
    out << " />"sv;
}

// ------------ Text -------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

std::string Text::DataShielding() const {
    std::string result = ""s;

    for (const char& symbol : data_) {
        switch (symbol) {
        case '"':
            result += "&quot;"sv;
            break;
        case '`':
            result += "&apos;"sv;
            break;
        case '\'':
            result += "&apos;"sv;
            break;
        case '<':
            result += "&lt;"sv;
            break;
        case '>':
            result += "&gt;"sv;
            break;
        case '&':
            result += "&amp;"sv;
            break;
        default:
            result += symbol;
            break;
        }
    }

    return result;
}

std::string Text::RemoveSpaces(const std::string& data) const {
    if (data.empty()) {
        return {};
    }

    auto left_bound = data.find_first_not_of(' ');
    auto right_bound = data.find_last_not_of(' ');
    return data.substr(left_bound, right_bound - left_bound + 1u);
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << size_ << "\""sv;

    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }

    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }

    RenderAttrs(context.out);

    out << ">"sv << RemoveSpaces(DataShielding()) << "</text>"sv;
}

// -------------------- Document -------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    int indent_step = 2;
    int indent = 2;

    RenderContext context(out, indent_step, indent);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    for (const auto& object : objects_) {
        object->Render(context);
    }

    out << "</svg>"sv;
}

}  // namespace svg