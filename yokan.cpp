#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <stack>
#include <map>
#include <functional>
#include <initializer_list>
#include <array> 
#include <fstream>
 
#include <cstdarg>
#include <memory>
#include <iterator>

#include <regex>
#include <chrono>
#include <iomanip>

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

    std::regex reg_special(R"([^0-9^a-z^A-Z]+)");
    std::regex reg_number(R"([0-9]+)");
    std::regex reg_letter(R"([\w]+)");

    auto isLetter(char c)
     -> bool{
        std::string str({c});
        if(std::regex_match( str, reg_letter)){
            return true;
        }
        return false;
    }
    auto isNumber(char c)
     -> bool{

        std::string str({c});
        if(std::regex_match( str, reg_number)){
            return true;
        }
        return false;
    }
    auto isSpecial(char c)
     -> bool{

        // TODO 正規表現にする
        if( c == '^'){
            return true;
        }

        std::string str({c});
        if(std::regex_match( str, reg_special)){
            return true;
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

    bool isFirst = true;

    int buf_index = 0;
    std::stack<int>       markers;
    std::vector<Token> headTokens;
    std::vector<Token> overHeadTokens;
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

    namespace Core{

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
        
        auto margin(int i)
         -> Token{
            return headTokens[buf_index+i-2];
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

    }

    namespace speculate{
        auto speculate(std::function<bool()> rule)
         -> bool{

            Core::mark();
            bool success = false;
            if(
                rule()
            ){
                success = true;
            }
            Core::release();
            return success;
        }

        auto speculate(std::vector< std::function<bool()>> rules)
         -> int{
            int case_num = 1;
            for(auto rule : rules){
                if(speculate(rule)){
                    log(3,"#speculate "+std::to_string(case_num));
                    return case_num;
                }
                case_num ++;
            }
            return 0;
        }
    };

    auto match_2(Token::Type type)
         -> bool{
            Token  token =  Core::LT(2);     

            curString = token.getName();
            //std::cout<<"c:"<<curString<<"\n";
            Token::Type t = token.getType();
            if(type == t){
                Core::nextToken();
                return true;
            }else{
                return false;
            }
    }

    auto match(Token::Type type)
         -> bool{
            Token  token =  Core::LT(1);     
            curString = token.getName();

            Token::Type t = token.getType();
            if(type == t){
                Core::nextToken();
                return true;
            }else{
                return false;
            }
    }

    auto match(Token::Type type, std::string reserved)
     -> bool{
        Token  token =  Core::LT(1);     
     
        curString = token.getName();

        Token::Type t = token.getType();
        if(type == t and curString == reserved){
            Core::nextToken();
            return true;
        }else{
            return false;
        }
    } 

    auto match(std::vector< std::function<bool()>> rules)
     -> int{
        int _result = speculate::speculate(rules);
        if(!_result){
            return 0;
        }
        rules[ _result-1 ]();
        return _result;
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

    namespace Rule{
        // Rules
        std::vector< std::function<bool()>> FIN;
        std::vector< std::function<bool()>> Number;
        std::vector< std::function<bool()>> List;
        std::vector< std::function<bool()>> Operator;
        std::vector< std::function<bool()>> BinaryExpr;
        std::vector< std::function<bool()>> Identifire;
        std::vector< std::function<bool()>> ListVariableDecl;
        std::vector< std::function<bool()>> FunctionDecl;
        std::vector< std::function<bool()>> ConditionExpr;
        std::vector< std::function<bool()>> IfStatement;
        std::vector< std::function<bool()>> Statement;
        std::vector< std::function<bool()>> RightValue;
        std::vector< std::function<bool()>> VariableDecl;

        std::vector< std::function<bool()>> TestCore;


        auto setup()
         -> bool{
            
            {//Fin
                FIN.push_back( 
                    []{
                        return match(Token::FIN);            
                    }
                );
            }

            {//Number
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

            {//ListVariableDecl
                ListVariableDecl.push_back(
                    []{
                        log(1, "ListVariableDecl: RightValue, ListVariableDecl");
                        return
                            match(RightValue) &&
                            match(Token::COMMA) &&
                            match(ListVariableDecl);
                    }
                );
                ListVariableDecl.push_back(
                    []{
                        log(1, "ListVariableDecl: RightValue");
                        return
                            match(RightValue);
                    }
                );
            }

            {//List
                List.push_back(
                    []{
                        
                        bool val1 = match(Token::LBRACKET);
                        bool val2 = match_2(Token::RBRACKET);
                        bool val3 = match(Identifire);
                        log(2, "List Resullt val1:"+std::to_string(val1)+" val1:"+std::to_string(val2)+" val3:"+std::to_string(val3));

                        return val1 && val2 && val3;
/*
                        return 
                            match(Token::LBRACKET) &&
                            match_2(Token::RBRACKET) &&
                            match(Identifire);
*/
                    }
                );
                List.push_back(
                    []{
                        log(1, "List: [ VariableDecl ]");
                        return 
                            match(Token::LBRACKET) &&                           
                            match(ListVariableDecl) &&
                            match(Token::RBRACKET);
                    }
                );
                List.push_back(
                    []{
                        log(1, "List: [ ]");
                        return 
                            match(Token::LBRACKET) &&                           
                            match(Token::RBRACKET);
                    }
                );        
            }

            {//Operator
                Operator.push_back(
                    []{
                        return match(Token::OPE_ADD);
                    }
                );
                Operator.push_back(
                    []{
                        return match(Token::OPE_SUB);
                    }
                );
                Operator.push_back(
                    []{
                        return match(Token::OPE_DIV);
                    }
                );
                Operator.push_back(
                    []{
                        return match(Token::OPE_MUL);
                    }
                );
            }

            {//BinaryExpr
                BinaryExpr.push_back(
                    []{
                        return 
                            match(Identifire) &&
                            match(Operator) &&
                            match(BinaryExpr);
                    }
                );
                BinaryExpr.push_back(
                     []{
                        return 
                            match(Identifire) &&
                            match(Operator) &&
                            match(Identifire);
                    }
                );
            }

            {//Identifire
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

            {//FunctionDecl
                FunctionDecl.push_back(
                    []{
                        return 
                            match(Token::NAME,"def") &&
                            match(Token::NAME) &&
                            match(Token::LPARENT) &&
                            match(ListVariableDecl) &&
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

            {//ConditionExpr
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

            {//IfStatement
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

            {//Statement
                Statement.push_back(
                    []{
                        bool val = match(VariableDecl);
                        log(2, "Statement:VariableDecl Resullt val:"+std::to_string(val));
                        return val;

//                        return match(VariableDecl);
                    }
                );
                Statement.push_back(
                    []{
                        bool val = match(IfStatement);
                        log(2, "Statement:IfStatement Resullt val:"+std::to_string(val));
                        return val;

//                        return match(IfStatement);
                    }
                );
                Statement.push_back(
                    []{

                        bool val = match(FIN);
                        log(2, "Statement:FIN Resullt val:"+std::to_string(val));
                        return val;
//                        return match(FIN);
                    }
                );
                Statement.push_back(
                    []{

                        bool val = match(FunctionDecl);
                        log(2, "Statement:FunctionDecl Resullt val:"+std::to_string(val));
                        return val;
//                        return match(FunctionDecl);
                    }
                );
            }

            {//RightValue
                RightValue.push_back(
                    []{
                        log(2, "RightValue: BinaryExpr");

                        bool val = match(BinaryExpr);
                        log(2, "RightValue:BinaryExpr val:"+std::to_string(val));
                        return val;

//                        return match(BinaryExpr);
                    }
                );
                RightValue.push_back(
                    []{
                        log(2, "RightValue: Identifire");
                        bool val = match(Identifire);
                        log(2, "RightValue:Identifire Resullt val:"+std::to_string(val));
                        return val;

//                        return match(Identifire);
                    }
                );
                RightValue.push_back(
                    []{
                        log(2, "RightValue: List");

                        bool val = match(List);
                        log(2, "RightValue:List Resullt val:"+std::to_string(val));
                        return val;

//                        return match(List);
                    }
                );
            }

            {//VariableDecl
                VariableDecl.push_back(
                    []{
                            bool val1 = match(Token::NAME); 
                            bool val2 = match(Token::EQUAL);
                            bool val3 = match(RightValue);
                            bool val4;
                            // match
                            Token  token =  Core::LT(1);     
                                        /*         
                                        std::cout<<"############\n";
                                        for(auto t : headTokens){
                                            std::cout<<"headTokens:"<<t.getName()<<"\n";
                                        }
                                        */
                                        curString = token.getName();
                                        
                                        Token::Type t = token.getType();
                                        if(Token::FIN == t){
                                            Core::nextToken();
                                            val4 = true;
                                        }else{
                                            val4 = false;
                                        }
                            // match
//                          bool val4 = match(Token::FIN);

                            log(2, "VariableDecl Resullt val1:"+std::to_string(val1)+" val1:"+std::to_string(val2)+" val3:"+std::to_string(val3)+" val4:"+std::to_string(val4));
                            return val1 && val2 && val3 && val4;
/*                          
                        return 
                            match(Token::NAME) &&
                            match(Token::EQUAL) && 
                            match(RightValue) &&
                            match(Token::FIN);
*/
                    }
                );
            }

            {//TestCore 
                TestCore.push_back(
                    []{
                        log(1, "List: [ Identifire ]");
                        return 
                            match(Token::LBRACKET) &&
                            match_2(Token::RBRACKET) &&
                            match(Identifire);
                    }
                );
                TestCore.push_back(
                    []{
                        log(1, "List: [ VariableDecl ]");
                        return 
                            match(Token::LBRACKET) &&                           
                            match(ListVariableDecl) &&
                            match(Token::RBRACKET);
                    }
                );
                TestCore.push_back(
                    []{
                        log(1, "List: [ ]");
                        return 
                            match(Token::LBRACKET) &&                           
                            match(Token::RBRACKET);
                    }
                );                        
            }

            return true;
        }        
    }

    auto perser()
     -> int{
        buf_index = 0;
        while(markers.size()!=0){
            markers.pop();
        }
        headTokens.clear();   
        
        if(isFirst){
            Rule::setup();     
            isFirst = false;
        }
        
        return match(Rule::Statement);
    }
}

int main(int argc, char* argv[]){	

    std::ifstream ifs;

    if(argc == 2){
        ifs.open(argv[1]);
        if(!ifs){
            std::cout<<"error: no such file or directory: '"<< argv[1] <<"'\n";
        }
        std::istreambuf_iterator<char> it(ifs);
        std::istreambuf_iterator<char> last;
        std::string data(it, last); 

        auto sTime = std::chrono::system_clock::now();
        tokens = Lexer::lexer(data);

        for(auto t : tokens){
            std::cout <<"\""<< t.getName() <<"\"  "<< t.getType() << "\n";
        }

        auto result = Perser::perser();
        auto eTime = std::chrono::system_clock::now();
        auto timeSpan = eTime - sTime;
        std::cout<< "Time:"<<std::setw(6)<< std::chrono::duration_cast<std::chrono::milliseconds>(timeSpan).count() <<"[ms]\n";

        if(!result){
            std::cout<<"Syntax error! \n";
        }else{
            std::cout<<"Syntax correct! \n";
        }

        return 0;
    }

	std::string line;
    std::string term = ">>> ";
    bool nest = false;
	while(true){
		std::cout<<term;
		std::getline(std::cin, line);

        auto sTime = std::chrono::system_clock::now();

		tokens = Lexer::lexer(line);
        /*
        for(auto t : tokens){
            std::cout <<"\""<< t.getName() <<"\"  "<< t.getType() << "\n";
        }
        */
        int result = Perser::perser();
        auto eTime = std::chrono::system_clock::now();
        auto timeSpan = eTime - sTime;
        std::cout<< "Time: "<<std::setw(6)<< std::chrono::duration_cast<std::chrono::milliseconds>(timeSpan).count() <<"[ms]\n";
        
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
