//
// Created by Frey Tinkerer on 11/11/21.
//

#include "Parser.hpp"

namespace IRC{
    Parser::Parser(Base clients, std::string buf, int fd)
    {
        id = std::find_if(clients.begin(), clients.end(), is_fd(fd)).base();
        value = buf;
        prefix_parsing();
    }

    int Parser::_parsing_param_word(std::string &buf, int first_char) // добавить обработку пробелов в конце
    {
        std::string param;
        int i = 0;
        bool sign = false;
        int space = 0;
        int res = -1;

        if (buf[first_char] != ':')
        {
            space = buf.find_first_of(" ");
            if (space != std::string::npos || buf[space + 1] != ':')
            {
                param = value.substr(1, first_char);
                res = _name_control(param, PREFIX);
                return ERROR_;
            }
        }
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

    int Parser::_parsing_command_word(std::string buf, int first_char)
    {
        std::string cmd;
        int res = -1;
        
        int second_char = buf.find_first_of(" ");
        cmd = buf.substr(0, second_char);// возвращаем команду
        res = _name_control(cmd, CMD);
        if (res == ERROR_)
            return res;
        if (second_char == std::string::npos)
            return TWO_WORDS;
        return second_char;
    }

    int Parser::_name_control(std::string &prefix, int sign)
    {
        int i = 0;
        bool sign = false;

        if (sign == 1)
        {
            for(int k = 0; k < prefix.length(); k++)
            {
                if (!std::isalpha(prefix[k]))
                    return ERROR_;
            }
        }
        else{
            std::string spec = "-[]\\^{}";
            while(prefix.length() > i)
            {
                if (!std::isalnum(prefix[i]))
                {
                    for (int j = 0; j < spec.length(); j++)
                    {
                        if (prefix[i] == spec[j])
                        {
                            sign = true;
                            break;
                        }
                    }
                    if (sign == false)
                        return ERROR_;
                    sign = false;
                }
                i++;
            }
        }
        params.push_back(prefix);
        return SUCSSES_;
    }

    int Parser::prefix_parsing()
    {
        int first_char = 0;
        std::string prefix;
        std::string cmd;
        int num_dots = 0;
        int res = 0;

        if (value.find("  ") != std::string::npos || value.find("\t") != std::string::npos)
            return ERROR_;
        if (value[0] == ':')
        {
            first_char = value.find_first_of(" ");
            if (first_char == std::string::npos)
            {
                return ERROR_;
            }
            else
            {
                prefix = value.substr(1, first_char - 1);//вычленяем певое слово
                res = _name_control(prefix, PREFIX); //проверяем что слово лексекически подходит под определение
                if (res == ERROR_){
                    return ERROR_;
                }
                res = _parsing_command_word(value, first_char + 1); //  ОСТАНОВИЛСЯ ЗДЕСЬ
                if (res != TWO_WORDS && res != ERROR_)
                {
                    res = _parsing_param_word(value, first_char + 1);
                }

            }
        }
        else if (std::isalpha(value[0]))
        {
            params.push_back("");
            _parsing_command_word(value, first_char);
            _parsing_param_word(value, first_char);

        }
        return 0;
    }
}