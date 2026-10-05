import Scanner
import Parser
import Tokens
import System.Environment(getArgs)
import Debug.Trace (trace)

import qualified Data.Map as Map
import GHC.Exts.Heap (GenClosure(value))
import Data.Fixed (mod')

main :: IO ()
main = do
    args <- getArgs
    case args of
        [file] -> do
            contents <- readFile file
            let tokens = scanTokens contents
            let program = parse tokens
            let (PROGRAM declarations eoftok) = program
            let (PRINTS backwardsResult, _) = interpret declarations (PRINTS []) newEnvironment 0
            let result = PRINTS (reverse backwardsResult)
            print result
        _ -> putStrLn "wrong number of arguments (usage: lox filename)"

errorHandler :: Int -> String -> String -> a
errorHandler line character cause =
    error ("\x1b[31mError at '\x1b[0m\x1b[3;31m" ++ character ++ "\x1b[0m\x1b[31m' (" ++ cause ++ ") in scope " ++ show line ++ "\x1b[0m")

newtype Printouts = PRINTS [String]
instance Show Printouts where
    show (PRINTS list) = loopPrint list
        where
            loopPrint [] = ""
            loopPrint [first] = first
            loopPrint (first:list) = first ++ "\n" ++ loopPrint list

data Value = VNUM Float | VSTR String | VBOO Bool | VNIL
instance Show Value where
    show (VNUM num) = floatOrInt num
        where
            floatOrInt :: Float -> String
            floatOrInt num = 
                let
                    modulo = (num `mod'` 1.0)
                in
                    if modulo == 0.0 then show (round num :: Int)
                    else show num
    show (VSTR str) = str
    show (VBOO boo) = if boo then "true" else "false"
    show VNIL = "nil"
instance Eq Value where
    (VNUM a) == (VNUM b) = a == b
    (VSTR a) == (VSTR b) = a == b
    (VBOO a) == (VBOO b) = a == b
    VNIL == VNIL = True
    _ == _ = False
    (VNUM a) /= (VNUM b) = a /= b
    (VSTR a) /= (VSTR b) = a /= b
    (VBOO a) /= (VBOO b) = a /= b
    VNIL /= VNIL = False
    _ /= _ = True
instance Ord Value where
    compare (VNUM a) (VNUM b) = compare a b
    compare (VSTR a) (VSTR b) = compare a b
    compare (VBOO a) (VBOO b) = compare a b
    compare VNIL VNIL = EQ
    compare a b = errorHandler 0 (show a) ("can not be compared with " ++ show b)


type VarEnvironment = Map.Map String ([Value], [Int])

newEnvironment :: VarEnvironment
newEnvironment = Map.empty

insertVariable :: VarEnvironment -> String -> Value -> Int -> VarEnvironment
insertVariable environment name value scope =
    if Map.member name environment then
        let
            Just(varValues, varScopes) = Map.lookup name environment
        in
            if scope == head varScopes then
                errorHandler scope name "Variable is already declared"
            else
                Map.insert name (value:varValues, scope:varScopes) environment
    else
        Map.insert name ([value], [scope]) environment

lookupVariable :: VarEnvironment -> String -> Int -> Value
lookupVariable environment name scope =
    let
        Just(varValues, varScopes) = Map.lookup name environment
    in
        if Map.member name environment then
            let
                validScopes = filter (<= scope) varScopes
            in
                if not (null validScopes) then
                    varValues !! (length varValues - length validScopes)
                else
                    errorHandler scope name "Varaible not found, variable is out of scope"
        else
            errorHandler scope name "Variable not declared"

updateVariable :: VarEnvironment -> String -> Value -> Int -> VarEnvironment
updateVariable environment name value scope =
    let
        Just(varValues, varScopes) = Map.lookup name environment
    in
        if Map.member name environment then
            if scope >= head varScopes then
                Map.adjust (\(h:b,s) -> (value:b,s)) name environment
            else
                errorHandler scope name "Could not update variable, variable is out of scope"
        else
            error $ "Undefined variable '" ++ name ++ "'"

scopeVariable :: VarEnvironment -> Int -> VarEnvironment
scopeVariable environment scope =
    Map.foldlWithKey (\newMap key (values, scopes) ->
        let
            validScopes = filter (<= scope) scopes
            dropped = (length values - length validScopes)
            validValues = drop dropped values
        in
            if not (null validScopes) then
                    Map.insert key (validValues, validScopes) newMap
            else
                newMap
                ) newEnvironment environment


interpret :: [Declaration] -> Printouts -> VarEnvironment -> Int -> (Printouts, VarEnvironment)
interpret [] prints varEnv _ = (prints, varEnv)
interpret (declaration:prog) prints variables scope =
    let
        (varEnv, printOut, currentScope) = evaluateDeclaration variables scope prints declaration
    in
        interpret prog printOut varEnv currentScope


evaluateDeclaration :: VarEnvironment -> Int -> Printouts -> Declaration -> (VarEnvironment, Printouts, Int)
evaluateDeclaration varEnv scope prints declaration =
    case declaration of
        STATEMENT stmt -> let (printStmt, varEnvironment, currentScope) = evaluateStatement varEnv scope prints stmt in
                            (varEnvironment, printStmt, currentScope)
        VARDECL name expr -> let (value, environment) = evaluateExpression varEnv scope expr in
                                (declareVariable environment name value scope, prints, scope)
        VAREMPTY name -> (declareVariable varEnv name VNIL scope, prints, scope)

declareVariable :: VarEnvironment -> Expression -> Value -> Int -> VarEnvironment
declareVariable varEnvironment (VARIABLE tok) value scope =
    let
        (TOKEN tt name lit row) = tok
    in
        insertVariable varEnvironment name value scope


evaluateStatement :: VarEnvironment -> Int -> Printouts -> Statement -> (Printouts, VarEnvironment, Int)
evaluateStatement varEnv scope prints statement =
    case statement of
        PRINTSTMT expr ->
            let
                (updatedPrints, updatedVarEnvironment) = statementPrint varEnv scope expr prints
            in
                (updatedPrints, updatedVarEnvironment, scope)
        EXPRESSION expr ->
            let
                (_, updatedVarEnvironment) = evaluateExpression varEnv scope expr
            in
                (prints, updatedVarEnvironment, scope)
        IFSTMT expr stmt ->
            let
                (exprValue, updatedVarEnvironment) = evaluateExpression varEnv scope expr
            in
                statementIf varEnv scope prints exprValue stmt
        IFELSESTMT expr thenstmt elsestmt ->
            let
                (exprValue, updatedVarEnvironment) = evaluateExpression varEnv scope expr
            in
                statementIfElse varEnv scope prints exprValue thenstmt elsestmt
        BLOCKSTMT declarations ->
            let
                (newPrints, newVarEnvironment) = interpret declarations prints varEnv (scope+1)
                scopedVarEnvironment = scopeVariable newVarEnvironment scope
            in
                (newPrints, scopedVarEnvironment, scope)
        WHILESTMT whileExpr doStmt -> statementWhile varEnv scope prints whileExpr doStmt

statementWhile :: VarEnvironment -> Int -> Printouts -> Expression -> Statement -> (Printouts, VarEnvironment, Int)
statementWhile varEnv scope prints expr statement =
    let
        (value, environment) = evaluateExpression varEnv scope expr
    in
        case value of
            VBOO boo ->
                if boo then
                    whileLoop varEnv scope prints expr statement
                else
                    (prints, environment, scope)
            VNIL -> (prints, environment, scope)
            _ -> whileLoop varEnv scope prints expr statement
            -- numbers and strings are interpreted as true

whileLoop :: VarEnvironment -> Int -> Printouts -> Expression -> Statement -> (Printouts, VarEnvironment, Int)
whileLoop varEnv scope prints expr statement =
    let
        (newPrints, environment, _) = evaluateStatement varEnv scope prints statement
    in
        statementWhile environment scope newPrints expr statement

statementIf :: VarEnvironment -> Int -> Printouts -> Value -> Statement -> (Printouts, VarEnvironment, Int)
statementIf varEnv scope prints value statement =
    case value of
        VBOO boo ->
            if boo then
                evaluateStatement varEnv scope prints statement
            else
                (prints, varEnv, scope)
        VNIL -> (prints, varEnv, scope)
        _ -> evaluateStatement varEnv scope prints statement
        -- numbers and strings are interpreted as true

statementIfElse :: VarEnvironment -> Int -> Printouts -> Value -> Statement -> Statement -> (Printouts, VarEnvironment, Int)
statementIfElse varEnv scope prints value ifStmt elseStmt =
    case value of
        VBOO boo ->
            if boo then
                evaluateStatement varEnv scope prints ifStmt
            else
                evaluateStatement varEnv scope prints elseStmt
        VNIL -> evaluateStatement varEnv scope prints elseStmt
        _ -> evaluateStatement varEnv scope prints ifStmt
        -- numbers and strings are interpreted as true

statementPrint :: VarEnvironment -> Int -> Expression -> Printouts -> (Printouts, VarEnvironment)
statementPrint varEnv scope expr (PRINTS stringList) =
    let
        (value, varEnvironment) = evaluateExpression varEnv scope expr
    in
        (PRINTS (show value:stringList), varEnvironment)


evaluateExpression :: VarEnvironment -> Int -> Expression -> (Value, VarEnvironment)
evaluateExpression varEnvironment scope expr =
    case expr of
        ASSIGN (TOKEN tt name lit row) expr -> expressionAssign varEnvironment name scope expr
        BINARY expr1 (TOKEN tt name lit row) expr2 -> expressionBinary varEnvironment scope expr1 tt expr2
        --CALL funccall args -> expressionCall varEnvironment scope funccall args
        GROUPING subExpr -> evaluateExpression varEnvironment scope subExpr
        LITERAL lit -> expressionLiteral varEnvironment scope lit
        LOGICAL expr1 (TOKEN comp _ _ _) expr2 -> expressionLogical varEnvironment scope expr1 comp expr2
        UNARY (TOKEN unary _ _ _) expr -> expressionUnary varEnvironment scope unary expr
        VARIABLE (TOKEN tt name lit row) ->
            let
                value = lookupVariable varEnvironment name scope
            in
                (value, varEnvironment)
        _ -> (VNIL, varEnvironment)

expressionAssign :: VarEnvironment -> String -> Int -> Expression -> (Value, VarEnvironment)
expressionAssign varEnvironment name scope expr =
    let
        (exprValue, expressionVarEnvironment) = evaluateExpression varEnvironment scope expr
        updatedVarEnvironment = updateVariable expressionVarEnvironment name exprValue scope
    in
        (exprValue, updatedVarEnvironment)

expressionBinary :: VarEnvironment -> Int -> Expression -> TokenType -> Expression -> (Value, VarEnvironment)
expressionBinary varEnvironment scope expr1 operator expr2 =
    let
        (val1, varEnvironment1) = evaluateExpression varEnvironment scope expr1
        (val2, varEnvironment2) = evaluateExpression varEnvironment1 scope expr2

    in
    case operator of
        EQUAL_EQUAL -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VBOO (numComp val1 (==) val2), varEnvironment2)
                        (VSTR str1, VSTR str2) -> (VBOO (numComp val1 (==) val2), varEnvironment2)
                        _ -> error "Operands must be two numbers or two strings."
        BANG_EQUAL -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VBOO (numComp val1 (/=) val2), varEnvironment2)
                        (VSTR str1, VSTR str2) -> (VBOO (not (numComp val1 (==) val2)), varEnvironment2)
                        _ -> error "Operands must be two numbers or two strings."
        GREATER_EQUAL -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VBOO (numComp val1 (>=) val2), varEnvironment2)
                        _ -> error "Operands must be two numbers."
        GREATER -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VBOO (numComp val1 (>) val2), varEnvironment2)
                        _ -> error "Operands must be two numbers."
        LESS_EQUAL -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VBOO (numComp val1 (<=) val2), varEnvironment2)
                        _ -> error "Operands must be two numbers."
        LESS -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VBOO (numComp val1 (<) val2), varEnvironment2)
                        _ -> error "Operands must be two numbers."
        PLUS -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VNUM (num1+num2), varEnvironment2)
                        (VSTR str1, VSTR str2) -> (VSTR (str1++str2), varEnvironment2)
                        _ -> error "Operands must be two numbers or two strings."
        MINUS -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VNUM (num1-num2), varEnvironment2)
                        _ -> error "Operands must be two numbers."
        STAR -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VNUM (num1*num2), varEnvironment2)
                        _ -> error "Operands must be two numbers."
        SLASH -> case (val1, val2) of
                        (VNUM num1, VNUM num2) -> (VNUM (num1/num2), varEnvironment2)
                        _ -> error "Operands must be two numbers."
        _ -> error "Unknown operand"

