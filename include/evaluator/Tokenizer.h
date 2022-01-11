#ifndef TOKENIZER_H_
#define TOKENIZER_H_
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <evaluator/EvaluatorDefs.h>
namespace eval
{
enum class TokenType
{
    NONE,
    OPERAND,
    ADD,     // +
    SUB,     // -
    MUL,     // *
    DIV,     // /
    POW,     // ^
    LPAREN,  // (
    RPAREN,  // )
    COMMA,   // ,
    EQ,      // =
    SYMBOL,  // variable or function name
};

inline int getOperatorPrecedence(const TokenType& ty)
{
    switch (ty)
    {
        case TokenType::ADD:
        case TokenType::SUB:
            return 1;
        case TokenType::MUL:
        case TokenType::DIV:
            return 2;
        case TokenType::POW:
            return 3;
        default:
            return (0);
    }
}

struct Token
{
    TokenType type;
    std::variant<std::monostate, operand_t, std::string> value;

    Token(const TokenType& _t = TokenType::NONE) : type(_t) {}
    Token(const operand_t& _v) : type(TokenType::OPERAND)
    {
        value.emplace<1>(_v);
    }
    Token(const std::string& _v) : type(TokenType::SYMBOL)
    {
        value.emplace<2>(_v);
    }

    inline bool isSymbol() const { return type == TokenType::SYMBOL; }
    inline bool isOperand() const { return type == TokenType::OPERAND; }
    inline bool isLParen() const { return type == TokenType::LPAREN; }
    inline bool isRParen() const { return type == TokenType::RPAREN; }
    inline bool isComma() const { return type == TokenType::COMMA; }
    inline bool isAdd() const { return type == TokenType::ADD; }
    inline bool isSub() const { return type == TokenType::SUB; }
    inline bool isEq() const { return type == TokenType::EQ; }

    inline bool isOperator() const
    {
        return (type == TokenType::ADD || type == TokenType::SUB ||
                type == TokenType::MUL || type == TokenType::DIV ||
                type == TokenType::POW);
    }

    inline std::string getSymbol() const
    {
#ifdef EVAL_DO_TYPE_CHECK
        EVAL_THROW(type != TokenType::SYMBOL, EVAL_UNEXPECTED_TOKEN_TYPE);
#endif
        return std::get<2>(value);
    }

    inline operand_t getOperand() const
    {
#ifdef EVAL_DO_TYPE_CHECK
        EVAL_THROW(type != TokenType::OPERAND, EVAL_UNEXPECTED_TOKEN_TYPE);
#endif
        return std::get<1>(value);
    }

    std::string toString() const;
    virtual ~Token() {}
};

class TokenList : public std::vector<Token>
{
   protected:
    template <typename T>
    static bool parseOperand(std::string::const_iterator&,
                             const std::string::const_iterator&, T&)
    {
        throw EvalException(EVAL_OPERAND_PARSER_UNDEFINED);
    }

    static inline bool isDigit(char c) { return '0' <= c && c <= '9'; }
    static inline bool isDigitNonZero(char c) { return '0' < c && c <= '9'; }
    static inline bool isSpace(char c)
    {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }
    static inline bool isSymbolStart(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
    }
    static inline bool isSymbol(char c)
    {
        return isSymbolStart(c) || isDigit(c);
    }

    Token parse(std::string::const_iterator& ite,
                const std::string::const_iterator& end);
    static void parseSpace(std::string::const_iterator& ite,
                           const std::string::const_iterator& end);
    static bool parseInt(std::string::const_iterator& ite,
                         const std::string::const_iterator& end, int_t& opnd);
    static bool parseDecimal(std::string::const_iterator& ite,
                             const std::string::const_iterator& end,
                             decimal_t& opnd);
    static bool parseOperator(std::string::const_iterator& ite,
                              const std::string::const_iterator& end,
                              TokenType& ty);
    static bool parseSymbol(std::string::const_iterator& ite,
                            const std::string::const_iterator& end,
                            std::string& symbol);

   public:
    TokenList() = default;
    TokenList(const TokenList::const_iterator& beg,
              const TokenList::const_iterator& end)
        : std::vector<Token>(beg, end)
    {
    }
    TokenList(const std::string& buffer);
    virtual ~TokenList() {}
};

TokenList::const_iterator findParen(const TokenList::const_iterator& beg,
                                    const TokenList::const_iterator& end);

TokenList::const_iterator findArgSep(const TokenList::const_iterator& beg,
                                     const TokenList::const_iterator& end);

}  // namespace eval

#endif