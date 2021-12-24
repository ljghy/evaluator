#include "evaluator/Tokenizer.h"
namespace evaluator
{
std::function<bool(std::string::const_iterator&,
                   const std::string::const_iterator&, operand_t&)>
    TokenList::parseOperand = TokenList::parseDecimal;

std::string Token::toString() const
{
    switch (type)
    {
        case TokenType::OPERAND:
            return std::to_string(std::get<1>(value));
        case TokenType::SYMBOL:
            return std::get<2>(value);
        case TokenType::ADD:
            return "+";
        case TokenType::SUB:
            return "-";
        case TokenType::MUL:
            return "*";
        case TokenType::DIV:
            return "/";
        case TokenType::POW:
            return "^";
        case TokenType::LPAREN:
            return "(";
        case TokenType::RPAREN:
            return ")";
        case TokenType::COMMA:
            return ",";
        case TokenType::EQ:
            return "=";
        default:
            return "None";
    }
}

TokenList::TokenList(const std::string& buffer)
{
    auto ite = buffer.begin(), end_ite = buffer.end();
    parseSpace(ite, end_ite);
    while (ite != end_ite)
    {
        auto tk = parse(ite, end_ite);
        EVAL_THROW(tk.type == TokenType::NONE, "parse failed");
        push_back(tk);
        parseSpace(ite, end_ite);
    }
}

Token TokenList::parse(std::string::const_iterator& ite,
                       const std::string::const_iterator& end)
{
    operand_t opnd;
    if (parseOperand(ite, end, opnd)) return Token(opnd);
    TokenType ty;
    if (parseOperator(ite, end, ty)) return Token(ty);
    std::string symbol;
    if (parseSymbol(ite, end, symbol)) return Token(symbol);
    return Token();
}
void TokenList::parseSpace(std::string::const_iterator& ite,
                           const std::string::const_iterator& end)
{
    while (ite != end && isSpace(*ite)) ++ite;
}
bool TokenList::parseInt(std::string::const_iterator& ite,
                         const std::string::const_iterator& end,
                         operand_t& opnd)
{
    if (ite == end) return false;
    if (!isDigit(*ite)) return false;
    std::stringstream ss;
    while (ite != end && isDigit(*ite))
    {
        ss << *ite;
        ++ite;
    }
    ss >> opnd;
    EVAL_THROW(ss.fail(), "operand overflow");
    return true;
}
bool TokenList::parseDecimal(std::string::const_iterator& ite,
                             const std::string::const_iterator& end,
                             operand_t& opnd)
{
    if (ite == end) return false;
    if (!isDigit(*ite)) return false;
    auto cpy = ite;
    std::stringstream ss;
    if (isDigitNonZero(*ite))
    {
        while (isDigit(*ite))
        {
            ss << *ite;
            ++ite;
        }
    }
    else
    {
        ss << *ite;
        ++ite;
    }
    if (*ite == '.')
    {
        ss << *ite;
        ++ite;

        while (isDigit(*ite))
        {
            ss << *ite;
            ++ite;
        }
    }
    if (*ite == 'e' || *ite == 'E')
    {
        ss << *ite;
        ++ite;
        if (*ite == '+' || *ite == '-')
        {
            ss << *ite;
            ++ite;
        }
        if (!isDigit(*ite))
        {
            ite = cpy;
            return false;
        }
        while (isDigit(*ite))
        {
            ss << *ite;
            ++ite;
        }
    }
    ss >> opnd;
    EVAL_THROW(ss.fail(), "operand overflow");
    return true;
}

bool TokenList::parseOperator(std::string::const_iterator& ite,
                              const std::string::const_iterator& end,
                              TokenType& ty)
{
    if (ite == end) return false;
    switch (*ite)
    {
        case '+':
            ty = TokenType::ADD;
            break;
        case '-':
            ty = TokenType::SUB;
            break;
        case '*':
            ty = TokenType::MUL;
            break;
        case '/':
            ty = TokenType::DIV;
            break;
        case '^':
            ty = TokenType::POW;
            break;
        case '(':
            ty = TokenType::LPAREN;
            break;
        case ')':
            ty = TokenType::RPAREN;
            break;
        case ',':
            ty = TokenType::COMMA;
            break;
        case '=':
            ty = TokenType::EQ;
            break;
        default:
            return false;
    }
    ++ite;
    return true;
}
bool TokenList::parseSymbol(std::string::const_iterator& ite,
                            const std::string::const_iterator& end,
                            std::string& symbol)
{
    if (ite == end) return false;
    if (!isSymbolStart(*ite)) return false;
    symbol.clear();
    symbol += *ite;
    ++ite;
    while (ite != end && isSymbol(*ite))
    {
        symbol += *ite;
        ++ite;
    }
    return true;
}
TokenList::iterator findParen(const TokenList& tkl,

                              const TokenList::iterator& beg,
                              const TokenList::iterator& end)
{
    int parenCount = 0;
    auto ite = beg;
    for (; ite != end; ++ite)
    {
        if (ite->type == TokenType::LPAREN)
        {
            ++parenCount;
        }
        else if (ite->type == TokenType::RPAREN)
        {
            --parenCount;
            if (parenCount == 0) break;
        }
    }
    return ite;
}

}  // namespace evaluator