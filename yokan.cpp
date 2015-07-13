#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <stack>
#include <map>

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
        log(3,"cur:" + curString );

        Token::Type t = token.getType();
        log(1, "hope:"+std::to_string(type)+" real:"+std::to_string(t));
        if(type==t){
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
        log(3,"cur:" + curString );

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
        int BinaryExpr();
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
            log(1,"speculate_BinaryExpr");

            int success = 0;
            mark();
            if(
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
                PerserRule::BinaryExpr() != -1
            ){
                success = 1;
                release();
                log(2,"speculate_BinaryExpr (1)- success");                
                return success;
            }
            release();

            mark();
            if(
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
                )
            ){
                success = 2;
                release();
                log(2,"speculate_BinaryExpr (2)- success");                
                return success;
            }
            release();
            log(2,"speculate_BinaryExpr - failed");            
            return success;
        }

        /* 
            Statement ->
                VariableDecl,
        */
        auto speculate_Statement()
         -> int{
            log(1,"speculate_Statement");
            int success = 0;
            mark();
            if(
                PerserRule::VariableDecl()
            ){
                success = 1;
                release();
                log(2,"speculate_Statement - success");
                return success;
            }
            release();
            log(2,"speculate_Statement - failed");
            return success;
        }

        /* 
            VariableDecl ->
                NAME, EQUAL, NUMBER
        */
        auto speculate_VariableDecl()
         -> int{
            log(1,"speculate_VariableDecl");
            int success = 0;

            mark();
            if(
            /* Write rule */
            match(Token::NAME) &&
            match(Token::EQUAL) && 
            PerserRule::BinaryExpr() &&
            match(Token::FIN)
            /* ---------- */
            ){
                success = 1;
                release();
                log(2,"speculate_VariableDecl (1)- success");
                return success;
            }
            release();
            log(3, "second!!!!!!!!");
            mark();
            if(
            /* Write rule */
            match(Token::NAME) &&
            match(Token::EQUAL) &&
            match(Token::NUMBER) &&
            match(Token::FIN)
            /* ---------- */
            ){
                success = 2;
                release();
                log(2,"speculate_VariableDecl (2)- success");
                return success;
            }
            release();
            log(2,"speculate_VariableDecl - failed");
            return success;
         }
    };

    namespace PerserRule{


        // そのまま値をここで返すべき？
        auto BinaryExpr()
         -> int{
            log(0,"BinaryExpr");  
            int _r_val, _l_val;  

            switch(speculate::speculate_BinaryExpr()){
                case 1:
                    log(1,"BinaryExpr.1"); 

                    match(
                    (new TypeSet(Token::NAME))->
                        OR(Token::NUMBER)
                    );

                    match(
                        (new TypeSet(Token::OPE_ADD))->
                            OR(Token::OPE_SUB)->
                            OR(Token::OPE_DIV)->
                            OR(Token::OPE_MUL)
                    );

                    _r_val = BinaryExpr();
                    if(_r_val != -1){
                        return 1;
                    }else{
                        log(3,"Error undefined:["+curString+"] !!");
                        return -1;
                    }

                case 2:
                    log(1,"BinaryExpr.1"); 
                    match(
                    (new TypeSet(Token::NAME))->
                        OR(Token::NUMBER)
                    );

                    match(
                        (new TypeSet(Token::OPE_ADD))->
                            OR(Token::OPE_SUB)->
                            OR(Token::OPE_DIV)->
                            OR(Token::OPE_MUL)
                    );

                    match(
                    (new TypeSet(Token::NAME))->
                        OR(Token::NUMBER)
                    );
                /*
                    if(defVariable(curString)){
                        _l_val = variableTable[curString];
                    }else{
                        log(3,"Error undefined:["+curString+"] !!");
                        return -1;
                    }
                */
                    return 1;                    
                default:
                    return -1;
            }
            return -1;
        }

        auto Statement()
         -> bool{
            log(0,"Statement");            
            switch(speculate::speculate_Statement()){
                case 1:
                    log(1,"Statement.VariableDecl"); 
                    VariableDecl();
                    return true;
                default:
                    return false;
            }
        }

        auto VariableDecl()
         -> bool{
            log(1,"VariableDecl");

            std::string _val_name;

            int _specilate_result = speculate::speculate_VariableDecl();
            if(!_specilate_result){
                return false;
            }

            match(Token::NAME);
            _val_name = curString;

            match(Token::EQUAL); 

            switch(_specilate_result){
                case 1:
                    BinaryExpr();
                    break;
                case 2:
                    match(Token::NUMBER);
                    break;
            }
            match(Token::FIN);
            return true;  
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
/*
        for(auto t : tokens){
            std::cout << t.getName() <<","<< t.getType() << "\n";
        }
*/
        if(!Perser::perser()){
            std::cout<<"Syntax error! \n";
//            return 0;
        }
        tokens.clear();
	}
	return 0;
}


