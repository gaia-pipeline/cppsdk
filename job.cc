#include <string>
#include <list>

using std::string;
using std::list;

enum input_type {
    textfield,
    textarea,
    boolean,
    vault
};

struct argument {
    string description;
    input_type type;
    string key;
    string value;
};

struct manual_interaction {
    string description;
    input_type type;
    string value;
};

struct job {
    void (*handler)(list<argument>);
    string title;
    string description;
    list<string> depends_on;
    list<argument> args;
    manual_interaction * interaction;
};
