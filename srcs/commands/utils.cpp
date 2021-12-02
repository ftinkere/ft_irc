//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"

namespace IRC {

    int check_num(const char *str) {
        //"Функция проверки числа"
        for (int i = 0; i < strlen(str); ++i) {
            if (!std::isdigit(str[i]))
                return false;
        }
        int res = std::atoi(str);
        return res;
    }
}