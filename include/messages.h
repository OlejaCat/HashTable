#ifndef MESSAGES_H
#define MESSAGES_H

const char HELP_MESSAGE[] =
    "Usage: your_script [options]\n"
    "\n"
    "Options:\n"
    "  --generate_tests           Generate test cases for hashing functions\n"
    "  --run_string_hashing_tests Run string hashing tests only\n"
    "  --help                     Show this help message and exit\n";

const char UNKNOWM_ARG[]     = "Unknown arguments passed\n";

const char HELP_FLAG[]       = "--help";
const char GENERATE_TESTS[]  = "--generate_tests";
const char RUN_STRING_TEST[] = "--run_string_hashing_tests";

#endif // MESSAGES_H

