//
// Created by Frey Tinkerer on 11/11/21.
//

#ifndef FT_IRC_PARSER_HPP
#define FT_IRC_PARSER_HPP
#include "Client.hpp"

#define ERROR_ 1
#define SUCSSES_ 0
#define TWO_WORDS -1

namespace IRC{
    class Parser {
        public:
            typedef std::vector<Client>& Base;
            Parser(Base clients, std::string buf, int fd);
            ~Parser();
        private:
            std::vector<std::string> params;
            Client* id;
            int prefix_parsing(std::string &buf);// парсим первое слово
            int _parsing_command_word(std::string &buf, int &first_char);// парсим второе слово
            int _parsing_param_word(std::string &buf, int &first_char);// парсим третье слово
    };
}

#endif //FT_IRC_PARSER_HPP
