#include "evaluator/Context.h"

#include <cmath>
namespace evaluator
{
std::pair<ExprType, operand_t> Context::exec(const std::string& input)
{
    auto tkList = TokenList(input);  // May be changed
    if (tkList.size() > 2 && tkList[0].isSymbol() &&
        tkList[1].type == TokenType::EQ)  // Assigning value to variable
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

operand_t Context::evalExpr(TokenList& tkl, const TokenList::iterator& beg,
                            const TokenList::iterator& end, unsigned int depth)
{
    if (depth > maxRecursionDepth) throw EvalException("stack overflow");
    if (beg == end) throw EvalException("invalid expression");
    if (beg + 1 == end)  // "1", "x"
    {
        if (beg->isOperand()) return beg->getOperand();
        if (beg->isSymbol())
        {
            auto vIte = varTable.find(beg->getSymbol());
            if (vIte != varTable.end()) return vIte->second;
            throw EvalException("undefined symbol");
        }
        throw EvalException("invalid expression");
    }
    if (beg->type == TokenType::LPAREN)  // "(1+2)", "(1+2)*3"
    {
        auto ite = findParen(tkl, beg, end);
        if (ite == end) throw EvalException("parentheses mismatched");
        if (ite + 1 == end) return evalExpr(tkl, beg + 1, ite, depth + 1);
        *ite = Token(evalExpr(tkl, beg + 1, ite, depth + 1));
        return evalExpr(tkl, ite, end, depth + 1);
    }
    if (beg->type == TokenType::SUB)  // "-x", "-(1)", "-(x+1)+3"
    {
        int inParen = 0;
        auto next = beg + 1;
        auto ite = next;
        while (ite != end)
        {
            if (ite->type == TokenType::LPAREN) ++inParen;
            if (ite->type == TokenType::RPAREN) --inParen;
            if (!inParen &&
                (ite->type == TokenType::ADD || ite->type == TokenType::SUB))
            {
                --ite;
                break;
            }
            ++ite;
        }
        if (inParen && ite == end)
            throw EvalException("parantheses mismatched");
        if (ite == end) --ite;

        *ite = Token(-evalExpr(tkl, next, ite + 1, depth + 1));
        return evalExpr(tkl, ite, end, depth + 1);
    }
    if (beg->isSymbol())  // variable or function
    {
        if (isVar(beg, end))
        {
            auto ite = varTable.find(beg->getSymbol());
            if (ite == varTable.end()) throw EvalException("undefined symbol");
            *beg = Token(ite->second);
            return evalExpr(tkl, beg, end, depth + 1);
        }
        else
        {
            auto ite = funcTable.find(beg->getSymbol());
            if (ite == funcTable.end()) throw EvalException("undefined symbol");
            auto rParenIte = findParen(tkl, beg + 1, end);
            if (rParenIte == end) throw EvalException("parantheses mismatched");
            if (rParenIte == end - 1)
                return ite->second.eval(*this, tkl, beg, end, depth + 1);
            *rParenIte = Token(
                ite->second.eval(*this, tkl, beg, rParenIte + 1, depth + 1));
            return evalExpr(tkl, rParenIte, end, depth + 1);
        }
    }

    if (!beg->isOperand()) throw EvalException("invalid expression");

    int minPre = 4;
    TokenList::iterator mainOperatorIte;
    for (auto ite = beg; ite != end; ++ite)
    {
        if (ite->type == TokenType::LPAREN)
        {
            ite = findParen(tkl, ite, end);
            if (ite == end) throw EvalException("parentheses mismatched");
            continue;
        }
        if (isOperator(ite->type) && !isNeg(beg, ite))
        {
            int pre = getOperatorPrecedence(ite->type);
            if (pre <= minPre)
            {
                minPre = pre;
                mainOperatorIte = ite;
            }
        }
    }

    if (minPre == 4) throw EvalException("invalid expression");
    switch (mainOperatorIte->type)
    {
        case TokenType::ADD:
            return evalExpr(tkl, beg, mainOperatorIte, depth + 1) +
                   evalExpr(tkl, mainOperatorIte + 1, end, depth + 1);
        case TokenType::SUB:
            return evalExpr(tkl, beg, mainOperatorIte, depth + 1) -
                   evalExpr(tkl, mainOperatorIte + 1, end, depth + 1);
        case TokenType::MUL:
            return evalExpr(tkl, beg, mainOperatorIte, depth + 1) *
                   evalExpr(tkl, mainOperatorIte + 1, end, depth + 1);
        case TokenType::DIV:
        {
            auto denominator =
                evalExpr(tkl, mainOperatorIte + 1, end, depth + 1);
            if (denominator == operand_zero)
                throw EvalException("division by zero");
            return evalExpr(tkl, beg, mainOperatorIte, depth + 1) / denominator;
        }
        case TokenType::POW:
            return std::pow(evalExpr(tkl, beg, mainOperatorIte, depth + 1),
                            evalExpr(tkl, mainOperatorIte + 1, end, depth + 1));
        default:
            throw EvalException("unsupported expression");
    }

    throw EvalException("unsupported expression");
}

bool Context::DefFunc(const TokenList& tkl)
{
    if (tkl.size() < 6) return false;  // f(x)=x
    if ((!tkl[0].isSymbol()) || (!(tkl[1].type == TokenType::LPAREN)))
        return false;
    bool found = false;
    TokenList::const_iterator rParenIte;
    for (auto ite = tkl.begin() + 3; ite != tkl.end() - 1; ++ite)
    {
        if (ite->type == TokenType::RPAREN && (ite + 1)->type == TokenType::EQ)
        {
            found = true;
            rParenIte = ite;
            break;
        }
    }
    if (!found) return false;
    auto ite = tkl.begin() + 2;
    std::unordered_map<std::string, int> parameterMap;

    size_t idx = 0;
    while (ite != rParenIte)
    {
        if (!ite->isSymbol()) return false;
        if (parameterMap.find(ite->getSymbol()) != parameterMap.end())
            throw EvalException("repeated parameter name");
        parameterMap[ite->getSymbol()] = idx;
        ++idx;
        ++ite;
        if (ite == rParenIte) break;
        if (ite->type != TokenType::COMMA) return false;
        ++ite;
    }
    Function f(tkl.begin()->getSymbol(), parameterMap.size(), rParenIte + 2,
               tkl.end());

    for (auto ite = f.tkList.begin(); ite != f.tkList.end(); ++ite)
    {
        if (ite->isSymbol())
        {
            auto i = parameterMap.find(ite->getSymbol());
            if (parameterMap.end() != i)
            {
                f.parameterTable[i->second].push_back(ite - f.tkList.begin());
            }
        }
    }
    funcTable[f.name] = f;
    return true;
}

void Context::importMath()
{
    varTable["pi"] = 3.1415926535898;
    varTable["e"] = 2.718281828459;

    funcTable["ln"] = Function(FuncType::ORDINARY,
                               [](TokenList& tkl, Context&) -> operand_t
                               { return log(tkl[0].getOperand()); });

    funcTable["lg"] = Function(FuncType::ORDINARY,
                               [](TokenList& tkl, Context&) -> operand_t
                               { return log10(tkl[0].getOperand()); });

    funcTable["log"] = Function(
        FuncType::ORDINARY,
        [](TokenList& tkl, Context&) -> operand_t
        { return log(tkl[1].getOperand()) / log(tkl[0].getOperand()); });

    funcTable["sin"] = Function(FuncType::ORDINARY,
                                [](TokenList& tkl, Context&) -> operand_t
                                { return sin(tkl[0].getOperand()); });
    funcTable["cos"] = Function(FuncType::ORDINARY,
                                [](TokenList& tkl, Context&) -> operand_t
                                { return cos(tkl[0].getOperand()); });
    funcTable["tan"] = Function(FuncType::ORDINARY,
                                [](TokenList& tkl, Context&) -> operand_t
                                { return tan(tkl[0].getOperand()); });
    funcTable["abs"] = Function(FuncType::ORDINARY,
                                [](TokenList& tkl, Context&) -> operand_t
                                { return abs(tkl[0].getOperand()); });

    funcTable["max"] = Function(FuncType::ORDINARY,
                                [](TokenList& tkl, Context&) -> operand_t
                                {
                                    operand_t m = tkl[0].getOperand();
                                    for (const auto& t : tkl)
                                        if (t.getOperand() > m)
                                            m = t.getOperand();
                                    return m;
                                });
    funcTable["min"] = Function(FuncType::ORDINARY,
                                [](TokenList& tkl, Context&) -> operand_t
                                {
                                    operand_t m = tkl[0].getOperand();
                                    for (const auto& t : tkl)
                                        if (t.getOperand() < m)
                                            m = t.getOperand();
                                    return m;
                                });

    funcTable["SUM"] = Function(
        FuncType::HIGH_ORDER,
        [](TokenList& tkl, Context& context) -> operand_t
        {
            size_t inParen = 0;
            TokenList exprTokens;
            auto ite = tkl.begin();
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->type == TokenType::LPAREN)
                    ++inParen;
                else if (ite->type == TokenType::RPAREN)
                    --inParen;
                else if (!inParen && ite->type == TokenType::COMMA)
                    break;
                exprTokens.push_back(*ite);
            }
            ++ite;

            std::string dummyVar = ite->getSymbol();
            for (auto& t : exprTokens)
            {
                if (t.isSymbol() && t.getSymbol() == dummyVar)
                    t = Token("#" + t.getSymbol());  // temp variable
            }
            dummyVar = "#" + dummyVar;
            ++ite;

            if (ite->type != TokenType::COMMA)
                throw EvalException("wrong number of arguments");
            ++ite;

            TokenList begExpr;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->type == TokenType::LPAREN)
                    ++inParen;
                else if (ite->type == TokenType::RPAREN)
                    --inParen;
                else if (!inParen && ite->type == TokenType::COMMA)
                    break;
                begExpr.push_back(*ite);
            }
            operand_t beg =
                context.evalExpr(begExpr, begExpr.begin(), begExpr.end());
            ++ite;

