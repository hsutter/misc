/*------------------------------------------------------------------------------

Heinously Simple Tester

A stupidly quick-and-dirty test harness, just because I needed something simple.
Not recommended for real use.

(c) 2020 Herb Sutter, CC BY 4.0 (https://creativecommons.org/licenses/by/4.0/)

------------------------------------------------------------------------------*/

#include <string>
#include <type_traits>

namespace hst {

//------------------------------------------------------------------------------
//  noisy<T>: A little helper to conveniently instrument T's SMF history

std::string history;

template<class T>
class noisy {
public:
    T t;
    noisy()                                  { history += "default-ctor "; };
    noisy(const noisy& rhs) : t{rhs.t}       { history += "copy-ctor "; }
    noisy(noisy&& rhs) : t{std::move(rhs.t)} { history += "move-ctor "; }
    void operator=(const noisy& rhs)         { t = rhs.t; history += "copy-assign "; }
    void operator=(noisy&& rhs)              { t = std::move(rhs.t); history += "move-assign "; }
    ~noisy()                                 { history += "dtor "; }
};

//  run_history: Run some code and return the history it generated
template<typename Func>
std::string run_history(Func f) {
    history = {};
    f();
    return history;
}


//------------------------------------------------------------------------------
//  tester: Helper to run tests and report pass/fail, then print a summary

class tester {
    std::string report;
    int         tests = 0, passed = 0;

public:
    //  Execute a test, and compare its history to the expected history.
    template<typename Func>
    void run(const std::string& name, Func f, const std::string& expected) {
        ++tests;
        auto actual = run_history(f);

        std::string diff;
        for (int i = 0; i < expected.size() && i < actual.size(); ++i) {
            if (expected[i] != actual[i]) {
                while (actual[i++] != ' ') diff.push_back('^');
                break;
            }
            diff.push_back(' ');
        }

        report += std::to_string(tests) + ": " + name + "... ";
        if (expected == actual) {
            ++passed;
            report += "passed";
        } else {
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
        return  "\nTest summary:"
                "\n  total:  " + std::to_string(tests) +
                "\n  passed: " + std::to_string(passed) +
                "\n  failed: " + std::to_string(tests-passed) +
                "\n\n" + report;
    }
};

}