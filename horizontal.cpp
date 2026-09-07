#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string>
#include <algorithm>

int get_random(int min_val, int max_val) {
    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> distrib(min_val, max_val);
    return distrib(gen);
}

std::vector<char> character_row(std::vector<int>& valid_rows, int height) {
    static const char characters[] = {'-', '_', '>', '@', '+', '=', '/'};
    constexpr size_t num_chars = sizeof(characters);

    std::vector<char> output_row(height, ' ');
    for (int i = 0; i < height; ++i) {
        if (valid_rows[i] > 0) {
            output_row[i] = characters[get_random(0, num_chars - 1)];
            --valid_rows[i];
        }
    }
    return output_row;
}

int main() {
    std::cout << "\033[?25l";

    int prev_width = 0;
    int prev_height = 0;

    std::vector<int> rows;
    std::vector<std::vector<char>> output_log;
    std::string frame_buffer;

    for (int time = 0; time < 10000; ++time) {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int height = std::max(1, static_cast<int>(w.ws_row));
        int width = std::max(1, static_cast<int>(w.ws_col));

        if (height != prev_height || width != prev_width) {
            rows.resize(height, 0);

            output_log.resize(width, std::vector<char>(height, ' '));

            for (auto& col : output_log) {
                col.resize(height, ' ');
            }

            std::cout << "\033[2J";
            prev_height = height;
            prev_width = width;
        }

        for (int i = 0; i < std::max(1, height / 10); ++i) {
            int position = get_random(0, height - 1);
            int amount = get_random(5, 30);
            rows[position] = amount;
        }

        for (int step = 0; step < 10; ++step) {
            std::vector<char> new_col = character_row(rows, height);

            if (!output_log.empty()) {
                output_log.pop_back();
            }
            output_log.insert(output_log.begin(), new_col);

            frame_buffer.clear();
            frame_buffer += "\033[H";

            for (int r = 0; r < height; ++r) {
                for (int c = 0; c < width; ++c) {
                    frame_buffer += output_log[c][r];
                }
                if (r < height - 1) {
                    frame_buffer += '\n';
                }
            }

            std::cout << frame_buffer << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    std::cout << "\033[?25h";
    return 0;
}