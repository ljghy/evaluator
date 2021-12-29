#include "evaluator/Context.h"

#include <cmath>
#include <cstdlib>
#include <ctime>
namespace evaluator
{
Context::Context()
{
    varTable["ANS"] = operand_zero;
    srand(time(0));
}

std::pair<ExprType, operand_t> Context::exec(const std::string& input)
{
    auto tkList = TokenList(input);
    if (tkList.size() > 2 && tkList[0].isSymbol() &&
        tkList[1].isEq())  // Assigning value to variable
    {
        varTable[tkList.begin()->getSymbol()] =
            evalExpr(tkList, tkList.begin() + 2, tkList.end());
        return {ExprType::VAR_ASSIGN, operand_zero};
    }
    if (DefFunc(tkList))
    {
        return {ExprType::FUNC_DEF, operand_zero};
    }
    return {ExprType::EXPR,
            varTable["ANS"] = evalExpr(tkList, tkList.begin(), tkList.end())};
}

operand_t Context::evalExpr(const TokenList& tkl,
                            const TokenList::const_iterator& beg,
                            const TokenList::const_iterator& end,
                            unsigned int depth)
{
    EVAL_THROW(depth > maxRecursionDepth, EVAL_STACK_OVERFLOW);
    EVAL_THROW(beg == end, EVAL_INVALID_EXPR);

    if (beg + 1 == end)  // "1", "x"
    {
        if (beg->isOperand()) return beg->getOperand();
        EVAL_THROW(!beg->isSymbol(), EVAL_INVALID_EXPR);
        auto vIte = varTable.find(beg->getSymbol());
        EVAL_THROW(vIte == varTable.end(), EVAL_UNDEFINED_SYMBOL);
        return vIte->second;
    }
    int minPre = 4;
    TokenList::const_iterator mainOperatorIte;

    if (beg->isSub())  // "-x", "-(1)", "-(x+1)+3"
    {
        int inParen = 0;
        auto ite = beg + 1;
        for (; ite != end; ++ite)
        {
            if (ite->isLParen())
                ++inParen;
            else if (ite->isRParen())
                --inParen;
            else if (!inParen && (ite->isAdd() || ite->isSub()))
                break;
        }
        if (ite == end)
        {
            EVAL_THROW(inParen, EVAL_PAREN_MISMATCH);
            return -evalExpr(tkl, beg + 1, end, depth + 1);
        }
        minPre = 1;
        mainOperatorIte = ite;
    }

    if (minPre == 4)
    {
        for (auto ite = beg; ite != end; ++ite)
        {
            if (ite->isLParen())
            {
                ite = findParen(ite, end);
                EVAL_THROW(ite == end, EVAL_PAREN_MISMATCH);
                continue;
            }
            if (ite->isOperator() && !isNeg(beg, ite))
            {
                int pre = getOperatorPrecedence(ite->type);
                if (pre <= minPre)
                {
                    minPre = pre;
                    mainOperatorIte = ite;
                }
            }
        }
    }
    if (minPre != 4)
    {
        switch (mainOperatorIte->type)
        {
            case TokenType::ADD:
                return evalExpr(tkl, beg, mainOperatorIte, depth + 1) +
                       evalExpr(tkl, mainOperatorIte + 1, end, depth + 1);
            case TokenType::SUB:
                return evalExpr(tkl, beg, mainOperatorIte, depth + 1) -
                       evalExpr(tkl, mainOperatorIte + 1, end, depth + 1);
            case TokenType::MUL:
            {
                auto l = evalExpr(tkl, beg, mainOperatorIte, depth + 1);
                if (l == operand_zero) return operand_zero;
                return l * evalExpr(tkl, mainOperatorIte + 1, end, depth + 1);
            }
            case TokenType::DIV:
            {
                auto denominator =
                    evalExpr(tkl, mainOperatorIte + 1, end, depth + 1);
                EVAL_THROW(denominator == operand_zero, EVAL_DIV_BY_ZERO);
                return evalExpr(tkl, beg, mainOperatorIte, depth + 1) /
                       denominator;
            }
            case TokenType::POW:
                return std::pow(
                    evalExpr(tkl, beg, mainOperatorIte, depth + 1),
                    evalExpr(tkl, mainOperatorIte + 1, end, depth + 1));
            default:
                EVAL_THROW(1, EVAL_INVALID_EXPR);
        }
    }
    if (beg->isLParen())  // "(1+2)", "(1+2)*3"
    {
        EVAL_THROW(!(end - 1)->isRParen(), EVAL_PAREN_MISMATCH);
        return evalExpr(tkl, beg + 1, end - 1, depth + 1);
    }

    auto fIte = funcTable.find(beg->getSymbol());
    EVAL_THROW(fIte == funcTable.end(), EVAL_UNDEFINED_SYMBOL);
    EVAL_THROW(!(end - 1)->isRParen(), EVAL_PAREN_MISMATCH);
    return fIte->second.eval(*this, tkl, beg, end, depth + 1);
}

bool Context::DefFunc(const TokenList& tkl)
{
    if (tkl.size() < 6) return false;  // f(x)=x
    if ((!tkl[0].isSymbol()) || (!(tkl[1].isLParen()))) return false;
    bool foundEq = false;
    TokenList::const_iterator rParenIte;
    for (auto ite = tkl.begin() + 3; ite != tkl.end() - 1; ++ite)
        if (ite->isRParen() && (ite + 1)->isEq())
        {
            foundEq = true;
            rParenIte = ite;
            break;
        }
    if (!foundEq) return false;
    std::unordered_map<std::string, int> parameterMap;

    size_t idx = 0;
    for (auto ite = tkl.begin() + 2; ite != rParenIte; ++ite)
    {
        if (!ite->isSymbol()) return false;
        EVAL_THROW(parameterMap.find(ite->getSymbol()) != parameterMap.end(),
                   EVAL_REPEATED_PARAMETER_NAME);
        parameterMap[ite->getSymbol()] = idx++;
        if (++ite == rParenIte) break;
        if (!ite->isComma()) return false;
    }
    Function f(rParenIte + 2, tkl.end());
    f.parameterTable.resize(parameterMap.size());
    for (auto ite = f.tkList.begin(); ite != f.tkList.end(); ++ite)
    {
        if (ite->isSymbol())
        {
            auto i = parameterMap.find(ite->getSymbol());
            if (parameterMap.end() != i)
                f.parameterTable[i->second].push_back(ite - f.tkList.begin());
        }
    }
    funcTable[tkl.begin()->getSymbol()] = f;
    return true;
}

void Context::importMath()
{
#ifdef EVAL_DECIMAL_OPERAND
    varTable["pi"] = 3.14159265358979323846264338328;
    varTable["e"] = 2.71828182845904523536028747135;
#endif

    funcTable["eq"] =
        Function(FuncType::ORDINARY,
                 [](const TokenList& tkl, Context&) -> operand_t
                 { return tkl[0].getOperand() == tkl[1].getOperand(); });

    funcTable["neq"] =
        Function(FuncType::ORDINARY,

                 [](const TokenList& tkl, Context&) -> operand_t
                 { return tkl[0].getOperand() != tkl[1].getOperand(); });

    funcTable["leq"] =
        Function(FuncType::ORDINARY,
                 [](const TokenList& tkl, Context&) -> operand_t
                 { return tkl[0].getOperand() <= tkl[1].getOperand(); });

    funcTable["lt"] =
        Function(FuncType::ORDINARY,
                 [](const TokenList& tkl, Context&) -> operand_t
                 { return tkl[0].getOperand() < tkl[1].getOperand(); });

    funcTable["geq"] =
        Function(FuncType::ORDINARY,
                 [](const TokenList& tkl, Context&) -> operand_t
                 { return tkl[0].getOperand() >= tkl[1].getOperand(); });

    funcTable["gt"] =
        Function(FuncType::ORDINARY,
                 [](const TokenList& tkl, Context&) -> operand_t
                 { return tkl[0].getOperand() > tkl[1].getOperand(); });

#ifdef EVAL_DECIMAL_OPERAND
    funcTable["ln"] = Function(FuncType::ORDINARY,
                               [](const TokenList& tkl, Context&) -> operand_t
                               { return log(tkl[0].getOperand()); });

    funcTable["lg"] = Function(FuncType::ORDINARY,
                               [](const TokenList& tkl, Context&) -> operand_t
                               { return log10(tkl[0].getOperand()); });

    funcTable["log"] = Function(
        FuncType::ORDINARY,
        [](const TokenList& tkl, Context&) -> operand_t
        { return log(tkl[1].getOperand()) / log(tkl[0].getOperand()); });

    funcTable["sin"] = Function(FuncType::ORDINARY,
                                [](const TokenList& tkl, Context&) -> operand_t
                                { return sin(tkl[0].getOperand()); });
    funcTable["cos"] = Function(FuncType::ORDINARY,
                                [](const TokenList& tkl, Context&) -> operand_t
                                { return cos(tkl[0].getOperand()); });
    funcTable["tan"] = Function(FuncType::ORDINARY,
                                [](const TokenList& tkl, Context&) -> operand_t
                                { return tan(tkl[0].getOperand()); });

    funcTable["asin"] = Function(FuncType::ORDINARY,
                                 [](const TokenList& tkl, Context&) -> operand_t
                                 { return asin(tkl[0].getOperand()); });
    funcTable["acos"] = Function(FuncType::ORDINARY,
                                 [](const TokenList& tkl, Context&) -> operand_t
                                 { return acos(tkl[0].getOperand()); });
    funcTable["atan"] = Function(FuncType::ORDINARY,
                                 [](const TokenList& tkl, Context&) -> operand_t
                                 { return atan(tkl[0].getOperand()); });

    funcTable["gamma"] =
        Function(FuncType::ORDINARY,
                 [](const TokenList& tkl, Context&) -> operand_t
                 { return tgamma(tkl[0].getOperand()); });

    funcTable["floor"] =
        Function(FuncType::ORDINARY,
                 [](const TokenList& tkl, Context&) -> operand_t
                 { return floor(tkl[0].getOperand()); });
    funcTable["ceil"] = Function(FuncType::ORDINARY,
                                 [](const TokenList& tkl, Context&) -> operand_t
                                 { return ceil(tkl[0].getOperand()); });
    funcTable["exp"] = Function(FuncType::ORDINARY,
                                [](const TokenList& tkl, Context&) -> operand_t
                                { return exp(tkl[0].getOperand()); });
    funcTable["erf"] = Function(FuncType::ORDINARY,
                                [](const TokenList& tkl, Context&) -> operand_t
                                { return erf(tkl[0].getOperand()); });
#endif

    funcTable["abs"] = Function(FuncType::ORDINARY,
                                [](const TokenList& tkl, Context&) -> operand_t
                                {
#ifdef EVAL_DECIMAL_OPERAND
                                    return fabs(tkl[0].getOperand());
#else
                                    return abs(tkl[0].getOperand());
#endif
                                });

    funcTable["rand"] = Function(FuncType::ORDINARY,
                                 [](const TokenList& tkl, Context&) -> operand_t
                                 {
                                     auto a = tkl[0].getOperand(),
                                          b = tkl[1].getOperand();
#ifdef EVAL_DECIMAL_OPERAND
                                     return a + rand() * (b - a) / RAND_MAX;
#else
                                    return a + rand()%(b-a);
#endif
                                 });

    funcTable["max"] = Function(FuncType::ORDINARY,
                                [](const TokenList& tkl, Context&) -> operand_t
                                {
                                    operand_t m = tkl[0].getOperand();
                                    for (const auto& t : tkl)
                                        if (t.getOperand() > m)
                                            m = t.getOperand();
                                    return m;
                                });
    funcTable["min"] = Function(FuncType::ORDINARY,
                                [](const TokenList& tkl, Context&) -> operand_t
                                {
                                    operand_t m = tkl[0].getOperand();
                                    for (const auto& t : tkl)
                                        if (t.getOperand() < m)
                                            m = t.getOperand();
                                    return m;
                                });

    funcTable["SUM"] = Function(
        FuncType::HIGH_ORDER,
        [](const TokenList& tkl, Context& context) -> operand_t
        {
            int inParen = 0;
            auto ite = tkl.begin(), exprIte = ite;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->isLParen())
                    ++inParen;
                else if (ite->isRParen())
                    --inParen;
                else if (!inParen && ite->isComma())
                    break;
            }
            TokenList exprTokens(exprIte, ite);

            std::string dummyVar = (++ite)->getSymbol();
            for (auto& t : exprTokens)
                if (t.isSymbol() && t.getSymbol() == dummyVar)
                    t = Token("#" + t.getSymbol());  // temp variable
            dummyVar = "#" + dummyVar;

            EVAL_THROW(!(++ite)->isComma(), EVAL_WRONG_NUMBER_OF_ARGS);

            auto begIte = ++ite;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->isLParen())
                    ++inParen;
                else if (ite->isRParen())
                    --inParen;
                else if (!inParen && ite->isComma())
                    break;
            }
            operand_t beg = context.evalExpr(tkl, begIte, ite);

            auto endIte = ++ite;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->isLParen())
                    ++inParen;
                else if (ite->isRParen())
                    --inParen;
                if (ite->isRParen() || (!inParen && ite->isComma())) break;
            }
            operand_t end = context.evalExpr(tkl, endIte, ite);

            operand_t step;
            if (ite->isComma())
            {
                auto stepIte = ++ite;
                for (; ite != tkl.end(); ++ite)
                {
                    if (!inParen && ite->isRParen()) break;
                    if (ite->isLParen())
                        ++inParen;
                    else if (ite->isRParen())
                        --inParen;
                }
                step = context.evalExpr(tkl, stepIte, ite);
            }
            else
                step = beg > end ? -operand_one : operand_one;
            operand_t s = operand_zero;

            EVAL_THROW((end - beg) * step < operand_zero, EVAL_INFINITE_LOOP);

            auto dummyVarIte =
                context.varTable.insert(std::make_pair(dummyVar, operand_zero))
                    .first;
            if (beg < end)
                for (operand_t x = beg; x < end; x += step)
                {
                    dummyVarIte->second = x;
                    s += context.evalExpr(exprTokens, exprTokens.begin(),
                                          exprTokens.end());
                }
            else
                for (operand_t x = beg; x > end; x += step)
                {
                    dummyVarIte->second = x;
                    s += context.evalExpr(exprTokens, exprTokens.begin(),
                                          exprTokens.end());
                }
            context.varTable.erase(dummyVar);
            return s;
        });

    funcTable["MUL"] = Function(
        FuncType::HIGH_ORDER,
        [](const TokenList& tkl, Context& context) -> operand_t
        {
            int inParen = 0;
            auto ite = tkl.begin(), exprIte = ite;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->isLParen())
                    ++inParen;
                else if (ite->isRParen())
                    --inParen;
                else if (!inParen && ite->isComma())
                    break;
            }
            TokenList exprTokens(exprIte, ite);

            std::string dummyVar = (++ite)->getSymbol();
            for (auto& t : exprTokens)
                if (t.isSymbol() && t.getSymbol() == dummyVar)
                    t = Token("#" + t.getSymbol());  // temp variable
            dummyVar = "#" + dummyVar;

            EVAL_THROW(!(++ite)->isComma(), EVAL_WRONG_NUMBER_OF_ARGS);

            auto begIte = ++ite;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->isLParen())
                    ++inParen;
                else if (ite->isRParen())
                    --inParen;
                else if (!inParen && ite->isComma())
                    break;
            }
            operand_t beg = context.evalExpr(tkl, begIte, ite);

            auto endIte = ++ite;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->isLParen())
                    ++inParen;
                else if (ite->isRParen())
                    --inParen;
                if (ite->isRParen() || (!inParen && ite->isComma())) break;
            }
            operand_t end = context.evalExpr(tkl, endIte, ite);

            operand_t step;
            if (ite->isComma())
            {
                auto stepIte = ++ite;
                for (; ite != tkl.end(); ++ite)
                {
                    if (!inParen && ite->isRParen()) break;
                    if (ite->isLParen())
                        ++inParen;
                    else if (ite->isRParen())
                        --inParen;
                }
                step = context.evalExpr(tkl, stepIte, ite);
            }
            else
                step = beg > end ? -operand_one : operand_one;
            operand_t s = operand_one;

            EVAL_THROW((end - beg) * step < operand_zero, EVAL_INFINITE_LOOP);

            auto dummyVarIte =
                context.varTable.insert(std::make_pair(dummyVar, operand_zero))
                    .first;
            if (beg < end)
                for (operand_t x = beg; x < end; x += step)
                {
                    dummyVarIte->second = x;
                    s *= context.evalExpr(exprTokens, exprTokens.begin(),
                                          exprTokens.end());
                }
            else
                for (operand_t x = beg; x > end; x += step)
                {
                    dummyVarIte->second = x;
                    s *= context.evalExpr(exprTokens, exprTokens.begin(),
                                          exprTokens.end());
                }
            context.varTable.erase(dummyVar);
            return s;
        });
}
}  // namespace evaluator