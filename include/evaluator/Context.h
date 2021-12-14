#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <unordered_map>
#include <utility>

#include "evaluator/EvaluatorDefs.h"
#include "evaluator/Function.h"

namespace evaluator
{
enum class ExprType
{
    EXPR,
    VAR_ASSIGN,
    FUNC_DEF
};

class Context
{
   public:
    std::unordered_map<std::string, operand_t> varTable;
    std::unordered_map<std::string, Function> funcTable;

    static inline bool isVar(const TokenList::const_iterator& ite,
                             const TokenList::const_iterator& end)
    {
        return ite + 1 == end || (ite + 1)->type != TokenType::LPAREN;
    }

    static inline bool isNeg(const TokenList::const_iterator& beg,
                             const TokenList::const_iterator& ite)
    {
        return ite->type == TokenType::SUB &&
               (ite == beg ||
                ((!(ite - 1)->isOperand()) && (!(ite - 1)->isSymbol())));
    }

    operand_t evalExpr(TokenList& tkl, const TokenList::iterator& beg,
                       const TokenList::iterator& end, unsigned int depth = 0);

    bool DefFunc(const TokenList& tkl);

   public:
    Context() { varTable["ANS"] = operand_zero; }
    void importMath();

    std::pair<ExprType, operand_t> exec(const std::string& input);

    virtual ~Context() {}
};
}  // namespace evaluator

#endif