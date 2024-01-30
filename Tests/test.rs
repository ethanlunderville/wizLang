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

echo("MAP/FILTER TEST")

def forEach(array,function) {
   for (i=0;i<array.size();i++)
       function(array[i]) 
}

def map(array, function) {
   retList = []
   for (i=0;i<array.size();i++)
       retList.append(function(array[i]))
   return retList
}

def filter(array, function) {
   retList = []
   for (i=0;i<array.size();i++){
       store = function(array[i])
       if (store != 0) 
           retList.append(store)
   }
   return retList
}

temp = "hello".map((e)=>{
   if (e == 'e') { 
       return e 
   } 
}).filter((e)=>{
   if (e == 'e'){
       return e
   }
})

echo("EXPECT [e] :: ")
echo(temp)
