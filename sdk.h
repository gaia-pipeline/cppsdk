#ifndef SDK_H
#define SDK_H

#include <string>
#include <list>
#include "plugin.grpc.pb.h"

using std::string;
using std::list;
using proto::Job;

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
    string (*handler)(list<argument>) throw(string);
    string title;
    string description;
    list<string> depends_on;
    list<argument> args;
    manual_interaction * interaction;
};

struct job_wrapper {
    string (*handler)(list<argument>) throw(string);
    Job job;
};

#endif 
