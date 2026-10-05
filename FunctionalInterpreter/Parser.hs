module Parser (parse, Program(..), Declaration(..), Statement(..), Expression(..)) where

--import Data.Maybe
import Data.Char
import Data.Bool
import Data.List
import Scanner
import Tokens
import Control.Monad.State (State)
import Distribution.Simple.Command (Description)


data Program = PROGRAM [Declaration] Token

instance Show Program where
    show (PROGRAM statements _) = show (length statements) ++ "\n" ++ unlines (map show statements)

-- A treelike structure following the gramar:
{--
expression     → assignment ;

assignment     → IDENTIFIER "=" assignment | logic_or

logic_or       → logic_and ( "or" logic_and )*
logic_and      → equality ( "and" equality )*
equality       → comparison ( ( "!=" | "==" ) comparison )*
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )*
term           → factor ( ( "-" | "+" ) factor )*
factor         → unary ( ( "/" | "*" ) unary )*

unary          → ( "!" | "-" ) unary | call
call           → primary ( "(" arguments? ")" )* 
primary        → "true" | "false" | "nil" | "this" | NUMBER | STRING | IDENTIFIER | "(" expression ")"
--}
data Expression
    = ASSIGN Token Expression
    | BINARY Expression Token Expression
    | CALL Expression [Expression]
    | GROUPING Expression
    | LITERAL Tokens.Literal
    | LOGICAL Expression Token Expression
    | UNARY Token Expression
    | VARIABLE Token
    | EMPTY

instance Show Expression where
    show (ASSIGN (TOKEN IDENTIFIER str _ _) expr) = str ++ "=" ++ show expr
    show (BINARY left (TOKEN _ op _ _) right) = "(" ++ show left ++ op ++ show right ++ ")"
    show (CALL expr arguments) = show expr ++ "(" ++ intercalate "," (map show arguments) ++ ")"
    show (GROUPING expr) = "(" ++ show expr ++ ")"
    show (LITERAL (NUM lit)) = show lit
    show (LITERAL (STR lit)) = show lit
    show (LITERAL (ID lit)) = lit
    show (LITERAL lit) = show lit
    show (LOGICAL left (TOKEN AND _ _ _) right) = "(" ++ show left ++ "&&" ++ show right ++ ")"
    show (LOGICAL left (TOKEN OR _ _ _) right) = "(" ++ show left ++ "||" ++ show right ++ ")"
    show (UNARY (TOKEN _ op _ _) expr) = "(" ++ op ++ show expr ++ ")"
    show (VARIABLE tok) = "var" ++ show tok

-- A treelike structure following the grammar:
{--
statement      → exprStmt
               | forStmt
               | ifStmt
               | printStmt
               | returnStmt
               | whileStmt
               | block

exprStmt       → expression ";"
forStmt        → "for" "(" ( varDecl | exprStmt | ";" )
                           expression? ";"
                           expression? ")" statement

ifStmt         → "if" "(" expression ")" statement
                 ( "else" statement )?
                 
printStmt      → "print" expression ";"
returnStmt     → "return" expression? ";"
whileStmt      → "while" "(" expression ")" statement
block          → "{" declaration* "}"
--}
data Statement
    = EXPRESSION Expression
    | FORFULL Statement Expression Expression Statement
    | FORNOINIT Expression Expression Statement
    | FORNOCOND Statement Expression Statement
    | FORNOINC Statement Expression Statement
    | FORONLYINIT Statement Statement
    | FORONLYCOND Expression Statement
    | FORONLYINC Expression Statement
    | FOREMPTY Statement
    | BLOCKSTMT [Declaration]
    | IFSTMT Expression Statement
    | IFELSESTMT Expression Statement Statement
    | PRINTSTMT Expression
    | RETURNSTMT Expression
    | RETURNEMPTY
    | WHILESTMT Expression Statement
    | EMPTYSTMT

