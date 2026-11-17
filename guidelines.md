# Welcome
Welcome to the rAthena Code Guidelines. 
This is here to help all developers in creating safe, maintainable, easily readable and understandable code.

# References
These guidelines are inspired by the CppCoreGuidelines. Some of the examples below maybe taken directly from there.
If you need more information that may not be listed here, please be sure to check out [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md).
If your questions are still unanswered after consulting this or CppCoreGuidelines, consider consulting the [rAthena forum](https://rathena.org/board/), [rAthena's Discord](https://rathena.org/discord/), or add your concerns directly to a pull request.

# Code Guidelines
### 1. Don't unnecessarily remove or add blank lines when committing new code

##### Reason

Commits should be on point, documented and as small as possible.

##### Enforcement

Check for removed or added blank lines on each commit.

### 2. Use std::remove_if, std::remove & ... within std::vector::erase

##### Reason

`std::vector` requires to be shrunk after using `std::remove`. To avoid forgetting about proper shrinking after removing an element the removing should always be bound to a call to `erase`.

##### Note

If using a `std::list` one should rely on `std::list::remove` instead of `std::remove`.

##### Example

    vec.erase(std::remove_if(vec.begin(), vec.end(), [](int a) {return a % 2 == 0;}), vec.end()); // good

##### Enforcement

Check for `std::remove` on datatype `std::vector` without being surrounded by an `erase`.

### 3. Don't add redundant `==` or `!=` to conditions

##### Reason

Doing so avoids verbosity and eliminates some opportunities for mistakes.
Helps make style consistent and conventional.

##### Example

By definition, a condition in an `if`-statement, `while`-statement, or a `for`-statement selects between `true` and `false`.
A numeric value is compared to `0` and a pointer value to `nullptr`.

    // These all mean "if `p` is not `nullptr`"
    if (p) { ... }            // good
    if (p != 0) { ... }       // redundant `!=0`; bad: don't use 0 for pointers
    if (p != nullptr) { ... } // redundant `!=nullptr`, not recommended

Often, `if (p)` is read as "if `p` is valid" which is a direct expression of the programmers intent,
whereas `if (p != nullptr)` would be a long-winded workaround.

##### All examples
https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#es87-dont-add-redundant--or--to-conditions

##### Note

Explicit comparison of an integer to `0` is in general not redundant.
The reason is that (as opposed to pointers and Booleans) an integer often has more than two reasonable values.
Furthermore `0` (zero) is often used to indicate success.
Consequently, it is best to be specific about the comparison.

    void f(int i)
    {
        if (i)            // suspect
        // ...
        if (i == success) // possibly better
        // ...
    }

Always remember that an integer can have more than two values.

##### Enforcement

Easy, just check for redundant use of `!=` and `==` in conditions.

### 5. Avoid "magic constants"; use symbolic constants

##### Reason

Unnamed constants embedded in expressions are easily overlooked and often hard to understand:

##### Example

    for (int m = 1; m <= 12; ++m)   // don't: magic constant 12
        cout << month[m] << '\n';

No, we don't all know that there are 12 months, numbered 1..12, in a year. Better:

    // months are indexed 1..12
    constexpr int first_month = 1;
    constexpr int last_month = 12;

    for (int m = first_month; m <= last_month; ++m)   // better
        cout << month[m] << '\n';

Better still, don't expose constants:

    for (auto m : month)
        cout << m << '\n';

##### Enforcement

Flag literals in code. Give a pass to `0`, `1`, `nullptr`, `\n`, `""`, and others on a positive list.

### 4. Use `auto` to avoid redundant repetition of type names

##### Reason

* Simple repetition is tedious and error-prone.
* When you use `auto`, the name of the declared entity is in a fixed position in the declaration, increasing readability.
* In a template function declaration the return type can be a member type.

##### Example

Consider:

    auto p = v.begin();   // vector<int>::iterator
    auto h = t.future();
    auto q = make_unique<int[]>(s);
    auto f = [](int x){ return x + 10; };

In each case, we save writing a longish, hard-to-remember type that the compiler already knows but a programmer could get wrong.

##### Exception

Avoid 'auto' when declaring a variable which is directly initiated with a return value of a function call which does not indicate the type. This is only required when using the function call the first time within a method.

Also avoid `auto` for initializer lists and in cases where you know exactly which type you want and where an initializer might require conversion.

##### Example (bad)

    auto lst = { 1, 2, 3 };   // lst is an initializer list
    auto x{1};   // x is an int (in C++17; initializer_list in C++11)

##### Example (good)

    void some_function()
    {
        my_type a = my_function();
        auto b = my_function(); // here we already know the return type of 'my_function'
        ...
    }
 
##### All examples
https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#es11-use-auto-to-avoid-redundant-repetition-of-type-names
 
##### Enforcement

Flag redundant repetition of type names in a declaration.
