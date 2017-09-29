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

struct evl_wire{
     std::string wire_name;
     int bus_size;
     int MSB;
     int LSB;
};

typedef std::list<evl_wire>evl_wires;

struct evl_module{
      std::string module_name;
      evl_wires wires;
};



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

//function to display tokens
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

//function to check end of statement
bool token_is_semicolon(const evl_token &token) {
return token.str == ";";
}

//function to copy tokens to statements
bool move_tokens_to_statement(evl_tokens &statement_tokens,
evl_tokens &tokens)
{

assert(statement_tokens.empty());
evl_tokens::iterator next_sc=std::find_if(tokens.begin(),tokens.end(),token_is_semicolon);
if(next_sc==tokens.end()){
std::cerr << "Unable to find ’;’ and group " << std::endl;
return false;
}

evl_tokens::iterator after_sc=next_sc;
++after_sc;
statement_tokens.splice(statement_tokens.begin(),tokens,tokens.begin(),after_sc);

return true;
}


//function to group tokens 
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
    std::cerr<<"TYPE ERROR :Unknown Statement @ token"<<token.str<<"in line: "<<token.line_no<<std::endl;
    return false;
}

}

return true;
}

//function to display statements
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


bool proper_module(const evl_statement statement,evl_module &inglobal_module)
{

 if(statement.tokens.size()!=3)
 std::cerr<<"SYNTAX ERROR : Number of tokens for MODULE statement should be three"<<std::endl;



enum state_type {INIT, MODULE, MODULENAME,DONE};
state_type state=state_type::INIT;

evl_tokens::const_iterator index;

for(index = statement.tokens.begin();index!=statement.tokens.end();)
{
  if(index->str=="module" && state==state_type::INIT)
    {
         state=state_type::MODULE;
           ++index;
    }
  else if (index->type==evl_token::NAME && state==state_type::MODULE)
      { 
          state=state_type::MODULENAME;
          inglobal_module.module_name=index->str;
         ++index;
       }        
   else if(index->str==";" && state==state_type::MODULENAME)
       {
           state=state_type::DONE;
            ++index; 
           break;     
        }
     else
      {
       std::cerr<<"MODULE STATEMENT SYNTAX ERROR NEAR -->"<<index->str<<" in line: "<<index->line_no<<std::endl;
       return false;
      }
}

if(!(index==statement.tokens.end() && state==state_type::DONE))
{
--index;
std::cerr<<"MODULE STATEMENT SYNTAX ERROR NEAR -->"<<index->str<<"in line:"<<index->line_no<<std::endl;
return false;
}



return true;
}

bool proper_endmodule(const evl_statement statement)
{

if(statement.tokens.size()!=1)
{
 const evl_token first_token=statement.tokens.front();
 std::cerr<<"SYNTAX ERROR around LINE "<<first_token.line_no<<" : Number of tokens for ENDMODULE statement should be one"<<std::endl;
return false;
}

const evl_token token=statement.tokens.front();
if(!(token.str=="endmodule"))
{
std::cerr<<"SYNTAX ERROR around LINE "<< token.line_no <<" : Unknown syntax error ENDMODULE should be written as endmodule"<<std::endl;
return false;
}

return true;
}



bool proper_wire_syntax(const evl_statement statement,evl_module &inglobal_module)
{

enum state_type {INIT, WIRE,WIRENAME,WIRES,BUS,BUSMSB,BUSCOLON,BUSLSB,BUSDONE,DONE};
state_type state=state_type::INIT;

evl_wire temp_wire;
evl_tokens::const_iterator index;
//global_module.wires bla blah
for(index = statement.tokens.begin();index!=statement.tokens.end();)
{

  if(index->str=="wire" && state==state_type::INIT)
    {
         state=state_type::WIRE;
           ++index;
    }
  else if (index->type==evl_token::NAME && (state==state_type::WIRE||state==state_type::BUSDONE))
      {
          temp_wire.wire_name=index->str;
         if(state==state_type::WIRE)
         {
          temp_wire.MSB=0;
          temp_wire.LSB=0;
          temp_wire.bus_size=1;
         }
          inglobal_module.wires.push_back(temp_wire);
         state=state_type::WIRENAME;
         ++index;
       }
   else if(index->str=="," && state==state_type::WIRENAME)
       {
           state=state_type::WIRES;
            ++index;
        }
   else if (index->type==evl_token::NAME && state==state_type::WIRES)
      {
          state=state_type::WIRENAME;
          temp_wire.wire_name=index->str;
          inglobal_module.wires.push_back(temp_wire);
         ++index;
       }
    else if(index->str==";" && state==state_type::WIRENAME)
       {
           state=state_type::DONE;
            ++index;
           break;
        }
     else if (index->str=="[" && state==state_type::WIRE)
      {
          state=state_type::BUS;
          ++index;
       }
     else if(index->type==evl_token::NUMBER && state==state_type::BUS)
       {
          state=state_type::BUSMSB;
          std::string::size_type sz;
          temp_wire.MSB=std::stoi(index->str,&sz);
          ++index;
       }
     else  if(index->str==":" && state==state_type::BUSMSB)
       {
          state=state_type::BUSCOLON;
          ++index;
       }
     else if(index->type==evl_token::NUMBER && state==state_type::BUSCOLON)
       {
          state=state_type::BUSLSB;
          std::string::size_type sz;
          temp_wire.LSB=std::stoi(index->str,&sz);
          temp_wire.bus_size=temp_wire.MSB-temp_wire.LSB+1;
          ++index;
        }
    else if(index->str=="]"&& state==state_type::BUSLSB)
       {
          state=state_type::BUSDONE;
         ++index;
        }
     else
      {
       std::cerr<<"WIRE STATEMENT SYNTAX ERROR NEAR -->"<<index->str<<" in line: "<<index->line_no<<std::endl;
       return false;
      }
}

if(!(index==statement.tokens.end() && state==state_type::DONE))
{
--index;
std::cerr<<"WIRE STATEMENT SYNTAX ERROR NEAR -->"<<index->str<<"in line:"<<index->line_no<<std::endl;
return false;
}


return true;
}



