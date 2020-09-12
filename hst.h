/*------------------------------------------------------------------------------

Heinously Simple Tester

A stupidly quick-and-dirty test harness, just because I needed something simple.
Not recommended for real use.

(c) 2020 Herb Sutter, CC BY 4.0 (https://creativecommons.org/licenses/by/4.0/)

------------------------------------------------------------------------------*/

#ifndef HST_DEFINED
#define HST_DEFINED

#include <string>
#include <type_traits>

namespace hst {

//------------------------------------------------------------------------------
//  Instrumentation to inspect SMF call history

//  run_history: Run some code and return the history it generated
std::string history;
auto run_history(auto f) { history = {};  f();  return history;  }

//  noisy<T>: A little helper to conveniently instrument T's SMF history
template<typename T>
struct noisy {
    T t;
    noisy()                                  { history += "default-ctor "; }
    ~noisy()                                 { history += "dtor "; }
    noisy(const noisy& rhs) : t{rhs.t}       { history += "copy-ctor "; }
    noisy(noisy&& rhs) : t{std::move(rhs.t)} { history += "move-ctor "; }
    auto operator=(const noisy& rhs)         { history += "copy-assign ";  t = rhs.t; return *this; }
    auto operator=(noisy&& rhs)              { history += "move-assign ";  t = std::move(rhs.t); return *this; }
};


//------------------------------------------------------------------------------
//  can_invoke: Tests whether a call of F(args) will be ambiguous or not.
//              Derived from code provided by Dr. Walter E. Brown, with thanks.
//
//  First, a simple version that doesn't work with overload sets.
template<typename Func, Func* F, typename ...Args >
void can_invoke (Args&&... args) {
    if constexpr( requires { F( static_cast<decltype(args)>(args)... ); } )
        history += "can-invoke ";
    else
        history += "cannot-invoke ";
}
#define HST_CAN_INVOKE(F) can_invoke<decltype(F), &(F)>

//  Second, one that can work with overload sets, but for each function
//  "myfunc" that you want to test you need to write this at global scope:
//      HST_CAN_INVOKE_OVERLOADED(myfunc)
//  and then you can write the test as
//      hst::can_invoke_overloaded_myfunc(params)
//  to query whether calling "myfunc(params)" is legal and unambiguous.
#define HST_CAN_INVOKE_OVERLOADED(F) \
    namespace hst { \
    template<typename ...Args > \
    void hst_can_invoke_overloaded_##F (Args&&... args) { \
        if constexpr( requires { F( static_cast<decltype(args)>(args)... ); } ) \
            history += "can-invoke "; \
        else \
            history += "cannot-invoke "; \
        } \
    }


//------------------------------------------------------------------------------
//  tester: Helper to run tests and report pass/fail, then print a summary

class tester {
    std::string name, passfail, report;
    int         total = 0, passed = 0;

public:
    tester(std::string name_) : name{name_} { }
 
    //  Execute a test, and compare its history to the expected history.
    template<typename Func>
    void run(const std::string& name, Func f, const std::string& expected) {
        ++total;
        auto actual = run_history(f);

        std::string diff;
        for (int i = 0; i < expected.size() && i < actual.size(); ++i) {
            if (expected[i] != actual[i]) {
                while (actual[i++] != ' ') diff.push_back('^');
                break;
            }
            diff.push_back(' ');
        }

        report += std::to_string(total) + ": " + name + "... ";
        if (expected == actual) {
            ++passed;
            passfail += '.';
            report += "passed";
        } else {
            passfail += 'x';
            report += "\n    expected: \"" + expected + 
                    "\"\n    actual:   \"" + actual + 
                    "\"\n               "  + diff;
        }
        report += "\n";
    }

    //  This overload tests against the history string of a baseline
    //  that is supposed to be equivalent and generate the same history.
    template<typename Func, typename Func2>
        requires std::is_invocable_v<Func2>
    void run(const std::string& name, Func f, Func2 expected) {
        run(name, f, run_history(expected));
    }

    std::string summary() {
        return  std::string(75, '=') + 
                "\n" + name + 
                "\n\n  total:  " + std::to_string(total) +
                "\t" + passfail +
                "\n  passed: " + std::to_string(passed) +
                "\n  failed: " + std::to_string(total-passed) +
                "\n" + std::string(75, '-') +
                "\n\n" + report;
    }
};

}

#endif