instance Show Statement where
    show (EXPRESSION expr) = show expr ++ ";"
    show (PRINTSTMT expr) = "print" ++ show expr ++ ";"
    show (IFSTMT expr stmt) = "if(" ++ show expr ++ ")" ++ show stmt
    show (IFELSESTMT expr thenstmt elsestmt) = "if(" ++ show expr ++ ")" ++ show thenstmt ++ "else" ++ show elsestmt
    show (BLOCKSTMT decl) = "{" ++ intercalate "" (map show decl) ++ "}"
    show (RETURNSTMT expr) = "return " ++ show expr ++ ";"
    show RETURNEMPTY = "return;"
    show (WHILESTMT expr stmt) = "while" ++ "(" ++ show expr ++ ")" ++ show stmt
    show (FORFULL init cond incr body) = "for(" ++ show init ++ show cond ++ ";" ++ show incr ++ ")" ++ show body
    show (FORNOINIT cond incr body) = "for(;" ++ show cond ++ ";" ++ show incr ++ ")" ++ show body
    show (FORNOCOND init incr body) = "for("++ show init ++ ";" ++ show incr ++ ")" ++ show body
    show (FORNOINC init cond body) = "for("++ show init ++ ";" ++ show cond ++ ")" ++ show body
    show (FORONLYINIT init body) = "for("++ show init ++ ";)" ++ show body
    show (FORONLYCOND cond body) = "for("++ ";" ++ show cond ++ ";" ++ ")" ++ show body
    show (FORONLYINC incr body) = "for(;;" ++ show incr ++ ")" ++ show body
    show (FOREMPTY body) = "for(;;)" ++ show body

data Declaration
    = FUNDECL Expression Statement
    | VARDECL Expression Expression
    | VAREMPTY Expression
    | STATEMENT Statement

instance Show Declaration where
    show (VARDECL (VARIABLE (TOKEN _ tok _ _)) expr) = "V DEC -> " ++ tok ++ "=" ++ show expr ++ ";"
    show (VAREMPTY (VARIABLE (TOKEN _ tok _ _))) = "V DEC -> " ++ tok ++ ";"
    show (FUNDECL expr stmt) = "F DEC -> " ++ show expr ++ show stmt
    show (STATEMENT stmt) = show stmt


errorAt :: Int -> String -> String -> a
errorAt line character cause =
    error ("\x1b[31mError at '\x1b[0m\x1b[3;31m" ++ character ++ "\x1b[0m\x1b[31m' (" ++ cause ++ ") on line " ++ show line ++ "\x1b[0m")

printtoken token =
    let
        (TOKEN _ str _ _) = token
    in
        error str


parse :: [Token] -> Program
parse tokens =
    let
        (declarations, eof) = parse' tokens []
    in
        PROGRAM declarations eof

parse' :: [Token] -> [Declaration] -> ([Declaration], Token)
parse' tokens program =
    let
        (decl, rest) = declarationHandler tokens
    in
        if not (null rest) then
            let
                (TOKEN tt _ _ _) = head rest
            in
                if tt == EOF then
                    (program ++ [decl], head rest)
                else
                    parse' rest (program ++ [decl])
        else
            let
                TOKEN _ str _ line = last tokens
            in
                errorAt line str "EOF is missing or was consumed"

declarationHandler :: [Token] -> (Declaration, [Token])
declarationHandler tokens =
    let
        TOKEN tokentype _ _ _ = head tokens
    in
        case tokentype of
            VAR -> variableStatement (tail tokens)
            FUN -> functionStatement (tail tokens)
            _ -> let
                    (stmt, rest) = statementHandler tokens
                in
                    (STATEMENT stmt, rest)

statementHandler :: [Token] -> (Statement, [Token])
statementHandler tokens =
    let
        TOKEN tokentype _ _ _ = head tokens
    in
        case tokentype of
        PRINT -> printStatement (tail tokens)
        IF -> ifStatement (tail tokens)
        WHILE -> whileStatement (tail tokens)
        FOR -> forStatement (tail tokens)
        LEFT_BRACE -> blockStatement (tail tokens)
        RETURN -> returnStatement (tail tokens)
        _ -> exprStatement tokens

