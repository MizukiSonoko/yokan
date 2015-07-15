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
        PERIOD,
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


std::map<std::string, int> values;
std::list<Token>  tokens;

namespace Lexer{

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
     	char list[] = "!#$%&'\",.<>(){}[];:=+";
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
                case '.':
                    tokens.push_back(Token(Token::PERIOD,"."));
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
};

namespace Perser{
    int buf_index = 0;
    std::stack<int>       markers;
    std::vector<Token> headTokens;
    std::string         curString;

    std::map<std::string, int> variableTable;
    std::map<std::string, int> functionTable;

    void log(int layour, std::string msg){
    #ifdef DEBUG
        for(int i=0; i < layour; i++){
            std::cout<< " ";
        }
        std::cout<< msg <<"\n";
    #endif
    }

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
        if(type == t){
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

    auto defVariable(std::string val_name)
     -> bool{
        if(variableTable.find(val_name) == variableTable.end()){
            return false;
        }
        return true;
    }

    namespace AST{
        class AST{

        };

        class FunctionVariableDeclAST : public AST{

        };

        class FunctionDeclAST : public AST{

        };

        class ConditionExprAST : public AST{

        };     

        class IfStatementAST : public AST{

        };     

        class BinaryExprAST : public AST{

        };            

        class VariableDeclAST : public AST{

        };            
     
        class StatementAST : public AST{
            public:
        };

        class TranslationAST : public AST{
            public:    
                // * TODO 積極的にsetter gettterにしていこうな
                std::vector<StatementAST*> statementAst; 

                ~TranslationAST(){
                    for(auto s : statementAst){
                        delete s;
                        s = NULL;
                    }
                }
        };
    };

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

        auto speculate(std::vector< std::function<bool()>> rules)
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

    auto match(std::vector< std::function<bool()>> rules)
     -> int{
        int _result = speculate::speculate(rules);
        if(!_result){
            return 0;
        }
        rules[ _result-1 ]();
        return _result;
    }

    template<std::size_t size>
    auto match(std::array< std::function<bool()>,size> rules)
     -> int{
        int _result = speculate::speculate(rules);
        if(!_result){
            return 0;
        }
        rules[ _result-1 ]();
        return _result;
    }

    namespace PerserRule{
        int NUMBER();
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

        std::vector< std::function<bool()>> FIN;
        std::vector< std::function<bool()>> Number;
        std::vector< std::function<bool()>> BinaryExpr;
        std::vector< std::function<bool()>> Identifire;
        std::vector< std::function<bool()>> FunctionVariableDecl;
        std::vector< std::function<bool()>> FunctionDecl;
        std::vector< std::function<bool()>> ConditionExpr;
        std::vector< std::function<bool()>> IfStatement;
        std::vector< std::function<bool()>> Statement;
        std::vector< std::function<bool()>> VariableDecl;

        auto setup()
         -> bool{
            
            {
                FIN.push_back( 
                    []{
                        return match(Token::FIN);            
                    }
                );
            }

            {
                Number.push_back(
                    []{
                        return
                            match(Token::NUMBER) &&
                            match(Token::PERIOD) &&
                            match(Token::NUMBER);
                    }
                );
                Number.push_back(
                    []{
                        return 
                            match(Token::NUMBER);
                    }
                );
            }

            {
                BinaryExpr.push_back(
                    []{
                        return 
                            match(Identifire) &&
                            (
                                match(Token::OPE_ADD) ||
                                match(Token::OPE_SUB) ||
                                match(Token::OPE_DIV) ||
                                match(Token::OPE_MUL)
                            ) &&
                            match(BinaryExpr);
                    }
                );
                BinaryExpr.push_back(
                     []{
                        return 
                            match(Identifire) &&
                            (
                                match(Token::OPE_ADD) ||
                                match(Token::OPE_SUB) ||
                                match(Token::OPE_DIV) ||
                                match(Token::OPE_MUL)
                            ) && 
                           match(Identifire);

                    }
                );
            }

            {
                Identifire.push_back(
                    []{
                        return match(Number);
                    }
                );
                Identifire.push_back(
                    []{
                        return match(Token::NAME);
                    }
                );
            }

            {
                FunctionVariableDecl.push_back(
                    []{
                        return
                            match(Token::NAME) &&
                            match(Token::COMMA) &&
                            match(FunctionVariableDecl);
                    }
                );
                FunctionVariableDecl.push_back(
                    []{
                        return
                            match(Token::NAME);
                    }
                );
            }

            {
                FunctionDecl.push_back(
                    []{
                        return 
                            match(Token::NAME,"def") &&
                            match(Token::NAME) &&
                            match(Token::LPARENT) &&
                            match(FunctionVariableDecl) &&
                            match(Token::RPARENT) &&
                            match(Token::COLON) &&
                            match(Token::FIN)
                            ? 2 : 0;
                    }
                );
                FunctionDecl.push_back(
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
                );
            }

            {
                ConditionExpr.push_back(
                    []{
                        return 
                            match(Identifire) &&
                            match(Token::EQUAL) &&
                            match(Token::EQUAL) &&
                            match(Identifire);
                    }                    
                );
                ConditionExpr.push_back(
                    []{
                        return 
                            match(Identifire) &&
                            match(Token::EXCLAMATION) &&
                            match(Token::EQUAL) &&
                            match(Identifire);
                    }
                );
            }

            {
                IfStatement.push_back(
                    []{
                        return 
                            match(Token::NAME,"if") &&
                            match(ConditionExpr) &&
                            match(Token::COLON) &&
                            match(Token::FIN) ? 2 : 0;
                    }
                );
            }

            {
                Statement.push_back(
                    []{
                        return match(VariableDecl);
                    }
                );
                Statement.push_back(
                    []{
                        return match(IfStatement);
                    }
                );
                Statement.push_back(
                    []{
                        return match(FIN);
                    }
                );
                Statement.push_back(
                    []{
                        return match(FunctionDecl);
                    }
                );
            }

            {
                VariableDecl.push_back(
                    []{
                        return 
                            match(Token::NAME) &&
                            match(Token::EQUAL) && 
                            match(BinaryExpr) &&
                            match(Token::FIN);
                    }
                );
                VariableDecl.push_back(
                    []{
                        return
                            match(Token::NAME) &&
                            match(Token::EQUAL) &&
                            match(Number) &&
                            match(Token::FIN);    
                    }
                );
            }
            return true;
        }        
    }

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
    };

    auto perser()
     -> int{
        buf_index = 0;
        while(markers.size()!=0){
            markers.pop();
        }
        headTokens.clear();   
        Rule::setup();     
        return match(Rule::Statement);
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
		tokens = Lexer::lexer(line);
/*
        for(auto t : tokens){
            std::cout <<"\""<< t.getName() <<"\"  "<< t.getType() << "\n";
        }
*/
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


