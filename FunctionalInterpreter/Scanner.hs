module Scanner (scanTokens) where

--import Data.Maybe
import Data.Char
import Data.Bool

import Tokens

scanTokens :: String -> [Token]
scanTokens str = scanToken str 1 []


scanToken :: String -> Int -> [Token] -> [Token]
-- Check if all characters have been scanned and we are at EOF
scanToken [] line tokenList = tokenList ++ [TOKEN EOF "" NONE line]

scanToken (currentChar:str) line tokenList
    ------------------- CHECK FOR WHITE SPACES --------------------------------
    | currentChar == '\n' = scanToken str (line+1) tokenList
    | currentChar == ' '  = scanToken str line tokenList
    | currentChar == '\r' = scanToken str line tokenList
    | currentChar == '\t' = scanToken str line tokenList
    ------------------- CHECK FOR WHITE SPACES --------------------------------

    --------------------------------- CHECK FOR SIGNLE SPECIAL SIGNS --------------------------------------------------
    -- match first character, Continue and increase current character by 1 and add the token to the list
    | currentChar == '(' = scanToken str line (tokenList ++ [TOKEN LEFT_PAREN "(" NONE line])
    | currentChar == ')' = scanToken str line (tokenList ++ [TOKEN RIGHT_PAREN ")" NONE line])
    | currentChar == '{' = scanToken str line (tokenList ++ [TOKEN LEFT_BRACE "{" NONE line])
    | currentChar == '}' = scanToken str line (tokenList ++ [TOKEN RIGHT_BRACE "}" NONE line])
    | currentChar == ',' = scanToken str line (tokenList ++ [TOKEN COMMA "," NONE line])
    | currentChar == '.' = scanToken str line (tokenList ++ [TOKEN DOT "." NONE line])
    | currentChar == '-' = scanToken str line (tokenList ++ [TOKEN MINUS "-" NONE line])
    | currentChar == '+' = scanToken str line (tokenList ++ [TOKEN PLUS "+" NONE line])
    | currentChar == ';' = scanToken str line (tokenList ++ [TOKEN SEMICOLON ";" NONE line])
    | currentChar == '*' = scanToken str line (tokenList ++ [TOKEN STAR "*" NONE line])
    --------------------------------- CHECK FOR SIGNLE SPECIAL SIGNS --------------------------------------------------

    -------------------------------- CHECK FOR DOUBLE COMPARISON OPERATIONS -----------------------------------------------------------------------------------------------------------------
    -- match first character, check that it isn't last in file and check the following character.   Continue but increase current with 2
    | currentChar == '!' && not (isEOF str) && head str == '=' = scanToken (tail str) line (tokenList ++ [TOKEN BANG_EQUAL "!=" NONE line])
    | currentChar == '=' && not (isEOF str) && head str == '=' = scanToken (tail str) line (tokenList ++ [TOKEN EQUAL_EQUAL "==" NONE line])
    | currentChar == '<' && not (isEOF str) && head str == '=' = scanToken (tail str) line (tokenList ++ [TOKEN LESS_EQUAL "<=" NONE line])
    | currentChar == '>' && not (isEOF str) && head str == '=' = scanToken (tail str) line (tokenList ++ [TOKEN GREATER_EQUAL ">=" NONE line])
    -------------------------------- CHECK FOR DOUBLE COMPARISON OPERATIONS -----------------------------------------------------------------------------------------------------------------

    -------------------------------- CHECK FOR SINGLE COMPARISON OPERATIONS ---------------------------------------
    | currentChar == '!' = scanToken str line (tokenList ++ [TOKEN BANG "!" NONE line])
    | currentChar == '=' = scanToken str line (tokenList ++ [TOKEN EQUAL "=" NONE line])
    | currentChar == '<' = scanToken str line (tokenList ++ [TOKEN LESS "<" NONE line])
    | currentChar == '>' = scanToken str line (tokenList ++ [TOKEN GREATER ">" NONE line])
    -------------------------------- CHECK FOR DOUBLE COMPARISON OPERATIONS ---------------------------------------

    ------------------------------------- CHECK FOR COMMENT AND SLASH -------------------------------------------------------------------------------------------
    -- match first character, check that it isn't last in file and check the following character is '/' then find the end of the line
    | currentChar == '/' && not (isEOF str) && head str == '/' =
        let
            rest = findNewline str
        in
            if not (null rest) && (head rest == '\n') then
                scanToken (tail rest) (line+1) tokenList
            else
                scanToken rest line tokenList
    -- Otherwise if it is single slash
    | currentChar == '/' = scanToken str line (tokenList ++ [TOKEN SLASH "/" NONE line])
    ------------------------------------- CHECK FOR COMMENT AND SLASH -------------------------------------------------------------------------------------------


    ----------------------------------------- CHECK FOR STRING ------------------------------------------------------------------------------
    | currentChar == '"' =
        let
            tokenString = getString str line
            newLinesInTS = length (filter (=='\n') tokenString)
        in
            scanToken (skipChars str (length tokenString+2) line) (line + newLinesInTS) (tokenList ++ [TOKEN STRING tokenString (STR tokenString) line])
            -- Skip forward with the length of the word + 2 because current character is " and moving length(tokenString) forward gives last
            -- character in the tokenString so we add 2 to skip the other " to the next character after the string.
    ----------------------------------------- CHECK FOR STRING ------------------------------------------------------------------------------

    ----------------------------------------- CHECK FOR NUMBER -------------------------------------------------------------------
    | isDigit currentChar =
        let
            number = getNumber (currentChar:str) line
        in
            scanToken (skipChars str (length number) line) line (tokenList ++ [TOKEN NUMBER number (NUM (read number :: Float)) line])
    ----------------------------------------- CHECK FOR NUMBER -------------------------------------------------------------------

    ----------------------------------------CHECK FOR IDENTIFIER ----------------------------------------------------------------------
    | isAlphaNumeric currentChar =
        let
            ident = getIdentifier (currentChar:str)
            identType = getTokenType ident
        in
            if identType == TRUE then scanToken (skipChars str (length ident) line) line (tokenList ++ [TOKEN TRUE ident TRUE_LIT line])
            else if identType == FALSE then scanToken (skipChars str (length ident) line) line (tokenList ++ [TOKEN FALSE ident FALSE_LIT line])
            else if identType == NIL then scanToken (skipChars str (length ident) line) line (tokenList ++ [TOKEN NIL ident NIL_LIT line])
            else if identType == IDENTIFIER then scanToken (skipChars str (length ident) line) line (tokenList ++ [TOKEN IDENTIFIER ident (ID ident) line])
            else scanToken (skipChars str (length ident) line) line (tokenList ++ [TOKEN identType ident NONE line])
    ----------------------------------------CHECK FOR IDENTIFIER ----------------------------------------------------------------------
    | otherwise = errorAt line [currentChar] "Unknown character"