printStatement :: [Token] -> (Statement, [Token])
printStatement tokens =
    let
        (expr, rest) = expression tokens
        TOKEN semicolonCheck semicolonStr _ semicolonLine = head rest
    in
        if not (null rest) && semicolonCheck == SEMICOLON then
            (PRINTSTMT expr, tail rest)
        else
            errorAt semicolonLine semicolonStr "Missing end of line ';'"


ifStatement :: [Token] -> (Statement, [Token])
ifStatement tokens =
    let
        (expr, exprRest) = expression (tail tokens)
        (TOKEN rightParenthesisCheck rightParenStr _ rightParenLine) = head exprRest
        (TOKEN firstToken firstStr _ firstLine) = head tokens
    in
        if firstToken == LEFT_PAREN then
            if not (null exprRest) && rightParenthesisCheck == RIGHT_PAREN then
                let
                    (thenstmt, thenrest) = statementHandler (tail exprRest)
                    (TOKEN elsecheck _ _ _) = head thenrest
                in
                    if elsecheck == ELSE then
                        let
                            (elsestmt, elserest) = statementHandler (tail thenrest)
                        in
                            (IFELSESTMT expr thenstmt elsestmt, elserest)
                    else
                        (IFSTMT expr thenstmt, thenrest)
            else
                errorAt rightParenLine rightParenStr "Expected a ')'"
        else
            errorAt firstLine firstStr "Expected a '('"

whileStatement :: [Token] -> (Statement, [Token])
whileStatement tokens =
    let
        (expr, exprRest) = expression (tail tokens)
        (TOKEN restHead restStr _ restLine) = head exprRest
        (TOKEN firstToken str _ line) = head tokens
    in
        if firstToken == LEFT_PAREN then
            if not (null exprRest) && restHead == RIGHT_PAREN then
                let
                    (whileBody, whileRest) = statementHandler (tail exprRest)
                in
                    (WHILESTMT expr whileBody, whileRest)
            else
                errorAt restLine restStr "Expected a ')'"
        else
            errorAt line ("while " ++ str) "Expected a '('"


forStatement :: [Token] -> (Statement, [Token])
forStatement tokens =
    let
        (TOKEN firstToken str _ line) = head tokens
        (initStmt, initRest, initEmpty) = forInitializer (tail tokens)
        (condExpr, condRest, condEmpty) = forCondition initRest
        (incrExpr, incrRest, incrEmpty) = forIncrement condRest
        (forBody, rest) = statementHandler incrRest
    in
        if firstToken == LEFT_PAREN then
            case (initEmpty, condEmpty, incrEmpty) of
                (False, False, False) -> (FORFULL initStmt condExpr incrExpr forBody, rest)
                (True, False, False) -> (FORNOINIT condExpr incrExpr forBody, rest)
                (False, True, False) -> (FORNOCOND initStmt incrExpr forBody, rest)
                (False, False, True) -> (FORNOINC initStmt condExpr forBody, rest)
                (False, True, True) -> (FORONLYINIT initStmt forBody, rest)
                (True, False, True) -> (FORONLYCOND condExpr forBody, rest)
                (True, True, False) -> (FORONLYINC incrExpr forBody, rest)
                (True, True, True) -> (FOREMPTY forBody, rest)
        else
            errorAt line str "Expected a '('"
forInitializer :: [Token] -> (Statement, [Token], Bool)
forInitializer tokens =
    let
        (TOKEN firstToken str _ line) = head tokens
    in
        if not (null tokens) && firstToken == SEMICOLON then
            (EMPTYSTMT, tail tokens, True)
        else
            let
                (stmt, rest) = statementHandler tokens
            in
                (stmt, rest, False)


forCondition :: [Token] -> (Expression, [Token], Bool)
forCondition tokens =
    let
        (TOKEN firstToken str _ line) = head tokens
    in
        if not (null tokens) && firstToken == SEMICOLON then
            (EMPTY, tail tokens, True)
        else
            let
                (expr, rest) = expression tokens
                (TOKEN condColon errorStr _ errorLine) = head rest
            in
                if not (null rest) && condColon == SEMICOLON then
                    (expr, tail rest, False)
                else
                    errorAt errorLine errorStr "Expected a ';'"
