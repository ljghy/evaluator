#include <evaluator/Tokenizer.h>
namespace eval
{
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
        EVAL_THROW(tk.type == TokenType::NONE, EVAL_PARSE_FAILED);
        push_back(tk);
        parseSpace(ite, end_ite);
    }
}

template <>
bool TokenList::parseOperand<int_t>(std::string::const_iterator& ite,
                                    const std::string::const_iterator& end,
                                    int_t& opnd)
{
    return parseInt(ite, end, opnd);
}

template <>
bool TokenList::parseOperand<decimal_t>(std::string::const_iterator& ite,
                                        const std::string::const_iterator& end,
                                        decimal_t& opnd)
{
    return parseDecimal(ite, end, opnd);
}

Token TokenList::parse(std::string::const_iterator& ite,
                       const std::string::const_iterator& end)
{
    operand_t opnd;
    if (parseOperand<operand_t>(ite, end, opnd)) return Token(opnd);
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
                         const std::string::const_iterator& end, int_t& opnd)
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
    EVAL_THROW(ss.fail(), EVAL_OPERAND_OVERFLOW);
    return true;
}
bool TokenList::parseDecimal(std::string::const_iterator& ite,
                             const std::string::const_iterator& end,
                             decimal_t& opnd)
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
    EVAL_THROW(ss.fail(), EVAL_OPERAND_OVERFLOW);
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
TokenList::const_iterator findParen(const TokenList::const_iterator& beg,
                                    const TokenList::const_iterator& end)
{
    int inParen = 0;
    auto ite = beg;
    for (; ite != end; ++ite)
    {
        if (ite->isLParen())
            ++inParen;
        else if (ite->isRParen() && (!(--inParen)))
            return ite;
    }
    return ite;
}
TokenList::const_iterator findArgSep(const TokenList::const_iterator& beg,
                                     const TokenList::const_iterator& end)
{
    int inParen = 0;
    auto ite = beg;
    for (; ite != end; ++ite)
    {
        if (ite->isLParen())
            ++inParen;
        else if (ite->isRParen())
        {
            if (!inParen) return ite;
            --inParen;
        }
        else if (!inParen && ite->isComma())
            return ite;
    }
    return ite;
}
}  // namespace eval