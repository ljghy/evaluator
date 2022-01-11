#include <fstream>
#include <iostream>

#include <evaluator/Context.h>
std::unordered_map<std::string, std::function<void(eval::Context &)>>
    commandTable{{"exit", [](eval::Context &)
                  { exit(0); }},
                 {"math", [](eval::Context &context)
                  { context.importMath(); }},
                 {"list",
                  [](eval::Context &context)
                  {
                      std::cout << " - Variables:\n";
                      for (const auto &p : context.varTable)
                          std::cout << '\t' << p.first << " = " << p.second
                                    << '\n';
                      std::cout << " - Functions:\n";
                      std::cout << '\t';
                      for (const auto &p : context.funcTable)
                          std::cout << p.first << ", ";
                      std::cout << '\n';
                  }}

    };

void process(const std::string &input, eval::Context &context,
             std::vector<std::string> &record)
{
    if (input.empty())
        return;
    if (input[0] == '!')
    {
        auto cmd = input.substr(1);
        while (cmd.back() == ' ' || cmd.back() == '\n' || cmd.back() == '\t' ||
               cmd.back() == '\r')
            cmd.pop_back();
        if (cmd.substr(0, 4) == "save")
        {
            std::string path = cmd.substr(5);
            std::ofstream os(path);
            if (!os)
            {
                std::cout << "failed to save to file " << path << '\n';
                return;
            }
            for (const auto &s : record)
                os << s << '\n';
            std::cout << "saved to " << path << '\n';
            os.close();
            return;
        }
        auto ite = commandTable.find(cmd);
        if (ite != commandTable.end())
        {
            (ite->second)(context);
            record.push_back(input);
        }
        else
            std::cout << "unknown command\n";
    }
    else
    {
        try
        {
            record.push_back(input);
            auto ret = context.exec(input);
            if (ret.first == eval::ExprType::EXPR)
                std::cout << " = " << ret.second << '\n';
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            record.pop_back();
        }
    }
}

int main(int argc, char *argv[])
{
    eval::Context context;
    std::vector<std::string> record;
    std::cout << std::fixed;
    if (argc == 2)
    {
        std::string path(argv[1]);
        std::ifstream is(path);
        if (!is)
        {
            std::cout << "failed to load file " << path << '\n';
            return 0;
        }
        while (!is.eof())
        {
            std::string input;
            std::getline(is, input);
            std::cout << "eval> " << input << '\n';
            process(input, context, record);
        }
        is.close();
    }

    while (true)
    {
        std::cout << "eval> ";
        std::string input;
        std::getline(std::cin, input);
        process(input, context, record);
    }
    return 0;
}