forIncrement :: [Token] -> (Expression, [Token], Bool)
forIncrement tokens =
    let
        (TOKEN firstToken str _ line) = head tokens
    in
        if not (null tokens) && firstToken == SEMICOLON then
            (EMPTY, tail tokens, True)
        else
            let
                (expr, rest) = expression tokens
                (TOKEN incrColon errorStr _ errorLine) = head rest
            in
                if not (null rest) && incrColon == RIGHT_PAREN then
                    (expr, tail rest, False)
                else
                    errorAt errorLine errorStr "Expected a ')'"

blockStatement :: [Token] -> (Statement, [Token])
blockStatement tokens = blockStatement' tokens []

blockStatement' :: [Token] -> [Declaration] -> (Statement, [Token])
blockStatement' tokens declarations =
    let
        (TOKEN endCheck _ _ _) = head tokens
        (decl, rest) = declarationHandler tokens
    in
        if endCheck == RIGHT_BRACE then
            (BLOCKSTMT declarations, tail tokens)
        else
            blockStatement' rest (declarations ++ [decl])


variableStatement :: [Token] -> (Declaration, [Token])
variableStatement tokens =
    let
        (TOKEN varTok varStr _ varLine) = head tokens
        (TOKEN equalsTok equalStr _ equalLine) = head (tail tokens)
    in
        if varTok == IDENTIFIER then
            if not (null (tail tokens)) && equalsTok == EQUAL then
                let
                    (expr, rest) = expression (drop 2 tokens)
                    (TOKEN semicolon semicolonStr _ semicolonLine) = head rest
                in
                    if not (null rest) && semicolon == SEMICOLON then
                        (VARDECL (VARIABLE (head tokens)) expr, tail rest)
                    else
                        errorAt semicolonLine semicolonStr "Expected a ';'"
            else if not (null (tail tokens)) && equalsTok == SEMICOLON then
                (VAREMPTY (VARIABLE (head tokens)), drop 2 tokens)
            else
                errorAt equalLine equalStr "Expected a '='"
        else
            errorAt varLine varStr (varStr ++ " is not a variable")


functionStatement :: [Token] -> (Declaration, [Token])
functionStatement tokens =
    let
        (expr, funcRest) = call tokens
        (funcBody, bodyRest) = statementHandler funcRest
    in
        (FUNDECL expr funcBody, bodyRest)


returnStatement :: [Token] -> (Statement, [Token])
returnStatement tokens =
    let
        (expr, rest) = expression tokens
        (TOKEN lastInStmt lastInStmtStr _ lastInStmtLine) = head rest
        (TOKEN firstToken _ _ _) = head tokens
    in
        if firstToken == SEMICOLON then
            (RETURNEMPTY, tail tokens)
        else if not (null rest) && lastInStmt == SEMICOLON then
            (RETURNSTMT expr, tail rest)
        else if lastInStmt == EOF then
            errorAt lastInStmtLine "EOF" "Expected a ';'"
        else
            errorAt lastInStmtLine lastInStmtStr "Expected a ';'"


exprStatement :: [Token] -> (Statement, [Token])
exprStatement tokens =
    let
        (expr, rest) = expression tokens
        TOKEN lastTokenInStatement lastTokenStr _ lastTokenline = head rest
    in
        if not (null rest) && lastTokenInStatement == SEMICOLON then
            (EXPRESSION expr, tail rest)
        else
            let
                TOKEN tokentype whatIsActuallyThere _ missingEOL = head rest
                TOKEN tokenbefore beforeErrorStr _ beforeLine = tokens !! (length tokens - length rest - 1)
            in case tokentype of
                EQUAL -> errorAt missingEOL beforeErrorStr "Can not be assigned a value"
                EOF -> errorAt missingEOL "EOF" "Missing end of line ';'"
                _ -> errorAt missingEOL whatIsActuallyThere "Missing end of line ';'"



-------------------------------------------------
-- expression takes a token list and recursivly
-- with depth first creates an expression
expression :: [Token] -> (Expression, [Token])
expression tokens =
    let
        (expr, rest) = assignment tokens
    in
        (expr, rest)

