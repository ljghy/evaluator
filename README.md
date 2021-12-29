# Expression evaluator in C++17

## Build

```
mkdir build
cd build
```

#### Windows MSVC

```
cmake ..
cmake --build . --config [Debug|Release]
```

#### Windows MinGW

```
cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=[Debug|Release] ..
mingw32-make
```

#### Linux GCC

```
cmake -DCMAKE_BUILD_TYPE=[Debug|Release] ..
make 
```

## Example

#### main.cpp

```c++
#include <iostream>
#include "evaluator/Context.h"
int main()
{
	eval::Context context;
	context.exec("a = 1");
	context.exec("f(x, y) = x ^ y + 1");
	auto result = context.exec("f(3, 2)");
	std::cout << "f(3, 2) = " << result.second << '\n'; // 10
	
	context.importMath();

	context.exec("f(n) = SUM(sin(x)/x, x, 1, n)");
	result = context.exec("f(1e6)");
	std::cout << "f(1e6) = " << result.second << '\n'; // 1.0708
    
	result = context.exec("(pi - 1) / 2");
	std::cout << "(pi - 1) / 2 = " << result.second << '\n'; // 1.0708
	
	context.exec("fib(n) = geq(n, 2) * (fib(n - 1) + fib(n - 2)) + lt(n, 2)");
	result = context.exec("fib(10)");
	std::cout << "fib(10) = " << result.second << '\n'; // 89
}
```

#### compile and run

```
./
├─include/evaluator┬─Context.h
│                  ├─EvaluatorDefs.h
│                  ├─Function.h
│                  └─Tokenizer.h
├─lib/libevaluator.a
└─main.cpp

g++ -Wall -O2 main.cpp -o main -Iinclude -Llib -levaluator -std=gnu++17
./main
```
