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

class EvalException : public std::runtime_error
{
   public:
    EvalException(const char* _what) : std::runtime_error(_what) {}
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
