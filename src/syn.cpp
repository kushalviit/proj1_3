#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <assert.h>
#include <algorithm>



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


bool token_is_semicolon(const evl_token &token) {
return token.str == ";";
}


bool move_tokens_to_statement(evl_tokens &statement_tokens,
evl_tokens &tokens)
{

assert(statement_tokens.empty());
evl_tokens::iterator next_sc=std::find_if(tokens.begin(),tokens.end(),token_is_semicolon);
if(next_sc==tokens.end()){
std::cerr << "Look for ’;’ but reach the end of file" << std::endl;
return false;
}

evl_tokens::iterator after_sc=next_sc;
++after_sc;
statement_tokens.splice(statement_tokens.begin(),tokens,tokens.begin(),after_sc);

return true;
}



bool group_tokens_into_statements(evl_statements &iostatements,evl_tokens &itokens)
{

for(;!itokens.empty();)
{
evl_token token = itokens.front();
if(token.str=="module")
{
evl_statement module;
module.type=evl_statement::MODULE;
  if(!move_tokens_to_statement(module.tokens,itokens))
    return false;
iostatements.push_back(module);
}
else if(token.str=="wire")
{
evl_statement wire;
wire.type=evl_statement::WIRE;
  if(!move_tokens_to_statement(wire.tokens,itokens))
    return false;
iostatements.push_back(wire);
}
else if((token.type==evl_token::NAME)&&(token.str!="module")
&&(token.str!="wire")&&(token.str!="endmodule"))
{
evl_statement component;
component.type=evl_statement::COMPONENT;
  if(!move_tokens_to_statement(component.tokens,itokens))
    return false;
iostatements.push_back(component);
}
else if(token.str=="endmodule")
{
evl_statement endmodule;
endmodule.type=evl_statement::ENDMODULE;
endmodule.tokens.push_back(token);
itokens.pop_front();
iostatements.push_back(endmodule);
}
else
{
    std::cerr<<"Unknown Statement @ token"<<token.str<<"in line: "<<token.line_no<<std::endl;
    return false;
}

}

return true;
}

void display_statements(std::ostream &out, const evl_statements &show_statements)
{
int i=0;
  for (evl_statements::const_iterator iter = show_statements.begin();iter != show_statements.end(); ++iter,++i)
   {
         if(iter->type == evl_statement::MODULE){
               out << "Statement "<<i<<": MODULE ,"<<iter->tokens.size()<<" tokens" <<std::endl;
            
         }
         else if(iter->type==evl_statement::WIRE){
              out << "Statement "<<i<<": WIRE ,"<<iter->tokens.size()<<" tokens" <<std::endl;
         }
         else if(iter->type==evl_statement::COMPONENT){
              out << "Statement "<<i<<": COMPONENT ,"<<iter->tokens.size()<<" tokens" <<std::endl;
         }
         else if (iter->type == evl_statement::ENDMODULE) {
             out << "Statement "<<i<<":ENDMODULE , "<<iter->tokens.size()<<" tokens" <<std::endl;
               }
          else { //Unknown token at this point
                out << "Unknown token at this point "<< std::endl;
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
    //display on screen
    //comment the next statement to not to see the tokens on command screen
    display_tokens(std::cout,tokens);

    
    //storing tokens in a file
   //to not to store comment the next block
    std::string output_lex_file_name = std::string(argv[1])+".tokens";
    std::ofstream output_lex_file(output_lex_file_name);

    if (!output_lex_file)
    {
        std::cerr << "I can't write " << argv[1] << ".tokens ." << std::endl;
        return -1;
    }
   display_tokens(output_lex_file,tokens);
   
   evl_statements statements;
    
    if(!group_tokens_into_statements(statements,tokens))
    {
     return -1;
    }
    //display on screen
    display_statements(std::cout,statements);

 
    return 0;
}
