# N++ Compiler/VM

Credits to Liam Vickers for the design, and credits to [Crafting Interpreters](https://craftinginterpreters.com) for the original code.

![](ico/N++.png)

## How to use (Command wise)

exe file.npp // args?
nppc3 file.npp // args?

## How to use (Code wise)

Variables:

```
int varName = 10;
```

Broadcasting:

```
broadcast("Hello, world!");
broadcast(10 + 2);
broadcast(varName + 5);
```

Getting input:

```
int input = receive("> ");
broadcast(input);
```

While loops:

```
int i = 0;
while (i < 100) {
    i = i + 1;
}
```

For loops:

```
for (int i = 0; i < 10; i = i + 1;) {
    broadcast("lol");
}
```

Functions:

```
def foo(x, y) {
    return x * y;
}

broadcast (foo(2873, 284));
```

Classes:

```
class Person {
    sayName() {
        broadcast(this.name);
    }
}

int jane = Person();
jane.name = "Jane";

int method = jane.sayName;
method();
```

Inhereted classes:

```
class A {
    method() {
        broadcast("A method");
    }
}

class B < A {
    method() {
        broadcast("B method");
    }

    test() {
        super.method();
    }
}

class C < B {}

C().test();
```

If statements:

```
int i = 0;

if (i == 0) {
    broadcast("A");
} else if (i == 1) {
    broadcast("B");
} else {
    broadcast("C");
}
```

Comments:

```
// This is a comment
```

Redefining variables:

```
int i = 0; // Original variable
broadcast(i);
i = 1; // Changing the variable's value
broadcast(i);
```

Others:

```
broadcast(true); // [TRUE]
broadcast(false); // [FALSE]
broadcast(null); // [NULL]
```

### Clock function:

```
int start = clock();
int end = clock();
broadcast(end - start);
```

### argc() and argv(i) functions

```
broadcast(argc());
broadcast(argv(0));
```

### stringize(v) and integize(v) functions (with argc and argv)

```
broadcast("there are " + stringize(argc()) + " args and arg 0 is " + stringize(argv(0)));
// stringize converts the given value to a string.
broadcast(integize("69"));
// integize converts the given value to a integer.
```

### isNum(v) and isStr(v) functions (with stringize and integize)

```
broadcast(isNum(integize("69")));
broadcast(isStr(integize("69")));
broadcast(isNum(stringize(69)));
broadcast(isStr(stringize(69)));
```

### Accessing .DLLs and .NPPs

```
cGetFunc("file.dll", "func"); // Access functions from dlls, best if they return something.
get("file.npp"); // Access code from a .npp file.
```

### All the functions

```
clock(); // Gets the runtimer
wait(1); // Waits the given number of seconds
time("M-D-Y H:m:S"); // Takes in format, returns a string with time

argc(); // Gets arg count
argv(0); // Gets arg[i]
stringize(69); // Converts the given value to a string
integize("69"); // Converts the given value to a number

isNum(0); // Checks if a value is a number
isBool(false); // Checks if a value is a bool
isObj("hi"); // Checks if a value is a object
isStr("hi"); // Checks if a value is a string
isNull(null); // Checks if a value is null
isInst(classInstance()); // Checks if a value is a instance
isNative(clock()); // Checks if a value is a native function
isClass(classButNoParenBcThatMakesAnInst); // Checks if a value is a class (no instances)
isBoundMethod(classButNoParenBcThatMakesAnInst.method); // Checks if a value is a bound method (i think this works)

broadcastXN("hi"); // Broadcasts the given value without default newline character
broadcast("hi"); // Broadcasts the given value
setColor(30); // Sets the terminal color with ANSI colors (30 - 37 for colors, do 38 for regular white).
receive("> "); // Broadcasts the first arg (without new line) and returns the input after it
system("dir"); // Runs a system command
cGetFunc("file.dll", "func"); // Gets a function from a dll
quit(); // Exits the program

// Triginometry stuff
sin(1);
cos(1);
tan(1);
abs(1);
asin(1);
acos(1);
atan(1);
hypot(1, 2);

// Math stuff
sqrt(1); // Square root
powr(5, 2); // Power operator
mdls(10, 5); // Modulus operator

collectGarbage(); // Collects garbage
runtimeError("Whoopsy daisy!"); // Does a runtime error
get("file.npp"); // Imports a file
strLen("Hello, world!"); // Gets the length of a string
strIndex("Hello, world!", 0); // Gets 0 from string, or the first character in the string.

array("Doors", "D1", "D2"); // Creates an array, objects must be strings.
getArray("Doors", 0); // Gets the given index of an object in the array
lenArray("Doors"); // Returns length of array
addArray("Doors", "D3"); // Adds an object to the array
rmvArray("Doors", 2); // Removes the given index
cngArray("Doors", 0, "Deez NUTZ"); // Changes the given index of an array to a different value.
bctArray("Doors"); // Broadcasts the given array
delArray("Doors"); // Deletes an Array (There is no need to delete an array at the end of the program because that is done automatically).
```
