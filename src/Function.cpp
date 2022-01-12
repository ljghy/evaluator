#include <evaluator/Function.h>

#include <evaluator/Context.h>
namespace eval
{
Function::Function(FuncType t, decltype(definition) def)
    : type(t), definition(def)
{
}

Function::Function(const TokenList::const_iterator& beg,
                   const TokenList::const_iterator& end)
    : type(FuncType::CUSTOM), tkList(beg, end)
{
}

void Function::setArguments(const TokenList& args, TokenList& tkl)
{
    EVAL_THROW(type == FuncType::CUSTOM && args.size() != parameterTable.size(),
               EVAL_WRONG_NUMBER_OF_ARGS);
    for (size_t i = 0; i < args.size(); ++i)
        for (auto& idx : parameterTable[i]) tkl[idx] = args[i];
}

operand_t Function::eval(Context& context,
                         const TokenList::const_iterator& beg,
                         const TokenList::const_iterator& end)
{
    if (type == FuncType::HIGH_ORDER)
        return definition(TokenList(beg + 2, end), context);

    TokenList args;
    args.reserve(parameterTable.size() + 1);

    auto start = beg + 2;
    for (auto ite = beg + 2; ite != end; ++ite)
    {
        ite = findArgSep(start, end);
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
                EVAL_THROW(fIte == context.funcTable.end(),
                           EVAL_UNDEFINED_SYMBOL);
                args.push_back(Token(symbol));
            }
        }
        else
            args.push_back(Token(context.evalExpr(start, ite)));
        start = ite + 1;
    }

    if (type == FuncType::ORDINARY) return definition(args, context);
    TokenList cpy(tkList);
    setArguments(args, cpy);
    return context.evalExpr(cpy.begin(), cpy.end());
}
}  // namespace eval