--exprComp :: Value -> (a -> a -> Bool) -> Value -> Bool
numComp :: Value -> (Float -> Float -> Bool) -> Value -> Bool
numComp (VNUM val1) operator (VNUM val2) = operator val1 val2
numCalc :: Value -> (Float -> Float -> Float) -> Value -> Float
numCalc (VNUM val1) operator (VNUM val2) = operator val1 val2
strComp :: Value -> (String -> String -> Bool) -> Value -> Bool
strComp (VSTR val1) operator (VSTR val2) = operator val1 val2
boolComp :: Value -> (Bool -> Bool -> Bool) -> Value -> Bool
boolComp (VBOO val1) operator (VBOO val2) = operator val1 val2

expressionCall :: VarEnvironment -> Int -> Expression -> [Expression] -> (Value, VarEnvironment)
expressionCall varEnvironment scope name args = (VNIL, varEnvironment)


expressionLiteral :: VarEnvironment -> Int -> Literal -> (Value, VarEnvironment)
expressionLiteral varEnvironment scope literal =
    case literal of
        (NUM lit) -> (VNUM lit, varEnvironment)
        (STR lit) -> (VSTR lit, varEnvironment)
        (ID name) ->
            let
                value = lookupVariable varEnvironment name scope
            in
                (value, varEnvironment)
        TRUE_LIT -> (VBOO True, varEnvironment)
        FALSE_LIT -> (VBOO False, varEnvironment)
        NIL_LIT -> (VNIL, varEnvironment)
        _ -> error "error expressionLiteral"