            TokenList endExpr;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->type == TokenType::LPAREN)
                    ++inParen;
                else if (ite->type == TokenType::RPAREN)
                    --inParen;
                if (ite->type == TokenType::RPAREN ||
                    (!inParen && ite->type == TokenType::COMMA))
                    break;
                endExpr.push_back(*ite);
            }
            operand_t end =
                context.evalExpr(endExpr, endExpr.begin(), endExpr.end());

            operand_t step;
            TokenList stepExpr;
            if (ite->type == TokenType::COMMA)
            {
                ++ite;
                for (; ite != tkl.end(); ++ite)
                {
                    if (!inParen && ite->type == TokenType::RPAREN) break;
                    if (ite->type == TokenType::LPAREN)
                        ++inParen;
                    else if (ite->type == TokenType::RPAREN)
                        --inParen;
                    stepExpr.push_back(*ite);
                }
                step = context.evalExpr(stepExpr, stepExpr.begin(),
                                        stepExpr.end());
            }
            else
                step = beg > end ? -operand_one : operand_one;
            operand_t s = operand_zero;

            if ((end - beg) * step < operand_zero)
                throw EvalException("infinite loop");

            if (beg < end)
                for (operand_t x = beg; x < end; x += step)
                {
                    context.varTable[dummyVar] = x;
                    auto cpy = exprTokens;
                    s += context.evalExpr(cpy, cpy.begin(), cpy.end());
                }
            else
                for (operand_t x = beg; x > end; x += step)
                {
                    context.varTable[dummyVar] = x;
                    auto cpy = exprTokens;
                    s += context.evalExpr(cpy, cpy.begin(), cpy.end());
                }
            context.varTable.erase(dummyVar);
            return s;
        });

    funcTable["MUL"] = Function(
        FuncType::HIGH_ORDER,
        [](TokenList& tkl, Context& context) -> operand_t
        {
            size_t inParen = 0;
            TokenList exprTokens;
            auto ite = tkl.begin();
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->type == TokenType::LPAREN)
                    ++inParen;
                else if (ite->type == TokenType::RPAREN)
                    --inParen;
                else if (!inParen && ite->type == TokenType::COMMA)
                    break;
                exprTokens.push_back(*ite);
            }
            ++ite;

            std::string dummyVar = ite->getSymbol();
            for (auto& t : exprTokens)
            {
                if (t.isSymbol() && t.getSymbol() == dummyVar)
                    t = Token("#" + t.getSymbol());  // temp variable
            }
            dummyVar = "#" + dummyVar;
            ++ite;

            if (ite->type != TokenType::COMMA)
                throw EvalException("wrong number of arguments");
            ++ite;

            TokenList begExpr;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->type == TokenType::LPAREN)
                    ++inParen;
                else if (ite->type == TokenType::RPAREN)
                    --inParen;
                else if (!inParen && ite->type == TokenType::COMMA)
                    break;
                begExpr.push_back(*ite);
            }
            operand_t beg =
                context.evalExpr(begExpr, begExpr.begin(), begExpr.end());
            ++ite;

            TokenList endExpr;
            for (; ite != tkl.end(); ++ite)
            {
                if (ite->type == TokenType::LPAREN)
                    ++inParen;
                else if (ite->type == TokenType::RPAREN)
                    --inParen;
                if (ite->type == TokenType::RPAREN ||
                    (!inParen && ite->type == TokenType::COMMA))
                    break;
                endExpr.push_back(*ite);
            }
            operand_t end =
                context.evalExpr(endExpr, endExpr.begin(), endExpr.end());

            operand_t step;
            TokenList stepExpr;
            if (ite->type == TokenType::COMMA)
            {
                ++ite;
                for (; ite != tkl.end(); ++ite)
                {
                    if (!inParen && ite->type == TokenType::RPAREN) break;
                    if (ite->type == TokenType::LPAREN)
                        ++inParen;
                    else if (ite->type == TokenType::RPAREN)
                        --inParen;
                    stepExpr.push_back(*ite);
                }
                step = context.evalExpr(stepExpr, stepExpr.begin(),
                                        stepExpr.end());
            }
            else
                step = beg > end ? -operand_one : operand_one;
            operand_t s = operand_one;

            if ((end - beg) * step < operand_zero)
                throw EvalException("infinite loop");

            if (beg < end)
                for (operand_t x = beg; x < end; x += step)
                {
                    context.varTable[dummyVar] = x;
                    auto cpy = exprTokens;
                    s *= context.evalExpr(cpy, cpy.begin(), cpy.end());
                }
            else
                for (operand_t x = beg; x > end; x += step)
                {
                    context.varTable[dummyVar] = x;
                    auto cpy = exprTokens;
                    s *= context.evalExpr(cpy, cpy.begin(), cpy.end());
                }
            context.varTable.erase(dummyVar);
            return s;
        });
}
}  // namespace evaluator