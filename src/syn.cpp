#include <iostream>
#include <fstream>
#include <string>
#include <list>

//definition and typing of tokens
struct evl_token {
enum token_type {NAME, NUMBER, SINGLE};
token_type type;
std::string str;
int line_no;
}; // struct evl_token

typedef std::list<evl_token> evl_tokens;

//definition and typing of statements

struct evl_statement{
   enum statement_type{MODULE,WIRE,COMPONENT,ENDMODULE};
   
   statement_type type;
   evl_tokens tokens;
}; //struct evl_statement

typedef std::list<evl_statement> evl_statements;



//function to extract tokens from line
bool extract_tokens_from_line(const std::string line, const int line_no, evl_tokens &otokens)
{



for (size_t i = 0; i < line.size();)
        {
            // comments
            if (line[i] == '/')
            {
                ++i;
                if ((i == line.size()) || (line[i] != '/'))
                {
                    std::cerr << "LINE " << line_no
                        << ": a single / is not allowed" << std::endl;
                    return false;
                }
                break; // skip the rest of the line by exiting the loop
            }

            // spaces
            if ((line[i] == ' ') || (line[i] == '\t')
                || (line[i] == '\r') || (line[i] == '\n'))
            {
                ++i; // skip this space character
                continue; // skip the rest of the iteration
            }

            // SINGLE
            if ((line[i] == '(') || (line[i] == ')')
                || (line[i] == '[') || (line[i] == ']')
                || (line[i] == ':') || (line[i] == ';')
                || (line[i] == ','))
            {
                evl_token temp_token;
                temp_token.line_no=line_no;
                temp_token.type=evl_token::SINGLE;
                temp_token.str=std::string(1,line[i]);
                otokens.push_back(temp_token);
                ++i; // we consumed this character
                continue; // skip the rest of the iteration
            }

            // NAME
            if (((line[i] >= 'a') && (line[i] <= 'z'))       // a to z
                || ((line[i] >= 'A') && (line[i] <= 'Z'))    // A to Z
                || (line[i] == '_'))
            {
                size_t name_begin = i;
                for (++i; i < line.size(); ++i)
                {
                    if (!(((line[i] >= 'a') && (line[i] <= 'z'))
                        || ((line[i] >= 'A') && (line[i] <= 'Z'))
                        || ((line[i] >= '0') && (line[i] <= '9'))
                        || (line[i] == '_') || (line[i] == '$')))
                    {
                        break; // [name_begin, i) is the range for the token
                    }
                }
                evl_token temp_token;
                temp_token.line_no=line_no;
                temp_token.type=evl_token::NAME;
                temp_token.str=line.substr(name_begin,i-name_begin);
                otokens.push_back(temp_token);
                
            }
            // NUMBER
            else if((line[i]>='0')&&(line[i]<='9'))
            {
                  size_t number_begin=i;
                  for(++i;i<line.size();++i)
                  {
                     if(!((line[i]>='0')&&(line[i]<='9')))
                     {
                      break;
                     }

                   }
                   evl_token temp_token;
                   temp_token.line_no=line_no;
                   temp_token.type=evl_token::NUMBER;
                   temp_token.str=line.substr(number_begin,i-number_begin);
                   otokens.push_back(temp_token);
            }
            else
            {
                std::cerr << "LINE " << line_no
                    << ": invalid character" << std::endl;
                return false;
            }
        }




return true;
}




//function to extract tokens from file
bool extract_tokens_from_file(std::string &input_file_name,evl_tokens &iotokens)
{

std::ifstream input_file(input_file_name);

if(!input_file)
{
std::cerr<<"Cannot read the input file "<<input_file_name<< " ."<<std::endl;
return false;
}

std::string line;

iotokens.clear(); // making sure tokens list is empty

for (int line_no = 1; std::getline(input_file, line); ++line_no)
{
   //extract tokens from each line
    if(!extract_tokens_from_line(line,line_no,iotokens))
     return false;
}

return true;
} 


void display_tokens(std::ostream &out, const evl_tokens &show_tokens)
{
  for (evl_tokens::const_iterator iter = show_tokens.begin();iter != show_tokens.end(); ++iter)
   {
         if (iter->type == evl_token::SINGLE) {
                         out << "SINGLE " << iter->str << std::endl;
                      }
         else if (iter->type == evl_token::NAME) {
                    out << "NAME " << iter->str << std::endl;
                     }
          else { // must be NUMBER
                out << "NUMBER " << iter->str << std::endl;
              }
   } 


}





int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "You should provide a file name." << std::endl;
        return -1;
    }

    std::string evl_file=argv[1];
    evl_tokens tokens;
    if(!extract_tokens_from_file(evl_file,tokens))
    {
        return -1;
    }
    display_tokens(std::cout,tokens);

    std::string output_file_name = std::string(argv[1])+".tokens";
    std::ofstream output_file(output_file_name);

    if (!output_file)
    {
        std::cerr << "I can't write " << argv[1] << ".tokens ." << std::endl;
        return -1;
    }

   display_tokens(output_file,tokens);
    
    return 0;
}
