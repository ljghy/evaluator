#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <unordered_map>
#include <utility>

#include <evaluator/EvaluatorDefs.h>
#include <evaluator/Function.h>
namespace eval
{
enum class ExprType
{
    EXPR,
    VAR_ASSIGN,
    FUNC_DEF
};

class Context
{
   protected:
    unsigned int depth;

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
        if (ite->type != TokenType::SUB) return false;
        if (ite == beg) return true;
        auto pre = ite - 1;
        return (!pre->isOperand()) && (!pre->isSymbol()) &&
               (pre->type != TokenType::RPAREN);
    }

    operand_t evalExpr(const TokenList::const_iterator& beg,
                       const TokenList::const_iterator& end);

    bool DefFunc(const TokenList& tkl);

   public:
    Context();
    void importMath();

    std::pair<ExprType, operand_t> exec(const std::string& input);

    virtual ~Context() {}
};
}  // namespace eval

#endif