#include "evaluator/Function.h"

#include "evaluator/Context.h"
namespace evaluator
{
void Function::setArguments(const TokenList& args)
{
    if (type == FuncType::CUSTOM && args.size() != parameterTable.size())
        throw EvalException("wrong number of arguments");
    for (size_t i = 0; i < args.size(); ++i)
    {
        for (auto& idx : parameterTable[i])
        {
            tkList[idx] = args[i];
        }
    }
}

operand_t Function::eval(Context& context, TokenList& tkl,
                         const TokenList::iterator& beg,
                         const TokenList::iterator& end, unsigned int depth)
{
    TokenList args;
    args.reserve(end - beg - 2);

    if (type == FuncType::HIGH_ORDER)
    {
        for (auto i = beg + 2; i != end; ++i) args.push_back(*i);
        return definition(args, context);
    }

    auto start = beg + 2;
    for (auto ite = beg + 2; ite != end; ++ite)
    {
        if (ite->type == TokenType::LPAREN)
        {
            ite = findParen(tkl, ite, end);
            continue;
        }

        if (ite->type == TokenType::COMMA || ite->type == TokenType::RPAREN)
        {
            auto len = ite - start;
            if (len == 1 && start->isSymbol())
            {
                std::string symbol = start->getSymbol();
                auto vIte = context.varTable.find(symbol);
                if (vIte != context.varTable.end())
                    args.push_back(Token(vIte->second));
                else
                {
                    auto fIte = context.funcTable.find(symbol);
                    if (fIte != context.funcTable.end())
                        args.push_back(Token(symbol));
                    else
                        throw EvalException("undefined symbol");
                }
            }
            else
                args.push_back(
                    Token(context.evalExpr(tkl, start, ite, depth + 1)));
            start = ite + 1;
        }
    }

    if (type == FuncType::ORDINARY) return definition(args, context);
    setArguments(args);
    auto cpy = tkList;
    return context.evalExpr(cpy, cpy.begin(), cpy.end(), depth + 1);
}
}  // namespace evaluator