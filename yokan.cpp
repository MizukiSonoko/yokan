#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <stack>
#include <map>
#include <functional>
#include <initializer_list>
#include <array> 
 
#include <cstdarg>
#include <memory>

#include "kushi.hpp"

class Token{
  public:
    enum Type{
        ERROR = -1,
        MARGIN,
        FIN,
        NAME,
        NUMBER,
        IDENTIFIER,
        EXCLAMATION,
        LPARENT,
        RPARENT,
        RBRACKET,
        LBRACKET,
        RABRACKET,
        LABRACKET,
        SEMICOLON,
        COLON,
        COMMA,
        DQUOTATION,
        EQUAL,
        OPE_ADD, 
        OPE_SUB, 
        OPE_MUL, 
        OPE_DIV, 
    };
    Token(){};
    Token(Type aType,std::string aName){
	    name = aName;
	    type = aType;
    }
    Type getType(){
    	return type;
	}
    std::string getName(){
    	return name;
	}
  private:
    std::string name;
    Type type;
};

class TypeSet{
    std::vector<Token::Type> types;
  public:
    TypeSet(Token::Type type){
        types.push_back(type);
    }
    ~TypeSet(){
        std::cout<<"~~~~~~~~~~~~~~~~~~~DESTRUCTOR\n";
    }
    TypeSet* OR(Token::Type type){
        types.push_back(type);
        return this;
    }
    int size(){
        return types.size();
    }
    Token::Type at(int pos){
        if(pos < this ->size()){
            return types[pos];
        }
        return Token::ERROR;
    }
};

std::map<std::string, int> values;
std::list<Token>  tokens;

auto isLetter(char c)
 -> bool{
    if((c>='a'&&c<='z')||
       (c>='A'&&c<='Z'))
        return true;
    return false;
}
auto isNumber(char c)
 -> bool{
    if(c>='0'&&c<='9')
        return true;
    return false;
}
auto isSpecial(char c)
 -> bool{
 	char list[] = "\",.<>(){}[];:=+";
 	for(auto v : list){
 		if(v==c){
			return true;
		}
	}
	return false;
}
auto lexer(std::string aLine)
 -> std::list<Token>{
	std::list<Token> tokens;

    bool isString = false;
	std::string buffer = "";
    for(auto c : aLine){

        if(isString){   
            if(c=='"'){
                tokens.push_back(Token(Token::NAME,buffer));
                buffer = "";
                isString = false;
                continue;
            }
            buffer += c;
            continue;
        }
        if(isLetter(c)||isNumber(c)){
            buffer += c;
            continue;
        }

        if(isSpecial(c)){
        	if(buffer != ""){
	            if(isNumber(buffer.at(0)))
	                tokens.push_back(Token(Token::NUMBER,buffer));
	            else
	                tokens.push_back(Token(Token::NAME,buffer));
		    	buffer = "";
		    }
        }

        switch(c){
            case ' ': case '\t':
            case '\n':case '\r':
            	if(buffer != ""){
		            if(isNumber(buffer.at(0)))
		                tokens.push_back(Token(Token::NUMBER,buffer));
		            else
		                tokens.push_back(Token(Token::NAME,buffer));
			    	buffer = "";
			    }
		    	break;
            case '"':
                isString = true;
                if(!buffer.empty()){
                    if(isNumber(buffer.at(0)))
                        tokens.push_back(Token(Token::NUMBER,buffer));
                    else
                        tokens.push_back(Token(Token::NAME,buffer));
                    buffer = "";
                }                
                break;
            case '(':
                tokens.push_back(Token(Token::LPARENT,"("));
                break;
            case ')':
                tokens.push_back(Token(Token::RPARENT,")"));
                break;
            case ']':
                tokens.push_back(Token(Token::RBRACKET,"]"));
                break;
            case '[':
                tokens.push_back(Token(Token::LBRACKET,"["));
                break;
            case '>':
                tokens.push_back(Token(Token::RABRACKET,">"));
                break;
            case '<':
                tokens.push_back(Token(Token::LABRACKET,"<"));
                break;
            case ';':
                tokens.push_back(Token(Token::SEMICOLON,";"));
                break;
            case ':':
                tokens.push_back(Token(Token::COLON,":"));
                break;
            case ',':
                tokens.push_back(Token(Token::COMMA,","));
                break;
            case '=':
                tokens.push_back(Token(Token::EQUAL,"="));
                break;
            case '+':
                tokens.push_back(Token(Token::OPE_ADD,"+"));
                break;
            case '-':
                tokens.push_back(Token(Token::OPE_SUB,"-"));
                break;
            case '*':
                tokens.push_back(Token(Token::OPE_MUL,"*"));
                break;
            case '/':
                tokens.push_back(Token(Token::OPE_DIV,"/"));
                break;
            case '!':
                tokens.push_back(Token(Token::EXCLAMATION,"!"));
                break;
            default:
                if(!buffer.empty()){
                    if(isNumber(buffer.at(0)))
                        tokens.push_back(Token(Token::NUMBER,buffer));
                    else
                        tokens.push_back(Token(Token::NAME,buffer));
                    buffer = "";
                }else{
                    std::cout<<"Error! invalid charator:"<< c <<std::endl;
                    exit(1);
                }
        }
    }
    if(!isSpecial(buffer[0])){
        if(isNumber(buffer.at(0)))
            tokens.push_back(Token(Token::NUMBER,buffer));
        else
            tokens.push_back(Token(Token::NAME,buffer));
        buffer = "";
    }else if(!buffer.empty()){
        std::cout<<"Error! invalid charator:"<< buffer <<std::endl;
    }

    tokens.push_back(Token(Token::FIN,"<FIN>"));
    tokens.push_back(Token(Token::MARGIN,"<MARGIN>"));

	return tokens;
}

