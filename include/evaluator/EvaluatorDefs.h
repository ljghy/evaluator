#ifndef EVALUATOR_DEFS_H_
#define EVALUATOR_DEFS_H_

#include <stdexcept>

namespace evaluator
{
using int_t = int;
using decimal_t = long double;
using operand_t = decimal_t;
constexpr operand_t operand_zero = 0;
constexpr operand_t operand_one = 1;

constexpr unsigned int maxRecursionDepth = 1024;

enum EVAL_EXCEPTION
{
    EVAL_INVALID_EXPR = 0,
    EVAL_UNDEFINED_SYMBOL,
    EVAL_PAREN_MISMATCH,
    EVAL_DIV_BY_ZERO,
    EVAL_STACK_OVERFLOW,
    EVAL_REPEATED_PARAMETER_NAME,
    EVAL_WRONG_NUMBER_OF_ARGS,
    EVAL_INFINITE_LOOP,
    EVAL_UNEXPECTED_TOKEN_TYPE,
    EVAL_PARSE_FAILED,
    EVAL_OPERAND_OVERFLOW,
};

static const char* EVAL_EXCEPTION_MSG[]{"invalid expression",
                                        "undefined symbol",
                                        "parentheses mismatched",
                                        "division by zero",
                                        "stack overflow",
                                        "repeated parameter name",
                                        "wrong number of arguments",
                                        "infinite loop",
                                        "unexpected token type",
                                        "parse failed",
                                        "operand overflow"};

class EvalException : public std::runtime_error
{
   public:
    EVAL_EXCEPTION code;
    EvalException(const EVAL_EXCEPTION& e)
        : std::runtime_error(EVAL_EXCEPTION_MSG[e]), code(e)
    {
    }
};

#define EVAL_DO_TYPE_CHECK

#ifndef EVAL_NO_THROW
#define EVAL_THROW(cond, msg)               \
    do                                      \
    {                                       \
        if (cond) throw EvalException(msg); \
    } while (0)
#else
#define EVAL_THROW(cond, msg)
#endif
}  // namespace evaluator

#endif
