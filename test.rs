opMap = {
    "+" : (x,y) => { return x + y }
    "-" : (x,y) => { return x - y }
    "*" : (x,y) => { return x * y }
    "/" : (x,y) => { return x / y }
}

echo("LAMBDA MAP TEST")
echo("\nEXPECT 30 :: " + opMap["+"](20,10))