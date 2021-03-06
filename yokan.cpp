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

#define RELEASE(x) {delete x;x=NULL;}
#define RELEASEA(x) {delete[] x;x=NULL;}

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
        CARET,
        AT_SIGN,
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

    std::regex reg_special(R"(\W+)");
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
                case '^':
                    tokens.push_back(Token(Token::CARET,"^"));                    
                    break;
                case '@':
                    tokens.push_back(Token(Token::AT_SIGN,"@"));
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

namespace parser{

    bool isFirst = true;

    int buf_index = 0;
    std::stack<int>       markers;
    std::vector<Token> headTokens;
    std::vector<Token> overHeadTokens;
    std::string         curString;

    std::map<std::string, int> variableTable;
    std::map<std::string, int> functionTable;

    std::string cur_val_name;

    void log(int layour, std::string msg){
    #ifdef DEBUG
        for(int i=0; i < layour; i++){
            std::cout<< " ";
        }
        std::cout<< msg <<"\n";
    #endif
    }


    auto defVariable(std::string val_name)
         -> bool{
        if(variableTable.find(val_name) == variableTable.end()){
            return false;
        }
        return true;
    }

    namespace AST{
        enum AstID{
            NONE = -1,
            FINID,
            NameID,
            NumberID,
            ListID,
            OperatorID,
            BinaryExprID,
            IdentifireID,
            ListVariableDeclID,
            FunctionDeclID,
            ConditionExprID,
            IfStatementID,
            StatementID,
            RightValueID,
            VariableDeclID
        };

        class AST{

                std::multimap<AstID, AST*> subAST;
                AstID type;
                std::string value;
            public:
                /* Using only speculate.*/
                bool isCorrect;
                AST(bool isC):
                    type(NONE),value(""),isCorrect(isC){};

                AST():type(FINID),value("<FIN>"){};
                AST(AstID type):type(type){};
                AST(AstID type,std::string value):type(type),value(value){};
                ~AST(){
                    for(auto it = subAST.begin(); it != subAST.end(); it++){ 
                        delete it->second;
                    } 
                }
                auto add(AstID type, AST* obj)
                 -> AST*{
                    this->subAST.insert(std::make_pair( type, obj));
                    return this;
                }
                auto print(int pos) 
                 -> void{
                    if( subAST.size() == 0 ){
                        log(pos, AstID2s(type) + ":" + value );
                    }else{
                        log(pos, AstID2s(type) + ":" + value );
                        for(auto it = subAST.begin(); it != subAST.end(); ++it){ 
                            it->second->print(pos + 2);
                        } 
                    }
                }
                std::multimap<AstID, AST*> getSubAST(){
                    return subAST;
                }
                bool is(AstID id){
                    return id == type;
                }
                bool has(AstID id){
                    return subAST.find(id) != subAST.end();
                }

                std::string getValue(){
                    return value;
                }

                std::vector<AST*> get(AstID id){
                    std::vector<AST*> res;
                    auto it  = subAST.lower_bound(id);
                    auto end = subAST.upper_bound(id);
                    while(it != end){
                        res.push_back( it-> second);
                        ++it;
                    }                       
                    return res;
                } 

                // For debug.
                std::string AstID2s(AstID id){
                    switch(id){
                    case NONE: return "NONE";
                    case FINID: return "Fin";
                    case NameID: return "Name";
                    case NumberID: return "Number";
                    case ListID: return "List";
                    case OperatorID: return "Operator";
                    case BinaryExprID: return "BinaryExpr";
                    case IdentifireID: return "Identifire";
                    case ListVariableDeclID: return "ListVariableDecl";
                    case FunctionDeclID: return "functionTable";
                    case ConditionExprID: return "ConditionExpr";
                    case IfStatementID: return "IfStatement";
                    case StatementID: return "Statement";
                    case RightValueID: return "RightValue";
                    case VariableDeclID: return "VariableDecl";
                    }
                }

