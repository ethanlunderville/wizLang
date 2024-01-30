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

temp = "hello".map((e)=>{ if (e == 'e') { return e } }).filter((e)=>{
    if (e == 'e'){
        return e
    }
})

e = [1,2,3]

def tmp() {
    return e
}

tmp()[0] = 55

echo(e)
