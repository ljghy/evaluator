#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <functional>
#include <list>

#include "evaluator/EvaluatorDefs.h"
#include "evaluator/Tokenizer.h"

namespace evaluator
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
    std::string name;
    std::vector<std::list<size_t>> parameterTable;
    TokenList tkList;
    FuncType type;
    std::function<operand_t(TokenList&, Context&)> definition;

    Function() = default;

    Function(FuncType t, decltype(definition) def);

    Function(const std::string& _name, size_t n,
             const TokenList::const_iterator& beg,
             const TokenList::const_iterator& end);

    void setArguments(const TokenList& args);

    operand_t eval(Context& context, TokenList& tkl,
                   const TokenList::iterator& beg,
                   const TokenList::iterator& end, unsigned int depth);
};
}  // namespace evaluator

#endif