---------------------------------------------------------------
-- assignment checks if the current token is a variable or function
-- then checks for a "=" after the token to be certain that it is 
-- a variable, Then this runs recursivly to group the rightside first
-- until something isnt a variable meaning that it is another expression
-- that the variable(s) should be equal to
--
-- The next can never be null because there is an EOF token that will
-- Send the controll flow back to the parse function. This is true for
-- all following functions
assignment :: [Token] -> (Expression, [Token])
assignment (next:tokens) =
    let
        (TOKEN firstToken _ _ _) = next
    in
        if firstToken == IDENTIFIER && not (null tokens) then
            let
                (TOKEN secondToken secondTokenStr _ secondTokenLine) = head tokens
            in
                if secondToken == EQUAL then
                    let
                        (expr, rest) = assignment (tail tokens)
                    in
                        (ASSIGN next expr, rest)
                else
                    logicOr (next:tokens)
        else
            logicOr (next:tokens)


-------------------------------------------------------------------------
-- logicOr takes a expression and checks if it is followed by
-- one or more expressions separated by "or" this groups the 
-- left most expressions first
logicOr :: [Token] -> (Expression, [Token])
logicOr tokens =
    let
        (expr, rest) = logicAnd tokens
    in
        logicOr' expr rest

logicOr' :: Expression -> [Token] -> (Expression, [Token])
logicOr' previousExpr tokens =
        if not (null tokens) then
            let
                (TOKEN firstToken _ _ _) = head tokens
            in
                if firstToken == OR then
                    let
                        (nextExpr, nextExprRest) = logicAnd (tail tokens)
                    in
                        logicOr' (LOGICAL previousExpr (head tokens) nextExpr) nextExprRest
                else
                    (previousExpr, tokens)
        else
            (previousExpr, tokens)


------------------------------------------------------------------------
-- logicAnd takes a expression and checks if it is followed by
-- one or more expressions separated by "and" this groups the 
-- left most expressions first
logicAnd :: [Token] -> (Expression, [Token])
logicAnd tokens =
    let
        (expr, rest) = equality tokens
    in
        logicAnd' expr rest

logicAnd' :: Expression -> [Token] -> (Expression, [Token])
logicAnd' previousExpr tokens =
        if not (null tokens) then
            let
                (TOKEN tokentype _ _ _) = head tokens
            in
                if tokentype == AND then
                    let
                        (expr2, rest2) = equality (tail tokens)
                    in
                        logicAnd' (LOGICAL previousExpr (head tokens) expr2) rest2
                else
                    (previousExpr, tokens)
        else
            (previousExpr, tokens)

----------------------------------------------------------------------------
-- equality takes a token list and with depth first gets all math expressions
-- "below" it and combinds them with a "==" or "!="
equality :: [Token] -> (Expression, [Token])
equality tokens =
    let
        (expr, rest) = comparison tokens
    in
        equality' expr rest

equality' :: Expression -> [Token] -> (Expression, [Token])
equality' previousExpr tokens =
        -- Must check that rest is not null before we can use the head
        if not (null tokens) then
            let
                (TOKEN firstToken _ _ _) = head tokens
            in
                -- If there is a "==" or "!=" then recursivly get more expressions to combine with
                if firstToken == BANG_EQUAL || firstToken == EQUAL_EQUAL then
                    let
                        (nextExpr, nextExprRest) = comparison (tail tokens) -- tail to not include the "==" or "!=" token
                    in
                        equality' (BINARY previousExpr (head tokens) nextExpr) nextExprRest
                else
                    (previousExpr, tokens)
        else
            (previousExpr, tokens)


-------------------------------------------------------------------------------------------
-- comparison takes a expression and checks if it is followed by
-- one or more expressions separated by "<","<=",">" or ">=" 
-- this groups the left most expressions first
comparison :: [Token] -> (Expression, [Token])
comparison tokens =
    let
        (expr, rest) = term tokens
    in
        comparison' expr rest