void log(int layour, std::string msg){
#ifdef DEBUG
    for(int i=0; i < layour; i++){
        std::cout<< " ";
    }
    std::cout<< msg <<"\n";
#endif
}

namespace Perser{
    int buf_index = 0;
    std::stack<int>       markers;
    std::vector<Token> headTokens;
    std::string         curString;

    std::map<std::string, int> variableTable;

    auto fill(int n)
     -> bool{
        for(int i = 0;i < n;i++){
            headTokens.push_back(tokens.front());
            tokens.pop_front();
        }
        return true;
    }

    auto sync(int i)
     -> bool{
        if(buf_index + i > headTokens.size()){
            int n = (buf_index + i) - (headTokens.size());
            fill(n);
        }
        return true;
    }

    auto LT(int i)
     -> Token{        
        sync(i);
        return headTokens[buf_index+i-1];
    }

    auto mark()
     -> int{
        markers.push(buf_index);
        return buf_index;
    }

    auto seek(int index)
     -> bool{
        buf_index = index;
        return true;
    }

    auto release()
     -> bool{
        if(!markers.empty()){
            int marker = markers.top();
            markers.pop();
            seek(marker);
        }
        return true;
    }

    auto isSpec()
     -> bool{
        return markers.size() > 0;
    }

    auto nextToken()
     -> bool{
        buf_index++;
        if(buf_index == headTokens.size() && !isSpec()){
            buf_index = 0;
            headTokens.clear();
            return false; 
        }
        sync(1);   
        return true;
    }

    auto match(Token::Type type)
     -> bool{
        Token  token =  LT(1);     
     
        curString = token.getName();

        Token::Type t = token.getType();
        log(1, "hope:"+std::to_string(type)+" real:"+std::to_string(t));
        if(type==t){
            nextToken();
            return true;
        }else{
            return false;
        }
    }

    auto match(Token::Type type, std::string reserved)
     -> bool{
        Token  token =  LT(1);     
     
        curString = token.getName();

        Token::Type t = token.getType();
        log(1, "hope:"+std::to_string(type)+" real:"+std::to_string(t));
        if(type == t and curString == reserved){
            nextToken();
            return true;
        }else{
            return false;
        }
    } 

    auto match(TypeSet* typeSet)
     -> bool{
        Token  token =  LT(1);     
     
        curString = token.getName();

        for(int i = 0; i < typeSet->size(); i++){
            Token::Type t = typeSet->at(i);
            if(match(t)){
                return true;
            }
        }
        return false;
    }    

    auto defVariable(std::string val_name)
     -> bool{
        if(variableTable.find(val_name) == variableTable.end()){
            return false;
        }
        return true;
    }

    namespace PerserRule{
        int FunctionVariableDecl();
        int FunctionDecl();
        int ConditionExpr();
        int IfStatement();
        int BinaryExpr();
        int VariableDecl();
        int Statement();
        int FIN();
    };

    namespace Rule{
        std::array< std::function<bool()>, 1> FIN{
            []{
                return match(Token::FIN);            
            }
        };

        std::array< std::function<bool()>, 2> FunctionVariableDecl{
            []{
                return
                    match(Token::NAME) &&
                    match(Token::COMMA) &&
                    PerserRule::FunctionVariableDecl();
            },
            []{
                return
                    match(Token::NAME);
            }
        };

        std::array< std::function<bool()>, 2> FunctionDecl{
            []{
                return 
                    match(Token::NAME,"def") &&
                    match(Token::NAME) &&
                    match(Token::LPARENT) &&
                    PerserRule::FunctionVariableDecl() &&
                    match(Token::RPARENT) &&
                    match(Token::COLON) &&
                    match(Token::FIN)
                    ? 2 : 0;
            },
            []{
                return 
                    match(Token::NAME,"def") &&
                    match(Token::NAME) &&
                    match(Token::LPARENT) &&
                    match(Token::RPARENT) &&
                    match(Token::COLON) &&
                    match(Token::FIN) 
                    ? 2 : 0;
            }            
        };

        std::array< std::function<bool()>, 1> IfStatement{
            []{
                return 
                    match(Token::NAME,"if") &&
                    PerserRule::ConditionExpr() &&
                    match(Token::COLON) &&
                    match(Token::FIN) ? 2 : 0;
            }
        };

