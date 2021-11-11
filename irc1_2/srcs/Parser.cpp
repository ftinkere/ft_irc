//
// Created by Frey Tinkerer on 11/11/21.
//

#include "Parser.hpp"

namespace IRC{
    Parser::Parser(Base clients, std::string buf, int fd)
    {
        id = std::find_if(clients.begin(), clients.end(), is_fd(fd)).base();

    }

    int Parser::_parsing_param_word(std::string &buf, int &first_char) // добавить обработку пробелов в конце
    {
        std::string param;
        int i = 0;
        bool sign = false;

        while(buf.length() > i){
            if (buf[i] == ' ' && buf[i + 1] != ':')
            {
                params.push_back(param);
                param.clear();

            }
            else if (buf[i] == ' ' && buf[i + 1] == ':')
            {
                sign = true;
                params.push_back(param);
                param.clear();
            }
            else if (i == buf.length() - 1)
            {
                params.push_back(param);
            }
            else{
                param.push_back(buf[i]);
            }
        }
        if (sign == true)
            return SUCSSES_;
        return (ERROR_);
    }

    int Parser::_parsing_command_word(std::string &buf, int &first_char)
    {
        std::string cmd;
        
        int second_char = buf.find_first_of(" ");
        if (second_char == std::string::npos)
        {
            cmd = buf.substr(first_char, second_char);// возвращаем команду
            params.push_back(cmd);
            return TWO_WORDS;
        }
        cmd = buf.substr(first_char, first_char);
        params.push_back(cmd);
        return second_char;
    }

    int Parser::prefix_parsing(std::string& buf)
    {
        int first_char;
        std::string prefix;
        std::string cmd;

        if (buf[0] == ':')
        {
            first_char = buf.find_first_of(" ");
            if (first_char == std::string::npos)
            {
                return ERROR_;
            }
            else
            {
                prefix = buf.substr(1, first_char);
                params.push_back(prefix);
                first_char++;//передвигаемся с пробела
                if (_parsing_command_word(buf, first_char) != -1)
                {
                    _parsing_param_word(buf, first_char);
                }

            }
        }
        else
        {
            params.push_back("");
            _parsing_command_word(buf, first_char);
            _parsing_param_word(buf, first_char);

        }
    }
}