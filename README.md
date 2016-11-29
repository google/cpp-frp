# cpp-frp
Static functional reactive programming in C++

cpp-frp is a modern multi-threaded lock-free type-safe header-only library written in standard C++14.

The library contains basic `transform`, `map` and `filter` functionality as shown below:

```C++
auto base = source(5);
auto exponent = source(2);

auto squared = transform(
	[](auto base, auto exponent) { return pow(base, exponent); },
	std::ref(base), std::ref(exponent));

auto random_sequence = transform([](auto i) {
	auto v = std::vector<int>(std::size_t(i));
	std::generate(v.begin(), v.end(), std::rand);
	return v;
}, std::ref(squared));

auto filtered = filter([](auto i) { return i % 2; }, std::ref(random_sequence));

auto strings = map([](auto i) { return std::to_string(i); }, std::ref(filtered));

auto print = sink(std::ref(strings));

// read the content of print
auto values(*print);
for (auto value : *values) {
	std::cout << value << " ";
}
std::cout << std::endl;

// Update base and exponent repositories with new values.
// changes will propagate through the graph where necessary.
base = 6;
exponent = 3;
```

The above example executes the lambda expressions on the current thread, to set an executor to use:

```C++

executor_type executor;

auto receiver = transform(execute_on(executor, [](auto i){}), std::ref(provider));
```
##Build and installation instructions
This is a header-only library. Just add ```cpp-frp/include``` as an include directory.
Tested compilers include
 - ```Visual Studio 2015```
 - ```GCC 5, 6```
Note that ```GCC 4``` is not supported.

```cmake``` is used for building and running the tests.

 - ```cmake -G``` to list the available generators.
 - ```cmake .``` to set up projects.
  * With Visual Studio, after execution, ```cpp-frp.sln``` can be found in the root directory.
 - ```cmake --build .``` to set up and build the project.
 - ```ctest -V .``` to run the tests.
 - ```cmake -DENABLE_COVERAGE:BOOL=true .``` to enable test coverage statistics. Available with ```GCC``` and ```clang```.
  * ```./test-coverage.sh``` to generate test coverage statistics. Requires ```lcov```
  * ```gnome-open test-coverage/index.html``` or equivalent to open the generated test coverage data.

##Type requirements
###Value types
The requirements for value types are as follows:

 - Must be *move constructible*
  * If used with ```map_cache``` input type must be *copy constructible*
  * If used with ```filter``` type must be *copy constructible*
 - Unless a custom *comparator* is used with ```transform```, ```filter```, ```map```, ```map_cache``` or ```source```:
  * Implement the equality comparator ```auto T::operator==(const T &) const``` or equivalent.

The *comparator* is used to suppress redundant updates while traversing the graph.

###Function types
Functions must implement the ```operator()``` with the argument types relevant. Lambda expressions with ```auto``` type deductions are allowed as seen above. ```std::bind```, function pointers etc works as well.

Functions can return ```void``` for ```transform```, the function will be executed on any dependency changes but the resulting repository becomes a leaf-node and can not be used by any other repository.

####Thread safety
Functions are expected to have **no side-effects** and only operate on their input arguments. That said, in order to interface with other code that might be a too strict requirement. This library uses atomics to synchronize logic, which means that in theory, at any point, there might be multiple invocations of your function running in parallell. The values returned by functions are guaranteed to arrive in the correct order, the invocation of functions are not.

There are two ways to manage functions with side effects:
 - Specify a single threaded executor with ```execute_on(...)```. This way the execution of your function will be serialized.
 - Use atomics or locks to protect your data. This way you can write thread-safe code while still leveraging multi-thread performance.

###Executor types
Executor types must have a signature equivalent to:
```C++
template<typename F>
auto operator()(F &&f);
```

The executor is expected to call the ```operator()``` of the given instance with no arguments.

###Notes on the "diamond problem"

####Example layout
```
   [A]
  /   \
[B]   [C]
  \   /
   [D]
  /   \
[E]   [F]
  \   /
   [G]
```
####Problem description

 1. ```A``` updates its value.
 2. ```B``` and ```C``` are evaluated independently in parallel.
 3. ```D``` is evaluated using the new value of ```B``` but the old value of ```C``` since it is not yet available.
 4. ```E``` and ```F``` are evaluated independently in parallel using the value of ```D```.
 5. ```G``` is evaluated using the new value of ```E``` but the old value of ```F```.
 6. ```G``` is evaluated again using the new value of ```E``` and ```F```.
 7. ```D``` is evaluated using the new value of ```B``` and ```C```.
 8. ```E``` and ```F``` are evaluated independently in parallel using the value of ```D```.
 9. ```G``` is evaluated using the new value of ```E``` but the old value of ```F```.
 10. ```G``` is evaluated again using the new value of ```E``` and ```F```.

**One single update of ```A``` causes ```G``` to be evaluated 4 times!**
####Solution 1
**Use the same single threaded executor**. The ```cpp-frp``` library uses an internal revision system to suppress redundant updates. Using a single threaded-executor, every evaluation of a node is pushed as a task onto the executor and executed in order (first-in, first-out). This means that the above graph would be evaluated in a **breadth-first** fashion.

 1. ```A``` updates its value and pushes evaluation of ```B``` and ```C``` on the executor queue.
 2. ```B``` is evaluated and pushes the evaluation of ```D``` onto the executor queue.
 3. ```C``` is evaluated and pushes the evaluation of ```D``` onto the executor queue.
 4. ```D``` is evaluated using the new values of ```B``` and ```C```.
 5. The second evaluation of ```D``` is suppressed as its current value if based on the latest revisions of ```B``` and ```C```.
 6. The evaluation continues in the same fashion with ```E``` and ```F```.
####Implementation suggestion 1
**After a node is updated, invalidate its children and then notify them**. By invalidating the children, they can not be used for further calculation until they have received a new value, when their node has succeeded evaluation. This introduces a barrier.

 1. ```A``` updates its value.
 2. ```B``` and ```C``` are invalidated.
 3. ```B``` and ```C``` are evaluated independently in parallel.
 4. An attempt to evaluate ```D``` is made but fails since either ```B``` or ```C``` is invalid.
 5. ```D``` is evaluated using the new values of ```B``` and ```C```.
 6. The evaluation continues in the same fashion with ```E``` and ```F```.

This implementation might starve ```D``` from ever obtaining a value. If the frequency of updates of ```A``` is much higher than of ```B``` or ```C```, the evaluation of ```D``` might fail continuously starving the system on updates.
####Implementation suggestion 2
Only ```source``` nodes can initiate graph traversals. All other nodes are expected to be deterministic. This means that it is possible to generate a unique signature or fingerprint for a certain update. The list of ```source``` nodes and their revisions when evaluated identifies a specific traversal. When a node is constructed, it can detect at compile-time, the parents that are of ```source``` type and it can inherit the list of ```source``` type nodes of its respective parents. During run-time construction, a node can identify a diamond-pattern by calculating the number of paths between a given ```source``` and itself. Using this information, a node can filter redundant evaluations caused by a redundant ```source``` node.

Although an interesting implementation, the decision has been not to implement the above suggestion. It introduces code complexity for a rare and specific use-case. It currently leaves many implementation details unanswered. The suggested **solution 1** solves the diamond problem with a small sacrifice of being limited to one thread.