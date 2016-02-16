
#include "machine.h"
 
#include <exception>
#include <memory>
#include <cstdio>
#include <map>
#include <string>
#include <cstdlib>
#include <cerrno>

static constexpr char project_url[] = "https://github.com/GBuella/chipemu";
static std::unique_ptr<testbench::machine> machine;

static void process_arguments(char**);
static const char *program_name;
static void usage_exit(int exit_code);
static void print_run_result(testbench::run_result);
FILE *trace_file = nullptr;
bool print_stats_on_exit = false;

/* Code for registering machine constructors in other translation units,
 * without recompiling this one.
 */


int main(int argc, char **argv)
{
    testbench::run_result result;

    if (testbench::machine::available.count() == 0) {   // forgot to link implementations?
        fputs("Fatal error no machines implemented\n", stderr);
        return 1;
    }
    process_arguments(argv);
    if (not machine) usage_exit(2);
    if (trace_file != nullptr) {
        machine->enable_trace(trace_file);
    }
    result = machine->run(stdin, stdout);
    if (print_stats_on_exit) {
        print_run_result(result);
    }
}

static void print_run_result(testbench::run_result result)
{
    printf("\nCycles: %llu\n", result.cycle_count);
}

static void usage_exit(int exit_code)
{
    FILE *output = ((exit_code == EXIT_SUCCESS) ? stdout : stderr);

    fprintf(output, "Retro basic interpreter, for testing the chipemu library\n"
     "%s\n"
     "Built: " __DATE__ " " __TIME__ "\n"
     "Usage:\n"
     "%s [-h] [-t path] <machine type>\n"
     "  -h\n"
     "  --help          print this very helpful text, and exit\n"
     "  -s              print some statistics on exit\n"
     "  -t path\n"
     "  --trace path    print trace to file at `path`\n"
     "  <machine type>  basic interpreter to emulate, available choices are:\n",
     project_url,
     program_name ? program_name : "./basic");
    for (auto machine : testbench::machine::available) {
        fprintf(output, "                     %s\n", machine);
    }
    exit(exit_code);
}

static void pick_machine(std::string name)
{
    if (testbench::machine::available.is(name.c_str())) {
        try {
            machine.reset(testbench::machine::create(name.c_str()));
        }
        catch (const std::exception& exception) {
            fputs(exception.what(), stderr);
        }
        if (not machine) {
            fputs("Error initializing machine\n", stderr);
            exit(1);
        }
    }
    else {
        fprintf(stderr, "Unkown machine: %s\n", name.c_str());
    }
}

static void setup_trace_path(const char *path)
{
    if (path == nullptr or path[0] == 0) {
        usage_exit(2);
    }
    errno = 0;
    trace_file = fopen(path, "w");
    if (trace_file == nullptr) {
        perror("Unable to write trace");
        exit(1);
    }
}

static void process_arguments(char **arg)
{
    if (*arg == nullptr) return;
    program_name = *arg++;
    while (*arg != nullptr) {
        std::string argument(*arg++);

        if (argument == "-h" or argument == "--help") {
            usage_exit(EXIT_SUCCESS);
        }
        else if (argument == "-t" or argument == "--trace") {
            setup_trace_path(*arg++);
        }
        else if (argument == "-s") {
            print_stats_on_exit = true;
        }
        else {
            pick_machine(argument);
        }
    }
}

