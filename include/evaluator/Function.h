#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <functional>
#include <list>

#include "evaluator/EvaluatorDefs.h"
#include "evaluator/Tokenizer.h"

namespace eval
{
class Context;
enum class FuncType
{
    CUSTOM,
    ORDINARY,
    HIGH_ORDER,
};

class Function
{
   public:
    std::vector<std::list<size_t>> parameterTable;
    FuncType type;
    TokenList tkList;
    std::function<operand_t(const TokenList&, Context&)> definition;

    Function() = default;
    Function(const Function&) = default;

    Function(FuncType t, decltype(definition) def);
    Function(const TokenList::const_iterator& beg,
             const TokenList::const_iterator& end);

    void setArguments(const TokenList& args, TokenList& tkl);

    operand_t eval(Context& context, const TokenList& tkl,
                   const TokenList::const_iterator& beg,
                   const TokenList::const_iterator& end, unsigned int depth);
};
}  // namespace eval

#endif