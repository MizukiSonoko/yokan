

Yokan
====

Yokan is a thick, jellied dessert made of red bean paste, agar, and sugar. It is usually sold in a block form, and eaten in slices.

## Description
 This is simple compiler.

## Setup

```bash
 $ https://github.com/MizukiSonoko/yokan
 $ cd yokan
 $ make
 ```

## Usage

```bash
 $ ./yokan
```

## Rule
 
#Basic

 - <FIN> ::= "FIN" 
 - <NUMBER> ::= [0-9] {[0-9]}
 - <CHAR>   ::= [A-Za-z] {[A-Za-z]}
 - <Name>   ::= <CHAR> {<NUMBER> | <CHAR>}

#Rule

 - <Number> ::= <NUMBER> "." <NUMBER> | <NUMBER>
 - <List> ::= "[" + <Identifire> + "]" | "[" + <ListVariableDecl> + "]"
 - <Operator> ::= "+" | "-" | "*" | "/"
 - <BinaryExpr> ::= <Identifire> <Operator> <BinaryExpr> | <Identifire> <Operator> <Identifire>
 - <Identifire> ::= <Number> | <Name>
 - <ListVariableDecl> ::= <RightValue> "," <RightValue> | <RightValue>
 - <FunctionDecl> ::= "def" <Name> "(" <ListVariableDecl> "):" <FIN> |  "def" <Name> "():" <FIN>
 - <ConditionExpr> ::= <Identifire> "==" <Identifire> | <Identifire> "!=" <Identifire>
 - <IfStatement> ::= "if" <ConditionExpr> ":" <FIN>
 - <Statement> ::= <VariableDecl> | <IfStatement> | <FunctionDecl> | <FIN>
 - <RightValue> ::= <BinaryExpr> | <Identifire> | <List>
 - <VariableDecl> ::= <Name> "=" <RightValue>

## Licence

MIT

## Author

[MizukiSonoko](https://github.com/MizukiSonoko)

