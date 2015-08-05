

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

## Rules
 
#Basic

- FIN
```
 <FIN> ::= "FIN" 
```

- NUMBER
```
 <NUMBER> ::= [0-9] {[0-9]}
```

- CHAR
```
 <CHAR>   ::= [A-Za-z] {[A-Za-z]}
```

- Name
```
 <Name>   ::= <CHAR> {<NUMBER> | <CHAR>}
```

#My Rules

- Number
```
 <Number> ::= <NUMBER> "." <NUMBER> | <NUMBER>
```

- List
```
 <List> ::= "[" + <Identifire> + "]" | "[" + <ListVariableDecl> + "]"
```

- Operator
```
 <Operator> ::= "+" | "-" | "*" | "/"
```

- BinaryExpr
```
 <BinaryExpr> ::= <Identifire> <Operator> <BinaryExpr> | <Identifire> <Operator> <Identifire>
```

- Identifire
```
 <Identifire> ::= <Number> | <Name>
```

- ListVariableDecl
```
 <ListVariableDecl> ::= <RightValue> "," <RightValue> | <RightValue>
```

- FunctionDecl
```
 <FunctionDecl> ::= "def" <Name> "(" <ListVariableDecl> "):" <FIN> |  "def" <Name> "():" <FIN>
```

- ConditionExpr
```
 <ConditionExpr> ::= <Identifire> "==" <Identifire> | <Identifire> "!=" <Identifire>
```

- IfStatement
```
 <IfStatement> ::= "if" <ConditionExpr> ":" <FIN>
```

- Statement
```
 <Statement> ::= <VariableDecl> | <IfStatement> | <FunctionDecl> | <FIN>
```

- RightValue
```
 <RightValue> ::= <BinaryExpr> | <Identifire> | <List>
```

- VariableDecl
```
 <VariableDecl> ::= <Name> "=" <RightValue>
```

## Licence

MIT

## Author

[MizukiSonoko](https://github.com/MizukiSonoko)

