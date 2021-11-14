//
// Created by Frey Tinkerer on 11/11/21.
//

#ifndef FT_IRC_PARSER_HPP
#define FT_IRC_PARSER_HPP
#include "Client.hpp"
#include <regex.h>
#include <iostream>

#define ERROR_ -1
#define SUCSSES_ 0
#define TWO_WORDS 2
#define CMD 1
#define PREFIX 0
#define PARAM 2

namespace IRC{
    class Parser {
        public:
            typedef std::vector<Client>& Base;
            Parser(Base clients, std::string buf, int fd);
            ~Parser() {}
        private:
            std::vector<std::string> params;
            Client* id;
            std::string value;
            int prefix_parsing();// парсим первое слово
            int _parsing_command_word(std::string buf, int first_char);// парсим второе слово
            int _parsing_param_word(std::string &buf, int first_char);// парсим третье слово
            int _name_control(std::string &prefix, int sign);
    };
}

#endif //FT_IRC_PARSER_HPP
