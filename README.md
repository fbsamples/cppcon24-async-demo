
# CppCon24 Async Stacks Demo Application
A demo application for a CppCon24 talk: How Meta made Debugging Async Code Easier with Coroutines and Senders. The demo application's purpose is to showcase async stacks integration in Unifex.

## Requirements
The demo application requires
* liburing
* C++20
* lldb
* clang-18
* x86_64

We've tested this with clang-18; updated versions of clang may also be compatible. co_bt.py did work for GDB, but that has not been recently confirmed.

## Building the demo application
```
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-stdlib=libc++ -Wno-c++11-narrowing" -DCMAKE_EXE_LINKER_FLAGS=-stdlib=libc++
```

## Importing co_bt.py in lldb
In an .lldbinit file or when you start lldb in the cppcon24-async-demo repo, paste the following code:

```
command script import scripts/co_bt.py
```

## How to view the async stack
In lldb, set the breakpoint on a line of code that will have a running async operation. Type
`co_bt` to see the async stack at that breakpoint.

If there is no running async operation, you will see an error similar to the one below:
```
No async operation detected
```

## Full documentation
This is a demo application for a CppCon talk so this repo will not be actively maintained. co_bt.py is a script maintained by [folly](https://github.com/facebook/folly/blob/main/folly/coro/scripts/co_bt.py). The co_bt.py included in this repo is purely for demo purposes.

See the [CONTRIBUTING](CONTRIBUTING.md) file for how to help out.

## Appendix

- [co_bt.py](https://github.com/facebook/folly/blob/main/folly/coro/scripts/co_bt.py) is maintained by folly.
- This demo application is based off the [AsyncStackRoot and AsyncStackFrame](https://github.com/facebook/folly/blob/main/folly/tracing/AsyncStack.h) data stuctures maintained by folly.
- [Unifex](https://github.com/facebookexperimental/libunifex) was a reference implementation for P2300 and is an open sourced C++ coroutines/sender library.

## License
This project is made available under the Apache license, as found in the LICENSE.txt file.
