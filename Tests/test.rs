def fib(n) {
    if (n==0)
        return 0
    else if (n==1)
        return 1
    else 
        return fib(n-1) + fib(n-2)
}

echo("FIBONACCI TEST ==> EXPECT 55 :: " + fib(10) + "\n")

opMap = {
    "+" : (x,y) => { return x + y }
    "-" : (x,y) => { return x - y }
    "*" : (x,y) => { return x * y }
    "/" : (x,y) => { return x / y }
}

echo("LAMBDA MAP TEST")
echo("\tEXPECT 10 :: "+ opMap["+"](5,5))
echo("\tEXPECT 5 :: " + opMap["-"](9,4))
echo("\tEXPECT 15 :: " + opMap["*"](3,5))
echo("\tEXPECT 4 :: " +opMap["/"](12,3))


echo("FLEX TEST")
def func() {
   return [
      () => { 
         return { 
            1 : () => {
               return [
                  () => {
                     return [
                        22
                     ]
                  }
               ]
            }
         } 
      }
   ]
}
echo("\tEXPECT 22 :: " + func()[0]()[1]()[0]()[0])

echo("COMPLEX EXPRESSIONS TEST")
echo("\tEXPECT 3 ::" + 1*2^3-3-4^1/7*7/2)