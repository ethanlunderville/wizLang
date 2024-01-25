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


l = [1,2,3].map((element) => { return element * element }).filter((e)=>{
    if (e == 1) 
        return e
}).forEach((e)=>{})

map = {
    '2':1
}

map['2'].echo()