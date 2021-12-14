#include <iostream>

#include "evaluator/Context.h"
std::unordered_map<std::string, std::function<void(evaluator::Context&)>>
    commandTable{
        {"exit", [](evaluator::Context&) { exit(0); }},
        {"math", [](evaluator::Context& context) { context.importMath(); }},
        {"view", [](evaluator::Context& context)
         {
             std::cout << "Variables:\n";
             for (const auto& p : context.varTable)
                 std::cout << '\t' << p.first << " = " << p.second << '\n';
             std::cout << "Functions:\n";
             std::cout << '\t';
             for (const auto& p : context.funcTable)
                 std::cout << p.first << ", ";
             std::cout << '\n';
         }}};
int main()
{
    evaluator::Context context;
    while (true)
    {
        std::cout << "eval> ";
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) continue;
        if (input[0] == '!')
        {
            auto cmd = input.substr(1);
            auto ite = commandTable.find(cmd);
            if (ite != commandTable.end())
                (ite->second)(context);
            else
                std::cout << "unknown command\n";
        }
        else
        {
            try
            {
                auto ret = context.exec(input);
                if (ret.first == evaluator::ExprType::EXPR)
                    std::cout << ret.second << '\n';
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }
        }
    }
    return 0;
}