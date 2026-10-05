import Scanner
import Parser

main :: IO ()
main = do 
    contents <- readFile "text.lox"
    let tokens = scanTokens contents
    mapM_ print tokens
    let text = parse tokens
    print text