                     auto execBinary(std::string ope,int a,int b)
                      -> int {
                        int result = -1;
                        if(ope == "+"){
                            result = a + b;
                        }else if(ope == "-"){
                            result = a - b;
                        }else if(ope == "*"){
                            result = a * b;
                        }else if(ope == "/"){
                            result = a / b;
                        }
                        return result;
                     }

//                namespace binaryExpr{
                    auto binaryExpr(AST* ast)
                     -> int{
                        auto ope = ast->get(OperatorID).at(0)->getValue();
                        auto identifires = ast->get(IdentifireID);
                        if(identifires.size() == 2){
                                return execBinary(ope, 
                                    std::stoi(identifires.at(0)->get(NumberID).at(0)->getValue()),
                                    std::stoi(identifires.at(1)->get(NumberID).at(0)->getValue()));
                        }else if(identifires.size() == 1){
                                int rightVal = std::stoi(identifires.at(0)->get(NumberID).at(0)->getValue());
                                int leftVal  = ast->get(BinaryExprID).at(0)->binaryExpr();
                                return execBinary(ope, rightVal, leftVal);
                        }
                        return -1;
                     }

                    auto binaryExpr()
                     -> int{
                        return binaryExpr(this);
                    }
                     
  //              };

                auto walk() 
                 -> void{
                    if( subAST.size() != 0 ){
                        if(this->type == VariableDeclID){
                            std::cout<<" variable decl\n";
                            auto name = get(NameID).at(0)->getValue();
                            std::cout<<"Name is "<< name << std::endl;
                            std::vector<AST*> rightValue = get(RightValueID);
                            if(rightValue.at(0)->has(BinaryExprID)){
                                int result = rightValue.at(0)->get(BinaryExprID).at(0)->binaryExpr();
                                std::cout <<"result = "<< result <<"\n";
                            }
                        }else if(this->type == BinaryExprID){
                            int result = binaryExpr();
                            std::cout <<" result = "<< result <<"\n";
                        }

                        for(auto it = subAST.begin(); it != subAST.end(); ++it){ 
                            it->second->walk();
                        }
                    }
                }
        };

   };

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
        auto speculate(std::function<AST::AST*(bool)> rule)
         -> bool{

            Core::mark();
            bool success = false;
            if(
                rule(true)->isCorrect
            ){
                success = true;
            }
            Core::release();
            return success;
        }

        auto speculate(std::vector< std::function<AST::AST*(bool)>> rules)
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

    auto match(std::vector< std::function<AST::AST*(bool)>> rules)
     -> AST::AST* {        
        int _result = speculate::speculate(rules);
        if(!_result){
            return nullptr;
        }
        return rules[ _result-1 ](false);
    }
    
    namespace Rule{
        // Rules
        std::map<AST::AstID, std::vector< std::function<AST::AST*(bool)>>> rules;
        std::vector< std::function<AST::AST*(bool)>> Number;
        std::vector< std::function<AST::AST*(bool)>> List;
        std::vector< std::function<AST::AST*(bool)>> Operator;
        std::vector< std::function<AST::AST*(bool)>> BinaryExpr;
        std::vector< std::function<AST::AST*(bool)>> Identifire;
        std::vector< std::function<AST::AST*(bool)>> ListVariableDecl;
        std::vector< std::function<AST::AST*(bool)>> ConditionExpr;
        std::vector< std::function<AST::AST*(bool)>> IfStatement;
        std::vector< std::function<AST::AST*(bool)>> Statement;
        std::vector< std::function<AST::AST*(bool)>> RightValue;
        std::vector< std::function<AST::AST*(bool)>> VariableDecl;

        std::vector< std::function<AST::AST*(bool)>> TestCore;