//function to check syntax
bool proper_syntax(evl_statements &istatements,evl_module &iglobal_module)
{
//check if the very first statement
evl_statements::iterator beg=istatements.begin();
if(beg->type != evl_statement::MODULE){
     evl_token first_token=beg->tokens.front();  
     std::cerr <<"SYNTAX ERROR around LINE "<<first_token.line_no<<" :The first statement should be of type MODULE" <<std::endl;
     return false;
      }
//check if the last statement is endmodule
evl_statements::iterator bac=istatements.end();
--bac;
if(bac->type != evl_statement::ENDMODULE){
     evl_token first_token=bac->tokens.front();    
     std::cerr <<"SYNTAX ERROR around LINE "<<first_token.line_no<<" :The last statement should be of type ENDMODULE" <<std::endl;
     return false;
      }
//check if there are any module/endmodule in between
++beg;
evl_statements::iterator next_mod=std::find_if(beg,istatements.end(),
[](const evl_statement &statement){
return statement.type==evl_statement::MODULE;
} );



if(next_mod!=istatements.end())
{
  evl_token first_token=next_mod->tokens.front();
  std::cerr<<"SYNTAX ERROR around LINE "<<first_token.line_no<<"  :Multiple statements of type MODULE"<<std::endl;
  return false;
}

evl_statements::iterator next_end_mod=std::find_if(beg,bac,[](const evl_statement &statement){
return statement.type==evl_statement::ENDMODULE;
});


if(next_end_mod!=bac)
{
  evl_token first_token=next_end_mod->tokens.front();
  std::cerr<<"SYNTAX ERROR around LINE "<<first_token.line_no<<"  :Multiple statements of type ENDMODULE"<<std::endl;
  return false;
}

//check module statement
if(!proper_module(istatements.front(),iglobal_module))
{
return false;
}

istatements.pop_front();
if(!proper_endmodule(istatements.back()))
{
return false;
}
istatements.pop_back();


for(;!(istatements.empty());)
{
evl_statement sfront=istatements.front();
if(sfront.type==evl_statement::WIRE)
{
   if(!proper_wire_syntax(sfront,iglobal_module))
      return false;
istatements.pop_front();
}
else if(sfront.type==evl_statement::COMPONENT)
{
   //if(!proper_component_syntax(sfront,iglobal_module))
   //   return false;
std::cout<<"currently not supporting syntax of COMPONENT Statements"<<std::endl;
istatements.pop_front();
}
else
{
std::cerr<<"SYNTAX ERROR:Unkown statement type;Statement should be of type MODULE/ENDMODULE/WIRE/COMPONENT"<<std::endl;
return false;
}
}

return true;
}



void display_syntax(std::ostream &out, const evl_module &out_module)
{
out << "module "<< out_module.module_name <<std::endl;
out << "wires "<< out_module.wires.size() <<std::endl;
for (evl_wires::const_iterator index = out_module.wires.begin();index!= out_module.wires.end(); ++index)
{
out << "  wire "<< index->wire_name <<" "<<index->bus_size<<std::endl;
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
    std::cout << "\n Displaying extracted tokens:"<< std::endl;
    std::cout << "===================================================================="<< std::endl;
    display_tokens(std::cout,tokens);
    std::cout <<"====================================================================="<< std::endl;
    std::cout << "End of tokens display"<< std::endl;
    
    //storing tokens in a file
   //to not to store comment the next block
    std::string output_lex_file_name = std::string(argv[1])+".tokens";
    std::ofstream output_lex_file(output_lex_file_name);

    if (!output_lex_file)
    {
        std::cerr << "can't write " << argv[1] << ".tokens ." << std::endl;
        return -1;
    }
   display_tokens(output_lex_file,tokens);
   
   evl_statements statements;
      
    if(!group_tokens_into_statements(statements,tokens))
    {
     return -1;
    }
    //display on screen
    std::cout << "\n \nDetails of grouped statements:"<< std::endl;
    std::cout << "**************************************************************************************"<< std::endl;
    display_statements(std::cout,statements);
    std::cout <<"***************************************************************************************"<< std::endl;
    std::cout << "End of grouped statements display"<< std::endl;


    std::string output_statement_file_name = std::string(argv[1])+".statements";
    std::ofstream output_statement_file(output_statement_file_name);

    if (!output_statement_file)
    {
        std::cerr << "can't write " << argv[1] << ".statements ." << std::endl;
        return -1;
    }
    display_statements(output_statement_file,statements);

     evl_module global_module;
    if(!proper_syntax(statements,global_module)) 
     {
      return -1;
     }
    
    std::cout << "\n \nDetails of NETLIST extracted during syntax check:"<< std::endl;
    std::cout <<"#########################################################################################"<< std::endl;
    display_syntax(std::cout,global_module);  
    std::cout <<"#########################################################################################"<< std::endl;
    std::cout << "End of NETLIST"<< std::endl;

   
    std::string output_syntax_file_name = std::string(argv[1])+".syntax";
    std::ofstream output_syntax_file(output_syntax_file_name);

    if (!output_syntax_file)
    {
        std::cerr << "can't write " << argv[1] << ".syntax ." << std::endl;
        return -1;
    }
    display_syntax(output_syntax_file,global_module);


    return 0;
}
