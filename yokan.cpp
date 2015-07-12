#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <stack>
#include <map>

#include "kushi.hpp"

class Token{
  public:
    enum Type{
        FIN,
        NAME,
        NUMBER,
        IDENTIFIER,
        LPARENT,
        RPARENT,
        RBRACKET,
        LBRACKET,
        SEMICOLON,
        COLON,
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
    std::string getVal(){
    	return name;
	}
  private:
    std::string name;
    Type type;
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
 	char list[] = "(){}[];:=+";
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
            case ';':
                tokens.push_back(Token(Token::SEMICOLON,";"));
                break;
            case ':':
                tokens.push_back(Token(Token::COLON,":"));
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
    }else{
        std::cout<<"Error! invalid charator:"<< buffer <<std::endl;
    }

    tokens.push_back(Token(Token::FIN,"<FIN>"));

	return tokens;
}


namespace Perser{
    int buf_index = 0;
    std::stack<int>       markers;
    std::vector<Token> headTokens;


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
     //   curString = token.getName();
        Token::Type t = token.getType();
        if(type==t){
            nextToken();
            return true;
        }else{
            return false;
        }
    }


    namespace PerserRule{
        bool BinaryExpr();
        bool VariableDecl();
        bool Statement();
    };

    namespace speculate{

        /* 
            BinaryExpr ->
                Name, OPE_ADD, Name
        */
        auto speculate_BinaryExpr()
         -> int{
            int success = 0;
            mark();
            if(
                match(Token::NAME) &&
                match(Token::OPE_ADD) &&
                match(Token::NAME)
            ){
                success = 1;
            }else if(
                match(Token::NAME) &&
                match(Token::OPE_ADD) &&
                match(Token::NUMBER)
            ){
                success = 2;
            }
            release();
            return success;
        }

        /* 
            Statement ->
                VariableDecl,
        */
        auto speculate_Statement()
         -> int{
            int success = 0;
            mark();
            if(
                PerserRule::VariableDecl()
            ){
                success = 1;
            }else if(
                PerserRule::BinaryExpr()
            ){
                success = 2;
            }
            release();
            return success;
        }

        /* 
            VariableDecl ->
                NAME, EQUAL, NUMBER
        */
        auto speculate_VariableDecl()
         -> bool{
            bool success = true;
            mark();
            if(!(
            /* Write rule */
            match(Token::NAME) &&
            match(Token::EQUAL) && 
            match(Token::NUMBER) 
            /* ---------- */
            )){
                success = false;
            }
            release();
            return success;
         }
    };

    namespace PerserRule{

        auto BinaryExpr()
         -> bool{
            switch(speculate::speculate_BinaryExpr()){
                case 1:
                    match(Token::NAME);
                    match(Token::OPE_ADD);
                    match(Token::NAME);
                    return true;
                case 2:
                    match(Token::NAME);
                    match(Token::OPE_ADD);
                    match(Token::NUMBER);
                    return true;
                default:
                    return false;
            }
        }

        auto Statement()
         -> bool{
            switch(speculate::speculate_Statement()){
                case 1:
                    VariableDecl();
                    return true;
                case 2:
                    BinaryExpr();
                    return true;
                default:
                    return false;
            }
        }

        auto VariableDecl()
         -> bool{
            if(speculate::speculate_VariableDecl()){
                match(Token::NAME);
                match(Token::EQUAL); 
                match(Token::NUMBER); 
                return true;
            }
            return false;  
        }
    };

    auto perser()
     -> bool{
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
	while(true){
		std::cout<<">>> ";
		std::getline(std::cin, line);
		tokens = lexer(line);
        for(auto t : tokens){
            std::cout << t.getVal() <<","<< t.getType() << "\n";
        }
        if(!Perser::perser()){
            std::cout<<"Syntax error! \n";
            return 0;
        }
        tokens.clear();
	}
	return 0;
}