comparison' :: Expression -> [Token] -> (Expression, [Token])
comparison' previousExpr tokens =
        if not (null tokens) then
            let
                (TOKEN firstToken _ _ _) = head tokens
            in
                if firstToken == GREATER || firstToken == GREATER_EQUAL || firstToken == LESS || firstToken == LESS_EQUAL then
                    let
                        (nextExpr, nextExprRest) = term (tail tokens)
                    in
                        comparison' (BINARY previousExpr (head tokens) nextExpr) nextExprRest
                else
                    (previousExpr, tokens)
        else
            (previousExpr, tokens)


-------------------------------------------------------------------------------------------
-- term takes a expression and checks if it is followed by
-- one or more expressions separated by "-" or "+" 
-- this groups the left most expressions first
term :: [Token] -> (Expression, [Token])
term tokens =
    let
        (expr, rest) = factor tokens
    in
        term' expr rest

term' :: Expression -> [Token] -> (Expression, [Token])
term' previousExpr tokens =
        if not (null tokens) then
            let
                (TOKEN firstToken _ _ _) = head tokens
            in
                if firstToken == MINUS || firstToken == PLUS then
                    let
                        (nextExpr, nextExprRest) = factor (tail tokens)
                    in
                        term' (BINARY previousExpr (head tokens) nextExpr) nextExprRest
                else
                    (previousExpr, tokens)
        else
            (previousExpr, tokens)


-----------------------------------------------------------------------
-- factor takes a expression and checks if it is followed by
-- one or more expressions separated by "*" or "/" 
-- this groups the left most expressions first
factor :: [Token] -> (Expression, [Token])
factor tokens =
    let
        (expr, rest) = unary tokens
    in
        factor' expr rest

factor' :: Expression -> [Token] -> (Expression, [Token])
factor' expr rest =
        if not (null rest) then
            let
                (TOKEN firstToken _ _ _) = head rest
            in
                if firstToken == SLASH || firstToken == STAR then
                    let
                        (nextExpr, nextExprRest) = unary (tail rest)
                    in
                        factor' (BINARY expr (head rest) nextExpr) nextExprRest
                else
                    (expr, rest)
        else
            (expr, rest)


-------------------------------------------------------------
-- unary checks if the next token is a "!" or "-" and then takes
-- an expression that should be negated or inversed
unary :: [Token] -> (Expression, [Token])
unary (next:tokens) =
    let
        (TOKEN firstToken _ _ _) = next
    in
        if firstToken == BANG || firstToken == MINUS then
            let
                (recursiveUnaryExpr, recursiveRest) = unary tokens
            in
                (UNARY next recursiveUnaryExpr, recursiveRest)
        else
            let
                (unaryMainpartExpr, mainpartRest) = call (next:tokens)
            in
                (unaryMainpartExpr, mainpartRest)


-------------------------------------------------------------------------------------------------
-- call 
-- takes an expression and checks if it is followed by a "("
-- if it is then it should be a function (checks that too) then
-- go and get the arguments to the function
-- getArgs
-- Get an expression then check if it is followed by a comma
-- if so then recursivly collect all expressions
-- and return a list of expressions (function arguments)

call :: [Token] -> (Expression, [Token])
call tokens =
    let
        (expr, rest) = primary tokens
        (TOKEN parenthesisCheck _ _ openingParenLine) = head rest
        (TOKEN firstToken firstTokenStr _ firstTokenLine) = head tokens
    in
        if not (null rest) && parenthesisCheck == LEFT_PAREN then
            if firstToken == IDENTIFIER then
                let
                    (argListExpr, argListRest) = getArgs (tail rest) []
                in
                    if not (null argListRest) then
                        funcOnFunc (CALL expr argListExpr) argListRest -- Check for more parenthesis and recursive function calls
                    else
                        errorAt openingParenLine "(" "Missing closing parenthesis in function call"
            else
                errorAt firstTokenLine firstTokenStr (firstTokenStr ++ " is not a valid function")
        else
            (expr, rest)

funcOnFunc :: Expression -> [Token] -> (Expression, [Token])
funcOnFunc expr [] = (expr, [])
funcOnFunc expr tokens =
    let
        (TOKEN firstToken _ _ _) = head tokens
