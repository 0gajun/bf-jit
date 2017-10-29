#include <chrono>
#include <string>

class Timer {
public:
    Timer();

    double elapsed();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> t1_;
};

void parse_command_line(int argc, const char** argv, std::string* bf_file_path, bool* verbose);
