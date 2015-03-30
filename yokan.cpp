#include <iostream>
#include <string>
#include <vector>
#include <map>

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
 -> std::vector<Token>{
	std::vector<Token> tokens;

	std::string buffer = "";
    for(auto c : aLine){

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
            case '"':
                tokens.push_back(Token(Token::DQUOTATION,"\""));
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

auto expr(std::vector<Token> tokens, int pos)
 -> int{
 	if((tokens.at(pos).getType()==Token::NAME &&
 	   	   values.find(tokens.at(pos).getVal()) != values.end()) ||
 	   tokens.at(pos).getType()==Token::NUMBER){
 		pos++;
 		if(tokens.at(pos).getType()==Token::OPE_ADD ||
 		   tokens.at(pos).getType()==Token::OPE_SUB ||
 		   tokens.at(pos).getType()==Token::OPE_MUL ||
 		   tokens.at(pos).getType()==Token::OPE_DIV){
 			pos++;
 			if((tokens.at(pos).getType()==Token::NAME &&
 	   			   values.find(tokens.at(pos).getVal()) != values.end()) ||
		 	   tokens.at(pos).getType()==Token::NUMBER){
 				int val1, val2;		

 				//一つ目の変数
	 			if(tokens.at(pos-2).getType() == Token::NAME){
	 				val1 = values[tokens.at(pos-2).getVal()];
	 			}else{
	 				val1 =std::stoi(tokens.at(pos-2).getVal());
	 			}
 				//二つの変数
	 			if(tokens.at(pos).getType() == Token::NAME){
	 				val2 = values[tokens.at(pos).getVal()];
	 			}else{
	 				val2 =std::stoi(tokens.at(pos).getVal());
	 			}

 				//演算子
	 			if(tokens.at(pos-1).getType() == Token::OPE_ADD){
	 				return val1 + val2;
	 			}else if(tokens.at(pos-1).getType() == Token::OPE_SUB){
	 				return val1 - val2;
	 			}else if(tokens.at(pos-1).getType() == Token::OPE_MUL){
	 				return val1 * val2;
	 			}else if(tokens.at(pos-1).getType() == Token::OPE_DIV){
	 				return val1 / val2;
	 			}
				return -1;
			}
 		}else if(tokens.at(pos).getType() == Token::FIN){
 			if(tokens.at(pos-1).getType() == Token::NAME){
 				return values[tokens.at(pos-1).getVal()];
 			}else{
 				return std::stoi(tokens.at(pos).getVal());
 			}
 		} 		
 	}
 	return -1;
}

auto statement(std::vector<Token> tokens, int pos)
 -> int{
	if(tokens.at(pos).getType()==Token::NAME){
		if(tokens.at(pos).getVal() == "put"){
			pos++;
			std::cout<<values[tokens.at(pos).getVal()]<<"\n";
			return 0;
		}else{
			values[tokens.at(pos).getVal()] = 0;
			pos++;
			if(tokens.at(pos).getType() == Token::EQUAL){
				pos++;
				if(expr(tokens, pos) != -1){
					values[tokens.at(pos-2).getVal()] = expr(tokens, pos);
					return 0;
				}
			}
		}
	}
	return -1;
}

auto perser(std::vector<Token> tokens, int pos)
 -> int{
 	if(statement(tokens, pos) != -1){
 		return 0;
 	}
 	return -1;
}

auto main()
 -> int{	
	std::string line;
	while(true){
		std::cout<<">>";
		std::getline(std::cin, line);
		auto tokens = lexer(line);
		/*
		for(auto token : tokens){
			std::cout<< token.getVal() <<"\n";
		}
		*/
		if(perser(tokens, 0) != -1){
		//	std::cout<<"Syntax is correct!\n";
		}else{
			std::cout<<"Syntax is invalid!\n";
		}
	}
	return 0;
}