--        (TOKEN arg _ _ _) = head (tail tokens)
    in
        if firstToken == LEFT_PAREN then
--            if not (null (tail tokens)) && arg == RIGHT_PAREN then
--                funcOnFunc (CALL expr []) (tail (tail tokens))
--            else 
                let
                    (argListExpr, argListRest) = getArgs (tail tokens) []
                in
                    funcOnFunc (CALL expr argListExpr) argListRest
        else
            (expr, tokens)


getArgs :: [Token] -> [Expression] -> ([Expression], [Token])
getArgs tokens arguments =
    let
        (expr, rest) = expression tokens
        TOKEN firstToken _ _ missingArgLine = head tokens
    in
        if firstToken == RIGHT_PAREN then
            if null arguments then
                ([], tail tokens)
            else
                errorAt missingArgLine "," "Missing argument in function call"
        else if firstToken == COMMA then
            errorAt missingArgLine "," "Missing argument in function call"
        else if firstToken == EOF then
            errorAt missingArgLine "(" "Missing closing parenthesis"
        else if not (null rest) then
            let
                (TOKEN nextToken nextTokenStr _ nextTokenLine) = head rest
            in
                if nextToken == COMMA then
                    getArgs (tail rest) (arguments ++ [expr])
                else if nextToken == RIGHT_PAREN then
                    (arguments ++ [expr], tail rest)
                else
                    errorAt nextTokenLine nextTokenStr (nextTokenStr ++ " missing closing parenthesis")
        else
                errorAt missingArgLine ")" "Missing closing parenthesis"



----------------------------------------------------------------------------------------
-- primary checks if the current token is a literal or parenthesis, if it is a literal
-- then create a expression out of it. If it is an parenthesis then take the expression
-- that is within and check for a closing parenthesis.
primary :: [Token] -> (Expression, [Token])
primary (next:tokens) =
    let
        (TOKEN firstToken firstTokenStr firstTokenLiteral firstTokenLine) = next
    in case firstToken of
        FALSE -> (LITERAL FALSE_LIT, tokens)
        TRUE -> (LITERAL TRUE_LIT, tokens)
        NIL -> (LITERAL NIL_LIT, tokens)
        STRING -> (LITERAL firstTokenLiteral, tokens)
        NUMBER -> (LITERAL firstTokenLiteral, tokens)
        IDENTIFIER -> (LITERAL firstTokenLiteral, tokens)
        LEFT_PAREN ->
            let
                (expr, rest) = expression tokens
                (TOKEN rightParenthesisCheck rightParenStr _ rightParenLine) = head rest
            in
                if rightParenthesisCheck == RIGHT_PAREN then
                    (GROUPING expr, tail rest)
                else
                    errorAt rightParenLine rightParenStr "Expected a closing parenthasis"
        _ ->
            let
                (TOKEN secondToken secondTokenStr secondTokenLiteral secondTokenLine) = head tokens
            in
                case firstToken of
                    VAR -> errorAt firstTokenLine firstTokenStr "Variable declaration is not allowed here"
                    FUN -> errorAt firstTokenLine firstTokenStr "Function declaration is not allowed here"
                    SEMICOLON -> errorAt firstTokenLine firstTokenStr "Empty statement"
                    PLUS -> errorAt firstTokenLine firstTokenStr "Plus is a function and requires two arguments"
                    RIGHT_PAREN -> errorAt firstTokenLine firstTokenStr "Empty parenthesis"
                    EOF -> (EMPTY, next:tokens)
                    PRINT -> errorAt firstTokenLine (firstTokenStr ++ " " ++ secondTokenStr) "Expected an expression"
                    WHILE -> errorAt firstTokenLine (firstTokenStr ++ " " ++ secondTokenStr) "Expected an expression"
                    RETURN -> errorAt firstTokenLine (firstTokenStr ++ " " ++ secondTokenStr) "Expected an expression"
                    IF -> errorAt firstTokenLine (firstTokenStr ++ " " ++ secondTokenStr) "Expected an expression"
                    

                    _ -> errorAt firstTokenLine firstTokenStr "Unexpected character"