        std::array< std::function<bool()>, 2> ConditionExpr{
            []{
                return 
                    match(
                        (new TypeSet(Token::NAME))->
                                  OR(Token::NUMBER)
                    ) &&
                    match(Token::EQUAL) &&
                    match(Token::EQUAL) &&
                    match(
                        (new TypeSet(Token::NAME))->
                                  OR(Token::NUMBER)
                    );
            },
            []{
                return 
                    match(
                        (new TypeSet(Token::NAME))->
                                  OR(Token::NUMBER)
                    ) &&
                    match(Token::EXCLAMATION) &&
                    match(Token::EQUAL) &&
                    match(
                        (new TypeSet(Token::NAME))->
                                  OR(Token::NUMBER)
                    );
            }
        };

        std::array< std::function<bool()>, 2> BinaryExpr{
            []{
                return 
                    match(
                    (new TypeSet(Token::NAME))->
                        OR(Token::NUMBER)
                    ) &&
                    match(
                        (new TypeSet(Token::OPE_ADD))->
                            OR(Token::OPE_SUB)->
                            OR(Token::OPE_DIV)->
                            OR(Token::OPE_MUL)
                    ) &&
                    PerserRule::BinaryExpr();
            },
            []{
                return 
                    match(
                        (new TypeSet(Token::NAME))->
                            OR(Token::NUMBER)
                    ) &&
                    match(
                        (new TypeSet(Token::OPE_ADD))->
                            OR(Token::OPE_SUB)->
                            OR(Token::OPE_DIV)->
                            OR(Token::OPE_MUL)
                    ) &&
                    match(
                        (new TypeSet(Token::NAME))->
                            OR(Token::NUMBER)
                    );
            }
        };

        std::array< std::function<bool()>, 4> Statement{
            []{
                return PerserRule::VariableDecl();
            },
            []{
                return PerserRule::IfStatement();
            },
            []{
                return PerserRule::FIN();
            },
            []{
                return PerserRule::FunctionDecl();
            },
        };

        std::array< std::function<bool()>, 2> VariableDecl{
            []{
                return 
                    match(Token::NAME) &&
                    match(Token::EQUAL) && 
                    PerserRule::BinaryExpr() &&
                    match(Token::FIN);
            },
            []{
                return
                    match(Token::NAME) &&
                    match(Token::EQUAL) &&
                    match(Token::NUMBER) &&
                    match(Token::FIN);                
            }
        };
    }

    namespace speculate{

        auto speculate(std::function<bool()> rule)
         -> bool{
            mark();
            bool success = false;
            if(
                rule()
            ){
                success = true;
            }
            release();
            return success;
        }

        template<std::size_t size>
        auto speculate(std::array< std::function<bool()>,size> rules)
         -> int{
            int case_num = 1;
            for(auto rule : rules){
                if(speculate(rule)){
                    return case_num;
                }
                case_num ++;
            }
            return 0;
        }

    };

    namespace PerserRule{

        template<std::size_t size>
        auto Perser(std::array< std::function<bool()>,size> rules)
         -> int{
            int _result = speculate::speculate(rules);
            if(!_result){
                return 0;
            }
            rules[ _result-1 ]();
            return _result;
        }

        auto FIN()
         -> int{
            log(0,"FIN");
            return Perser(Rule::FIN);
        }

        auto FunctionVariableDecl()
         -> int{
            log(0,"FunctionVariableDecl");
            return Perser(Rule::FunctionVariableDecl);
        }

        auto FunctionDecl()
         -> int{
            log(0,"FunctionDecl");
            return Perser(Rule::FunctionDecl);
        }

        auto ConditionExpr()
         -> int{
            log(0,"ConditionExpr");
            return Perser(Rule::ConditionExpr);
        }

        auto IfStatement()
         -> int{
            log(0,"IfStatement");
            return Perser(Rule::IfStatement);
        }

        auto BinaryExpr()
         -> int{
            log(0,"BinaryExpr");  
            return Perser(Rule::BinaryExpr);
        }

        auto Statement()
         -> int{
            log(0,"Statement");  
            return Perser(Rule::Statement);
        }

        auto VariableDecl()
         -> int{
            log(1,"VariableDecl");
            return Perser(Rule::VariableDecl);
        }
    };

    auto perser()
     -> int{
        buf_index = 0;
        while(markers.size()!=0){
            markers.pop();
        }
        headTokens.clear();        
        return PerserRule::Statement();
    }
}

auto main()
 -> int{	
	std::string line;
    std::string term = ">>> ";
    bool nest = false;
	while(true){
		std::cout<<term;
		std::getline(std::cin, line);
		tokens = lexer(line);

        for(auto t : tokens){
            std::cout <<"\""<< t.getName() <<"\"  "<< t.getType() << "\n";
        }

        int result = Perser::perser();
        if(!result){
            std::cout<<"Syntax error! \n";
        }        
        if(result==3 && nest){
            term = "... ";
            continue;
        }
        if(result==2 || result == 4 ){
            term = "... ";
            nest = true;
        }else{
            term = ">>> ";
            nest = false;
        }
        tokens.clear();
	}
	return 0;
}


