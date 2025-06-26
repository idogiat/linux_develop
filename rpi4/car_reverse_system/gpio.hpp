#pragma once

#include <gpiod.h>
#include <iostream>

class GPIO {
    gpiod_chip* chip;
    gpiod_line* line;

public:
    GPIO(const std::string& chipname, int line_num) {
        chip = gpiod_chip_open_by_name(chipname.c_str());
        if (!chip) throw std::runtime_error("Failed to open chip");

        line = gpiod_chip_get_line(chip, line_num);
        if (!line) throw std::runtime_error("Failed to get line");

        if (gpiod_line_request_output(line, "gpio_writer", 0) < 0)
            throw std::runtime_error("Failed to request output line");
    }

    void write(int value) {
        gpiod_line_set_value(line, value);
    }

    ~GPIO() {
        gpiod_line_release(line);
        gpiod_chip_close(chip);
    }
};