expressionLogical :: VarEnvironment -> Int -> Expression -> TokenType -> Expression -> (Value, VarEnvironment)
expressionLogical varEnvironment scope expr1 comp expr2 =
    let
        (val1, env1) = evaluateExpression varEnvironment scope expr1
        (val2, env2) = evaluateExpression varEnvironment scope expr2
    in
        case comp of
            AND -> case val1 of
                    VBOO boo1 -> if not boo1 then (VBOO False, env1)
                                else case val2 of
                                    VBOO boo2 -> (VBOO (boo1 && boo2), env2)
                                    _ -> errorHandler scope (show val2) "Both values have to be booleans"
                    VNUM num1 -> case val2 of
                                    VNUM num2 -> (VNUM num2, env2)
                                    _ -> errorHandler scope (show val2) "Both values have to be numbers"
                    _ -> error "can only use booleans or numbers in comparisons with AND/OR"
            OR -> case val1 of
                    VBOO boo1 -> if boo1 then (VBOO True, env1)
                                else case val2 of
                                    VBOO boo2 -> (VBOO (boo1 || boo2), env2)
                                    _ -> errorHandler scope (show val2) "Both values have to be booleans"
                    VNUM num1 -> (VNUM num1, env1)
            _ -> errorHandler scope (show comp) "Not a valid logical comparator"

expressionUnary :: VarEnvironment -> Int -> TokenType -> Expression -> (Value, VarEnvironment)
expressionUnary varEnvironment scope unary expr =
    let
        (value, varEnv) = evaluateExpression varEnvironment scope expr
    in
    case unary of
                MINUS -> case value of
                            VNUM num -> (VNUM (num * (-1)), varEnv)
                            _ -> error "Can only have negative on numbers"
                BANG -> case value of
                            VBOO boo -> (VBOO (not boo), varEnv)