--------------------------------------------------------------
-- errorAt(line:Int, character:String)
--
errorAt :: Int -> String -> String -> a
errorAt line character cause =
    error ("\x1b[31mError at '\x1b[0m\x1b[3;31m" ++ character ++ "\x1b[0m\x1b[31m' (" ++ cause ++ ") on line " ++ show line ++ "\x1b[0m")

---------------------------------------------------------------
-- isEOF(currentChar:char)
-- if current character index is equal to input string length 
-- then current is larger than the index of the last character
--
isEOF :: String -> Bool
isEOF [] = True
isEOF _ = False
---------------------------------------------------------------

-----------------------------------------------------------------
-- findNewline(str:String, current:int)
-- Loop until it finds the newline character and skips it or EOF
--
findNewline :: String -> String
findNewline [] = []
findNewline (currentChar:str) =
    if currentChar == '\n' then currentChar:str
    else findNewline str
-----------------------------------------------------------------


------------------------------------------------------------------------------
-- getString(str:String, current:int, line:int)
-- Loop until EOF or '"' is found. For each loop append the current char 
-- to a string and return that string when '"' is found
--
getString :: String -> Int -> String
getString str line = getString' str line []
getString' :: String -> Int -> String -> String
getString' (currentChar:str) line tokenString
  | currentChar == '"' = tokenString
  | isEOF str = errorAt line [currentChar] "No closing \""
  | otherwise = getString' str line (tokenString ++ [currentChar])
------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- skipChars(str:String, skips:int, line:int)
-- Skip a number of characters in str equal to skips.
-- Has to stop at one because it is already skipping
-- the current char in the call to this function.
skipChars :: String -> Int -> Int -> String
skipChars str 1 _ = str
skipChars [] skips line = errorAt line "EOF" ("Reached end of line with " ++ show skips ++ " skips left")
skipChars (_:str) skips line = skipChars str (skips-1) line
-----------------------------------------------------------------------------------------------------------------------------------------------------
-- getNumber(str:String, current:int, line:int)
-- Loop until EOF or non digit is found. For each loop append the current
-- char to a string and return that string when non digit is found
--
getNumber :: String -> Int -> String
getNumber str line = getNumber' str line [] False
getNumber' :: String -> Int -> String -> Bool -> String
getNumber' [] _ numberString _ = numberString
getNumber' (currentChar:str) line numberString hasDot
  | isDigit currentChar = getNumber' str line (numberString ++ [currentChar]) hasDot
  | currentChar == '.' && not hasDot = getNumber' str line (numberString ++ [currentChar]) True
  | last numberString == '.' = init numberString
  | otherwise = numberString
-----------------------------------------------------------------------------------------------------------------------------------------------------


-------------------------------------------------------------------------------------------------------------------
-- getIdentifier(str:String, current:int)
-- Loop until EOF or non identifier sign is found.
-- (identifier signs: 'letter' 'number' '_')
-- For each loop append the current char to a string
-- and return that string when non digit is found
--
getIdentifier :: String -> String
getIdentifier str = getIdentifier' str []
getIdentifier' :: String -> String -> String
getIdentifier' [] ident = ident
getIdentifier' (currentChar:str) ident =
    if isAlphaNumeric currentChar then getIdentifier' str (ident ++ [currentChar])
    else ident
-------------------------------------------------------------------------------------------------------------------

-------------------------------------------------
-- isAlphaNumeric(character:char)
-- Checks if input is a identifier sign
-- (identifier signs: 'letter' 'number' '_')
--
isAlphaNumeric :: Char -> Bool
isAlphaNumeric character =
    isAsciiLower character ||
    isAsciiUpper character ||
    character == '_' ||
    isDigit character
------------------------------------------------



------------------------------------------------- getTokenType(ident:string)
-- Checks which tokenType the identifier is
-- can be reserved keywords or variable name
-- 
getTokenType :: String -> TokenType
getTokenType ident
    | ident == "and" = AND
    | ident == "class" = CLASS
    | ident == "else" = ELSE
    | ident == "for" = FOR
    | ident == "fun" = FUN
    | ident == "if" = IF
    | ident == "or" = OR
    | ident == "print" = PRINT
    | ident == "return" = RETURN
    | ident == "super" = SUPER
    | ident == "this" = THIS
    | ident == "var" = VAR
    | ident == "while" = WHILE
    | ident == "true" = TRUE
    | ident == "false" = FALSE
    | ident == "nil" = NIL
    | otherwise = IDENTIFIER
-----------------------------------------------
