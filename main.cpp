#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "httplib.h"

struct Token {
    std::string lexeme;
    std::string type;
};

class Lexer {
public:
    explicit Lexer(const std::string& input) : input_(input), pos_(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (pos_ < input_.size()) {
            char ch = input_[pos_];

            if (std::isspace(static_cast<unsigned char>(ch))) {
                ++pos_;
                continue;
            }

            if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
                tokens.push_back(readIdentifier());
                continue;
            }

            if (std::isdigit(static_cast<unsigned char>(ch))) {
                tokens.push_back(readNumber());
                continue;
            }

            if (isBracket(ch)) {
                tokens.push_back(Token{std::string(1, ch), "SPECIAL_CHAR"});
                ++pos_;
                continue;
            }

            if (isOperatorStart(ch)) {
                tokens.push_back(readOperator());
                continue;
            }

            if (ch == ';') {
                tokens.push_back(Token{";", "SPECIAL_CHAR"});
                ++pos_;
                continue;
            }

            if (ch == '.' || ch == ',' || ch == ':') {
                tokens.push_back(Token{std::string(1, ch), "SPECIAL_CHAR"});
                ++pos_;
                continue;
            }

            tokens.push_back(Token{std::string(1, ch), "UNKNOWN"});
            ++pos_;
        }

        return tokens;
    }

private:
    Token readIdentifier() {
        std::size_t start = pos_;
        while (pos_ < input_.size()) {
            char ch = input_[pos_];
            if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
                break;
            }
            ++pos_;
        }
        std::string lexeme = input_.substr(start, pos_ - start);
        if (isKeyword(lexeme)) {
            return Token{lexeme, "KEYWORD"};
        }
        return Token{lexeme, "IDENTIFIER"};
    }

    Token readNumber() {
        std::size_t start = pos_;
        while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
            ++pos_;
        }

        if (pos_ < input_.size() && input_[pos_] == '.' &&
            pos_ + 1 < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_ + 1]))) {
            ++pos_;
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                ++pos_;
            }
        }

        std::size_t exponentStart = pos_;
        if (pos_ < input_.size() && (input_[pos_] == 'e' || input_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < input_.size() && input_[pos_] == '^') {
                ++pos_;
            }
            if (pos_ < input_.size() && (input_[pos_] == '+' || input_[pos_] == '-')) {
                ++pos_;
            }

            std::size_t exponentDigitsStart = pos_;
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                ++pos_;
            }

            if (exponentDigitsStart == pos_) {
                pos_ = exponentStart;
            }
        }

        return Token{input_.substr(start, pos_ - start), "NUMERIC_CONSTANT"};
    }

    Token readOperator() {
        char first = input_[pos_];
        std::string lexeme(1, first);

        if (pos_ + 1 < input_.size()) {
            char second = input_[pos_ + 1];
            if ((first == '<' && second == '<') ||
                (first == '>' && second == '>') ||
                (first == '=' && second == '=') ||
                (first == '!' && second == '=') ||
                (first == '<' && second == '>') ||
                (first == '<' && second == '=') ||
                (first == '>' && second == '=')) {
                lexeme.push_back(second);
                pos_ += 2;
                return Token{lexeme, "OPERATOR"};
            }
        }

        ++pos_;
        return Token{lexeme, "OPERATOR"};
    }

    static bool isOperatorStart(char ch) {
        return ch == '=' || ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
               ch == '<' || ch == '>' || ch == '!';
    }

    static bool isBracket(char ch) {
        return ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']';
    }

    static bool isKeyword(const std::string& lexeme) {
        static const std::unordered_set<std::string> keywords = {
            "if", "else", "for", "while", "do", "switch", "case",
            "break", "continue", "return", "int", "float", "double",
            "char", "void", "long", "short", "signed", "unsigned"
        };
        return keywords.find(lexeme) != keywords.end();
    }

    std::string input_;
    std::size_t pos_;
};

static std::string escapeJson(const std::string& input) {
    std::ostringstream out;
    for (char c : input) {
        switch (c) {
            case '\"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    out << "\\u"
                        << "00"
                        << "0123456789abcdef"[(c >> 4) & 0x0F]
                        << "0123456789abcdef"[c & 0x0F];
                } else {
                    out << c;
                }
        }
    }
    return out.str();
}

static std::string extractCodeField(const std::string& jsonBody) {
    const std::string key = "\"code\"";
    std::size_t keyPos = jsonBody.find(key);
    if (keyPos == std::string::npos) {
        return "";
    }

    std::size_t colonPos = jsonBody.find(':', keyPos + key.size());
    if (colonPos == std::string::npos) {
        return "";
    }

    std::size_t firstQuote = jsonBody.find('\"', colonPos + 1);
    if (firstQuote == std::string::npos) {
        return "";
    }

    std::string result;
    bool escaped = false;
    for (std::size_t i = firstQuote + 1; i < jsonBody.size(); ++i) {
        char c = jsonBody[i];

        if (escaped) {
            switch (c) {
                case '\"': result.push_back('\"'); break;
                case '\\': result.push_back('\\'); break;
                case '/': result.push_back('/'); break;
                case 'b': result.push_back('\b'); break;
                case 'f': result.push_back('\f'); break;
                case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break;
                case 't': result.push_back('\t'); break;
                default: result.push_back(c); break;
            }
            escaped = false;
            continue;
        }

        if (c == '\\') {
            escaped = true;
            continue;
        }

        if (c == '\"') {
            return result;
        }

        result.push_back(c);
    }

    return "";
}

static std::string tokensToJson(const std::vector<Token>& tokens) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) {
            out << ",";
        }
        out << "{\"lexeme\":\"" << escapeJson(tokens[i].lexeme)
            << "\",\"type\":\"" << escapeJson(tokens[i].type) << "\"}";
    }
    out << "]";
    return out.str();
}

static void applyCors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main() {
    httplib::Server server;

    server.Options("/analyze", [](const httplib::Request&, httplib::Response& res) {
        applyCors(res);
        res.status = 204;
    });

    server.Post("/analyze", [](const httplib::Request& req, httplib::Response& res) {
        applyCors(res);

        std::string code = extractCodeField(req.body);
        Lexer lexer(code);
        std::vector<Token> tokens = lexer.tokenize();

        res.set_content(tokensToJson(tokens), "application/json");
    });

    std::cout << "Lexer server running at http://localhost:8080\n";
    server.listen("0.0.0.0", 8080);
    return 0;
}