        auto setup()
         -> bool{

            {//Fin
                rules[AST::FINID].push_back( 
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::FIN) ){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            return new AST::AST(AST::FINID,"<FIN>");
                        }
                    }
                );
            }

            {//Number
                Number.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if(
                                match(Token::NUMBER) &&
                                match(Token::PERIOD) &&
                                match(Token::NUMBER)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            match(Token::NUMBER);
                            std::string _int = curString;
                            match(Token::PERIOD);
                            match(Token::NUMBER);
                            std::string _frac = curString;
                            return new AST::AST(AST::NumberID, _int + "." + _frac);
                        }
                    }
                );
                Number.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){                       
                            if( match(Token::NUMBER) ){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);                                
                            }
                        }else{
                            match(Token::NUMBER);
                            return new AST::AST(AST::NumberID, curString);
                        }
                    }
                );
            }

            {//ListVariableDecl
                ListVariableDecl.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(RightValue) &&
                                match(Token::COMMA) &&
                                match(ListVariableDecl)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            auto _rightValue = match(RightValue);
                            match(Token::COMMA);
                            auto _listVariableDecl = match(ListVariableDecl);
                            return (new AST::AST(AST::ListVariableDeclID))
                                ->add(AST::RightValueID, _rightValue)
                                ->add(AST::ListVariableDeclID, _listVariableDecl);
                        }
                    }
                );
                ListVariableDecl.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(RightValue) ){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            return (new AST::AST(AST::ListVariableDeclID))
                                ->add(AST::RightValueID, match(RightValue));
                        }
                    }
                );
            }

            {//List
                List.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::LBRACKET) &&
                                match(Identifire) &&
                                match(Token::RBRACKET)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            match(Token::LBRACKET);
                            auto _identifire = match(Identifire);
                            match(Token::RBRACKET);
                            return (new AST::AST(AST::ListID))
                                ->add(AST::IdentifireID, _identifire);
                        }
                    }
                );
                List.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::LBRACKET) &&                           
                                match(ListVariableDecl) &&
                                match(Token::RBRACKET)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            match(Token::LBRACKET);                        
                            auto _listVariableDecl = match(ListVariableDecl);
                            match(Token::RBRACKET);
                            return (new AST::AST(AST::ListID))
                                ->add(AST::ListVariableDeclID, _listVariableDecl);
                        }
                    }
                );
                List.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::LBRACKET) &&                           
                                match(Token::RBRACKET)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            match(Token::LBRACKET);                       
                            match(Token::RBRACKET);    
                            return new AST::AST(AST::ListID, "[]");
                        }
                    }
                );        
            }

            {//Operator
                Operator.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(Token::OPE_ADD) ?
                                new AST::AST(true) :
                                new AST::AST(false);                                    
                        }else{
                            match(Token::OPE_ADD);
                            return new AST::AST(AST::OperatorID,"+");
                        }
                    }
                );
                Operator.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(Token::OPE_SUB) ?
                                new AST::AST(true) :
                                new AST::AST(false);                                    
                        }else{
                            match(Token::OPE_SUB);
                            return new AST::AST(AST::OperatorID,"-");
                        }
                    }
                );
                Operator.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(Token::OPE_MUL) ?
                                new AST::AST(true) :
                                new AST::AST(false);                                    
                        }else{
                            match(Token::OPE_MUL);
                            return new AST::AST(AST::OperatorID,"*");
                        }
                    }
                );
                Operator.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(Token::OPE_DIV) ?
                                new AST::AST(true) :
                                new AST::AST(false);                                    
                        }else{
                            match(Token::OPE_DIV);
                            return new AST::AST(AST::OperatorID,"/");
                        }
                    }
                );
            }

            {//BinaryExpr
                BinaryExpr.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if ( match(Identifire) &&
                                 match(Operator) &&
                                 match(BinaryExpr)){
                                return new AST::AST(true);   
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            auto _identifire = match(Identifire);
                            auto _operator = match(Operator);
                            auto _binaryExpr = match(BinaryExpr);      
                            return (new AST::AST(AST::BinaryExprID))
                                ->add(AST::IdentifireID, _identifire)
                                ->add(AST::OperatorID, _operator)
                                ->add(AST::BinaryExprID, _binaryExpr);
                        }
                    }
                );
                BinaryExpr.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Identifire) &&
                                match(Operator) &&
                                match(Identifire)){
                                return new AST::AST(true);   
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            auto _identifire1 = match(Identifire);
                            auto _operator = match(Operator);
                            auto _identifire2 = match(Identifire);                            

                            return (new AST::AST(AST::BinaryExprID))
                                ->add(AST::IdentifireID, _identifire1)
                                ->add(AST::OperatorID, _operator)
                                ->add(AST::IdentifireID, _identifire2);                            
                        }
                    }
                );
            }

            {//Identifire
                Identifire.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if ( match(Number) ){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            AST::AST* _number = match(Number);
                            return (new AST::AST(AST::IdentifireID))
                                ->add(AST::NumberID, _number);
                        }
                    }
                );
                Identifire.push_back(                    
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if ( match(Token::NAME) ){
                                return new AST::AST(true);
                            }else{

                                return new AST::AST(false);
                            }
                        }else{
                            match(Token::NAME);
                            return new AST::AST(AST::IdentifireID, curString);
                        }
                    }
                );
            }

            /*
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
        */
            {//Statement

                Statement.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(VariableDecl) ?
                                new AST::AST(true) :
                                new AST::AST(false);
                        }else{
                            auto _variableDecl = match(VariableDecl);
                            return (new AST::AST(AST::StatementID))
                                ->add(AST::VariableDeclID, _variableDecl);
                        }
                    }
                );
                Statement.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(rules[AST::FINID]) ?
                                new AST::AST(true) :
                                new AST::AST(false);
                        }else{
                            match(rules[AST::FINID]);
                            return (new AST::AST(AST::StatementID))
                                ->add(AST::FINID, new AST::AST(AST::FINID));
                        }
                    }
                );
            }

            {//RightValue
                RightValue.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(BinaryExpr) ?
                                new AST::AST(true) :
                                new AST::AST(false);
                        }else{
                            auto _binaryExpr = match(BinaryExpr);
                            return (new AST::AST(AST::RightValueID))
                                ->add(AST::BinaryExprID, _binaryExpr);
                        }
                    }
                );
                RightValue.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(Identifire) ?
                                new AST::AST(true) :
                                new AST::AST(false);
                        }else{
                            auto _identifire = match(Identifire);
                            return (new AST::AST(AST::RightValueID))
                                ->add(AST::IdentifireID, _identifire);
                        }
                    }
                );
                RightValue.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(List) ?
                                new AST::AST(true) :
                                new AST::AST(false);
                        }else{
                            auto _list = match(List);
                            return (new AST::AST(AST::RightValueID))
                                ->add(AST::ListID, _list);
                        }
                    }
                );
            }

            {//VariableDecl
                VariableDecl.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::NAME) &&
                                match(Token::EQUAL) &&
                                match(RightValue) &&
                                match(Token::FIN)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            match(Token::NAME);
                            auto _name = curString;
                            log(0,"define "+ curString);
                            variableTable[curString] = 1;

                            match(Token::EQUAL);
                            auto _rightValue = match(RightValue);
                            match(Token::FIN);
                            return (new AST::AST(AST::VariableDeclID))
                                ->add(AST::NameID, new AST::AST(AST::NameID, _name))
                                ->add(AST::RightValueID, _rightValue);
                        }
                    }
                );
            }

            {//TestCore 
                TestCore.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                                if( match(Token::LBRACKET) &&
                                    match(Identifire) &&
                                    match(Token::RBRACKET)){
                                    return new AST::AST(true);
                                }else{
                                    return new AST::AST(false);
                                }
                        }else{
                            match(Token::LBRACKET);
                            AST::AST* _identifire = match(Identifire);
                            match(Token::LBRACKET);
                            return (new AST::AST(AST::AstID::ListID))
                                ->add(AST::IdentifireID, _identifire);
                        }
                    }
                );            
            }
            return true;
        }        
    }
    /*
        namespace CodeGen{
            void CodeGen(){
                llvm::LLVMContext& context = llvm::getGlobalContext();
                llvm::Module *module = new llvm::Module("top", context);
                llvm::IRBuilder<> builder(context); 
            }
        }
    */
    auto parser()
     -> AST::AST*{
        buf_index = 0;
        while(markers.size()!=0){
            markers.pop();
        }
        headTokens.clear();   
        
        if(isFirst){
            Rule::setup();     
            isFirst = false;
        }
        
        auto result = match(Rule::Statement);
        return result;
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

        parser::AST::AST* result = parser::parser();
        auto eTime = std::chrono::system_clock::now();
        auto timeSpan = eTime - sTime;
        std::cout<< "Time: "<<std::setw(6)<< std::chrono::duration_cast<std::chrono::milliseconds>(timeSpan).count() <<"[ms]\n";
        
        if(result == nullptr){
            std::cout<<"Syntax error! \n";
        }else{
            result->print(0);
            delete result;
        }

        return 0;
    }

	std::string line;
    std::string term = ">>> ";
	while(true){
        std::cout<<"Start \n";
		std::cout<<term;
		std::getline(std::cin, line);

        auto sTime = std::chrono::system_clock::now();

		tokens = Lexer::lexer(line);

        parser::AST::AST* result = parser::parser();

        auto eTime = std::chrono::system_clock::now();
        auto timeSpan = eTime - sTime;
        std::cout<< "Time: "<<std::setw(6)<< std::chrono::duration_cast<std::chrono::milliseconds>(timeSpan).count() <<"[ms]\n";
        
        if(result == nullptr){
            std::cout<<"Syntax error! \n";
        }else{

            result->print(0);            
            std::cout<<"Syntax OK! \n";        
            result->walk();
            std::cout<<"Syntax OK!!! \n";            
            delete result;
        }
        tokens.clear();
    }
	return 0;
}
