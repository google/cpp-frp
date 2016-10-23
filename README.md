# cpp-frp
Functional reactive programming in C++

cpp-frp is a modern multi-threaded lock-free header-only library written in standard C++14 (**or 11?**).

The library contains basic `transform`, `map` and `filter` functionality as shown below:

```C++
auto base = variable(5);
auto exponent = variable(2);

auto squared = transform(
	[](auto base, auto exponent){ return pow(base, exponent); },
	std::ref(base), std::ref(exponent));

auto random_sequence = transform([](auto i) {
		std::vector<int> v(i);
	    std::generate(v.begin(), v.end(), std::rand);
	    return v;
    }, std::ref(squared));

auto filtered = filter([](auto i) { return i % 2; }, std::ref(random_sequence));

auto strings = map([](auto i) { return std::to_string(i); }, std::ref(filtered));

auto print = transform([](auto strings) {
		std::copy(std::begin(strings), std::end(strings),
			std::ostream_iterator<std::string>(std::cout, " "))
	}, std::ref(strings));

// Update base and exponent repositories with new values.
// changes will propagate through the graph where necessary.
base = 6;
exponent = 3;

```

The above example executes the lambda expressions on the current thread, to set an executor to use:

```C++

executor_type executor;

auto receiver = transform(execute_on(executor, [](auto i){}, std::ref(provider));
```

## Type requirements

The value types used with the operators should be *copyable* and implement the comparator ```auto T::operator==(const T &) const``` or equivalent.

----------
Functions should implement the ```operator()``` with the argument types relevant. Lambda expressions with ```auto``` type deductions are allowed as seen above. ```std::bind```, function pointers etc works as well. (**Double check this!**)

Functions can return ```void``` for ```transform```, the function will be executed on any dependency changes but the resulting repository becomes a leaf-node and can not be used by any other repository.

### Thread safety
Functions are expected to have **no side-effects** and only operate on their input arguments. That said, in order to interface with other code that might be a too strict requirement. This library uses atomics to synchronize logic, which means that in theory, at any point, there might be multiple invocations of your function running in parallell.

There are two ways to manage functions with side effects:
 - Specify a single threaded executor with ```execute_on(...)```. This way the execution of your function will be serialized.
 - Use atomics or locks to protect your data. This way you can write thread-safe code while still leveraging multi-thread performance.

----------

Executor types should have a signature equivalent to:
```C++
template<typename F>
auto operator()(F &&f);
```

The executor is expected to call the ```operator()``` of the given instance with no arguments.